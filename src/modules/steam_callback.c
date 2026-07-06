#include "../php_steamworks.h"
#include "../steam_api_c.h"
#include <string.h>

/* ── General callback dispatch (not CallResults) ────────────────────────────
 *
 * Some Steam results arrive as *callbacks* (delivered during SteamAPI_RunCallbacks)
 * rather than SteamAPICall_t call results — e.g. GetTicketForWebApiResponse_t.
 *
 * The flat C API has no callback registration helper, so we fabricate objects
 * that are binary-compatible with the SDK's CCallbackBase (steam_api_common.h):
 *
 *     [ vtable ptr (8) ][ uint8 m_nCallbackFlags (@8) ][ pad ][ int m_iCallback (@12) ]
 *
 * and register them with SteamAPI_RegisterCallback. This coexists with
 * SteamAPI_RunCallbacks, so the verified SteamAPICall_t path (steam_async.c) is
 * left completely untouched.
 *
 * CCallbackBase vtable (no virtual destructor — Valve keep it out for cross-DLL ABI):
 *   [0] void Run(this, void *pvParam)
 *   [1] void Run(this, void *pvParam, bool bIOFailure, SteamAPICall_t)
 *   [2] int  GetCallbackSizeBytes(this)
 */

struct steamworks_ccallback;

struct steamworks_ccallback_vtbl {
    void (*Run)(struct steamworks_ccallback *self, void *param);
    void (*RunIO)(struct steamworks_ccallback *self, void *param, bool io_failure, SteamAPICall_t call);
    int  (*GetCallbackSizeBytes)(struct steamworks_ccallback *self);
};

struct steamworks_ccallback {
    const struct steamworks_ccallback_vtbl *vtbl; /* offset 0 */
    uint8_t m_nCallbackFlags;                     /* offset 8  (CCallbackBase) */
    int     m_iCallback;                          /* offset 12 (CCallbackBase) */
    /* Fields past here are ours; the SDK never reads them. */
    int     kind;
    int     size_bytes;
};

enum { CB_KIND_WEBAPI_TICKET = 1, CB_KIND_NET_CONNECTION_STATUS = 2 };

/* Stored web-api ticket response, keyed by HAuthTicket handle. */
struct webapi_ticket {
    int            result; /* EResult (1 = OK) */
    int            len;
    unsigned char *bytes;  /* pemalloc'd, len bytes (NULL if len == 0) */
};

/* Queued ISteamNetworkingSockets connection-status change. */
struct net_event {
    uint32_t       conn;
    int            new_state;
    int            old_state;
    uint64_t       peer;   /* remote SteamID (0 if not a SteamID identity) */
};

static HashTable webapi_tickets;
static HashTable net_events;      /* auto-indexed queue of struct net_event* */
static bool      storage_initialized = false;

/* Helper for reading fields out of the raw callback param by byte offset. */
#define NET_READ(base, off, type) (*(type *)((const unsigned char *)(base) + (off)))

static void webapi_ticket_free(struct webapi_ticket *t)
{
    if (t->bytes) { pefree(t->bytes, 1); }
    pefree(t, 1);
}

/* ── Fabricated-callback vtable implementation ─────────────────────────────── */

static void cb_run(struct steamworks_ccallback *self, void *param)
{
    if (self->kind == CB_KIND_WEBAPI_TICKET && storage_initialized && param) {
        GetTicketForWebApiResponse_t *r = (GetTicketForWebApiResponse_t *)param;

        struct webapi_ticket *t = pemalloc(sizeof(*t), 1);
        t->result = r->m_eResult;
        t->len    = r->m_cubTicket;
        if (t->len > 0) {
            if (t->len > (int)sizeof(r->m_rgubTicket)) { t->len = (int)sizeof(r->m_rgubTicket); }
            t->bytes = pemalloc(t->len, 1);
            memcpy(t->bytes, r->m_rgubTicket, t->len);
        } else {
            t->bytes = NULL;
        }

        struct webapi_ticket *old = zend_hash_index_find_ptr(
            &webapi_tickets, (zend_ulong)r->m_hAuthTicket);
        if (old) {
            webapi_ticket_free(old);
            zend_hash_index_del(&webapi_tickets, (zend_ulong)r->m_hAuthTicket);
        }
        zend_hash_index_update_ptr(&webapi_tickets, (zend_ulong)r->m_hAuthTicket, t);
    }
    else if (self->kind == CB_KIND_NET_CONNECTION_STATUS && storage_initialized && param) {
        struct net_event *e = pemalloc(sizeof(*e), 1);
        e->conn      = NET_READ(param, STEAMWORKS_NETCB_CONN_OFF, uint32_t);
        e->new_state = NET_READ(param, STEAMWORKS_NETCB_NEWSTATE_OFF, int32_t);
        e->old_state = NET_READ(param, STEAMWORKS_NETCB_OLDSTATE_OFF, int32_t);
        /* Peer identity lives inline at m_info.m_identityRemote. */
        e->peer = SteamAPI_SteamNetworkingIdentity_GetSteamID64(
            (void *)((const unsigned char *)param + STEAMWORKS_NETCB_IDENTITY_OFF));
        zend_hash_next_index_insert_ptr(&net_events, e);
    }
}

static void cb_run_io(struct steamworks_ccallback *self, void *param, bool io_failure, SteamAPICall_t call)
{
    (void)io_failure; (void)call;
    cb_run(self, param);
}

