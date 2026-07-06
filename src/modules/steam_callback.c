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

enum { CB_KIND_WEBAPI_TICKET = 1 };

/* Stored web-api ticket response, keyed by HAuthTicket handle. */
struct webapi_ticket {
    int            result; /* EResult (1 = OK) */
    int            len;
    unsigned char *bytes;  /* pemalloc'd, len bytes (NULL if len == 0) */
};

static HashTable webapi_tickets;
static bool      storage_initialized = false;

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
static bool callbacks_registered = false;

/* ── Lifecycle (called from MINIT/MSHUTDOWN and steam_init/steam_shutdown) ──── */

void steamworks_callbacks_minit(void)
{
    zend_hash_init(&webapi_tickets, 8, NULL, NULL, 1);
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
    storage_initialized = false;
}

void steamworks_callbacks_register(void)
{
    if (callbacks_registered) { return; }
    cb_webapi_ticket.vtbl             = &CB_VTBL;
    cb_webapi_ticket.m_nCallbackFlags = 0;
    cb_webapi_ticket.m_iCallback      = k_iCallback_GetTicketForWebApiResponse;
    cb_webapi_ticket.kind             = CB_KIND_WEBAPI_TICKET;
    cb_webapi_ticket.size_bytes       = (int)sizeof(GetTicketForWebApiResponse_t);
    SteamAPI_RegisterCallback(&cb_webapi_ticket, k_iCallback_GetTicketForWebApiResponse);
    callbacks_registered = true;
}

void steamworks_callbacks_unregister(void)
{
    if (!callbacks_registered) { return; }
    SteamAPI_UnregisterCallback(&cb_webapi_ticket);
    callbacks_registered = false;
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