static int cb_size(struct steamworks_ccallback *self)
{
    return self->size_bytes;
}

static const struct steamworks_ccallback_vtbl CB_VTBL = { cb_run, cb_run_io, cb_size };

static struct steamworks_ccallback cb_webapi_ticket;
static struct steamworks_ccallback cb_net_status;
static bool callbacks_registered = false;

static void net_event_dtor(zval *zv)
{
    pefree(Z_PTR_P(zv), 1);
}

/* ── Lifecycle (called from MINIT/MSHUTDOWN and steam_init/steam_shutdown) ──── */

void steamworks_callbacks_minit(void)
{
    zend_hash_init(&webapi_tickets, 8, NULL, NULL, 1);
    zend_hash_init(&net_events, 8, NULL, net_event_dtor, 1);
    storage_initialized = true;
}

void steamworks_callbacks_mshutdown(void)
{
    if (!storage_initialized) { return; }
    struct webapi_ticket *t;
    ZEND_HASH_FOREACH_PTR(&webapi_tickets, t) {
        webapi_ticket_free(t);
    } ZEND_HASH_FOREACH_END();
    zend_hash_destroy(&webapi_tickets);
    zend_hash_destroy(&net_events); /* net_event_dtor frees each entry */
    storage_initialized = false;
}

static void cb_setup(struct steamworks_ccallback *cb, int icallback, int kind, int size_bytes)
{
    cb->vtbl             = &CB_VTBL;
    cb->m_nCallbackFlags = 0;
    cb->m_iCallback      = icallback;
    cb->kind             = kind;
    cb->size_bytes       = size_bytes;
    SteamAPI_RegisterCallback(cb, icallback);
}

void steamworks_callbacks_register(void)
{
    if (callbacks_registered) { return; }
    cb_setup(&cb_webapi_ticket, k_iCallback_GetTicketForWebApiResponse,
             CB_KIND_WEBAPI_TICKET, (int)sizeof(GetTicketForWebApiResponse_t));
    /* Connection-status param is SteamNetConnectionStatusChangedCallback_t
       (712 bytes per SDK 1.64); we read fields by offset only. */
    cb_setup(&cb_net_status, k_iCallback_SteamNetConnectionStatusChanged,
             CB_KIND_NET_CONNECTION_STATUS, 712);
    callbacks_registered = true;
}

void steamworks_callbacks_unregister(void)
{
    if (!callbacks_registered) { return; }
    SteamAPI_UnregisterCallback(&cb_webapi_ticket);
    SteamAPI_UnregisterCallback(&cb_net_status);
    callbacks_registered = false;
}

PHP_FUNCTION(steam_net_get_connection_events)
{
    ZEND_PARSE_PARAMETERS_NONE();

    array_init(return_value);
    if (!storage_initialized) {
        return;
    }

    struct net_event *e;
    ZEND_HASH_FOREACH_PTR(&net_events, e) {
        zval ev;
        array_init(&ev);
        add_assoc_long(&ev, "connection", (zend_long)e->conn);
        add_assoc_long(&ev, "state",      (zend_long)e->new_state);
        add_assoc_long(&ev, "old_state",  (zend_long)e->old_state);
        add_assoc_long(&ev, "peer",       (zend_long)e->peer);
        add_next_index_zval(return_value, &ev);
    } ZEND_HASH_FOREACH_END();

    /* Consume the queue. */
    zend_hash_clean(&net_events);
}

/* ── PHP functions: web-api auth ticket (backend auth) ─────────────────────── */

PHP_FUNCTION(steam_user_get_auth_ticket_for_web_api)
{
    zend_string *identity = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(identity)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUser *user = SteamAPI_SteamUser_v023();
    if (!user) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    HAuthTicket handle = SteamAPI_ISteamUser_GetAuthTicketForWebApi(
        user, identity ? ZSTR_VAL(identity) : NULL);
    if (handle == 0 /* k_HAuthTicketInvalid */) {
        RETURN_FALSE;
    }
    /* The ticket itself arrives later via the GetTicketForWebApiResponse_t
       callback; poll steam_user_get_web_api_ticket_result($handle) after
       steam_run_callbacks(). */
    RETURN_LONG((zend_long)handle);
}

PHP_FUNCTION(steam_user_get_web_api_ticket_result)
{
    zend_long handle;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(handle)
    ZEND_PARSE_PARAMETERS_END();

    if (!storage_initialized) {
        RETURN_NULL();
    }

    struct webapi_ticket *t = zend_hash_index_find_ptr(
        &webapi_tickets, (zend_ulong)(HAuthTicket)handle);
    if (!t) {
        /* Not delivered yet — keep polling (call steam_run_callbacks() first). */
        RETURN_NULL();
    }

    array_init(return_value);
    add_assoc_bool(return_value, "success", t->result == 1 /* k_EResultOK */);
    add_assoc_long(return_value, "result", t->result);
    if (t->len > 0 && t->bytes) {
        add_assoc_stringl(return_value, "ticket", (char *)t->bytes, t->len);
    } else {
        add_assoc_stringl(return_value, "ticket", "", 0);
    }

    /* Consume the stored result. */
    webapi_ticket_free(t);
    zend_hash_index_del(&webapi_tickets, (zend_ulong)(HAuthTicket)handle);
}
