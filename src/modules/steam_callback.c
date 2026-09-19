#include "../php_steamworks.h"
#include "../steam_iface.h"
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

enum {
    CB_KIND_WEBAPI_TICKET = 1,
    CB_KIND_NET_CONNECTION_STATUS = 2,
    CB_KIND_RICH_PRESENCE_JOIN = 3,
    CB_KIND_LOBBY_JOIN_REQUESTED = 4,
    CB_KIND_LOBBY_DATA_UPDATE = 5,
    CB_KIND_LOBBY_CHAT_UPDATE = 6,
    CB_KIND_LOBBY_CHAT_MSG = 7,
};

/* One queued lobby callback. All four kinds share a queue, so the game sees
   them in the order Steam delivered them (someone joins, then talks). */
struct mm_event {
    int      kind;         /* CB_KIND_LOBBY_* */
    uint64_t lobby;
    uint64_t user;         /* friend / member / user, depending on kind */
    uint64_t changed_by;   /* chat_update only */
    uint32_t state;        /* chat_update: EChatMemberStateChange bits */
    int      value;        /* data_update: success; chat_message: entry type */
    char    *message;      /* chat_message only, pemalloc'd */
    size_t   message_len;
};

/* Queued GameRichPresenceJoinRequested_t: a friend accepted an invite or clicked
   "Join", and the game should connect with this string. */
struct join_request {
    uint64_t friend_id;
    char     connect[k_cchMaxRichPresenceValueLength];
};

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
    uint32_t       listen_socket; /* incoming: the socket it arrived on; 0 = outgoing */
};

static HashTable webapi_tickets;
static HashTable net_events;      /* auto-indexed queue of struct net_event* */
static HashTable join_requests;   /* auto-indexed queue of struct join_request* */
static HashTable mm_events;       /* auto-indexed queue of struct mm_event* */
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
        e->listen_socket = NET_READ(param, STEAMWORKS_NETCB_LISTEN_OFF, uint32_t);
        /* Peer identity lives inline at m_info.m_identityRemote. */
        e->peer = SteamAPI_SteamNetworkingIdentity_GetSteamID64(
            (void *)((const unsigned char *)param + STEAMWORKS_NETCB_IDENTITY_OFF));
        zend_hash_next_index_insert_ptr(&net_events, e);
    }
    else if (self->kind == CB_KIND_RICH_PRESENCE_JOIN && storage_initialized && param) {
        const GameRichPresenceJoinRequested_t *r = (const GameRichPresenceJoinRequested_t *)param;
        struct join_request *j = pemalloc(sizeof(*j), 1);
        j->friend_id = r->m_steamIDFriend;
        memcpy(j->connect, r->m_rgchConnect, sizeof(j->connect));
        j->connect[sizeof(j->connect) - 1] = '\0';
        zend_hash_next_index_insert_ptr(&join_requests, j);
    }
    else if (self->kind >= CB_KIND_LOBBY_JOIN_REQUESTED && self->kind <= CB_KIND_LOBBY_CHAT_MSG
             && storage_initialized && param) {
        struct mm_event *e = pecalloc(1, sizeof(*e), 1);
        e->kind = self->kind;

        if (self->kind == CB_KIND_LOBBY_JOIN_REQUESTED) {
            const GameLobbyJoinRequested_t *r = param;
            e->lobby = r->m_steamIDLobby;
            e->user  = r->m_steamIDFriend;
        } else if (self->kind == CB_KIND_LOBBY_DATA_UPDATE) {
            const LobbyDataUpdate_t *r = param;
            e->lobby = r->m_ulSteamIDLobby;
            e->user  = r->m_ulSteamIDMember;
            e->value = r->m_bSuccess != 0;
        } else if (self->kind == CB_KIND_LOBBY_CHAT_UPDATE) {
            const LobbyChatUpdate_t *r = param;
            e->lobby      = r->m_ulSteamIDLobby;
            e->user       = r->m_ulSteamIDUserChanged;
            e->changed_by = r->m_ulSteamIDMakingChange;
            e->state      = r->m_rgfChatMemberStateChange;
        } else {
            /* The text is fetched now: the chat id is only an index into Steam's
               buffer, and reading it later is not guaranteed to work. */
            const LobbyChatMsg_t *r = param;
            e->lobby = r->m_ulSteamIDLobby;
            e->user  = r->m_ulSteamIDUser;
            e->value = r->m_eChatEntryType;

            ISteamMatchmaking *mm = steamworks_matchmaking();
            char buffer[STEAMWORKS_LOBBY_CHAT_MAX];
            uint64_steamid sender = 0;
            int entry_type = 0;
            int len = mm ? SteamAPI_ISteamMatchmaking_GetLobbyChatEntry(
                mm, e->lobby, (int)r->m_iChatID, &sender, buffer, (int)sizeof(buffer), &entry_type) : 0;
            if (len > 0) {
                e->message = pemalloc((size_t)len, 1);
                memcpy(e->message, buffer, (size_t)len);
                e->message_len = (size_t)len;
            }
        }
        zend_hash_next_index_insert_ptr(&mm_events, e);
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
static struct steamworks_ccallback cb_rich_presence_join;
static struct steamworks_ccallback cb_lobby_join;
static struct steamworks_ccallback cb_lobby_data;
static struct steamworks_ccallback cb_lobby_chat_update;
static struct steamworks_ccallback cb_lobby_chat_msg;
static bool callbacks_registered = false;

static void net_event_dtor(zval *zv)
{
    pefree(Z_PTR_P(zv), 1);
}

static void mm_event_dtor(zval *zv)
{
    struct mm_event *e = Z_PTR_P(zv);
    if (e->message) { pefree(e->message, 1); }
    pefree(e, 1);
}

/* ── Lifecycle (called from MINIT/MSHUTDOWN and steam_init/steam_shutdown) ──── */

void steamworks_callbacks_minit(void)
{
    zend_hash_init(&webapi_tickets, 8, NULL, NULL, 1);
    zend_hash_init(&net_events, 8, NULL, net_event_dtor, 1);
    zend_hash_init(&join_requests, 8, NULL, net_event_dtor, 1); /* same pefree dtor */
    zend_hash_init(&mm_events, 8, NULL, mm_event_dtor, 1);
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
    zend_hash_destroy(&join_requests);
    zend_hash_destroy(&mm_events);
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
       (712 bytes under pack(8), 704 under pack(4)); we read fields by offset only. */
    cb_setup(&cb_net_status, k_iCallback_SteamNetConnectionStatusChanged,
             CB_KIND_NET_CONNECTION_STATUS, STEAMWORKS_NETCB_SIZE);
    cb_setup(&cb_rich_presence_join, k_iCallback_GameRichPresenceJoinRequested,
             CB_KIND_RICH_PRESENCE_JOIN, (int)sizeof(GameRichPresenceJoinRequested_t));
    cb_setup(&cb_lobby_join, k_iCallback_GameLobbyJoinRequested,
             CB_KIND_LOBBY_JOIN_REQUESTED, (int)sizeof(GameLobbyJoinRequested_t));
    cb_setup(&cb_lobby_data, k_iCallback_LobbyDataUpdate,
             CB_KIND_LOBBY_DATA_UPDATE, (int)sizeof(LobbyDataUpdate_t));
    cb_setup(&cb_lobby_chat_update, k_iCallback_LobbyChatUpdate,
             CB_KIND_LOBBY_CHAT_UPDATE, (int)sizeof(LobbyChatUpdate_t));
    cb_setup(&cb_lobby_chat_msg, k_iCallback_LobbyChatMsg,
             CB_KIND_LOBBY_CHAT_MSG, (int)sizeof(LobbyChatMsg_t));
    callbacks_registered = true;
}

void steamworks_callbacks_unregister(void)
{
    if (!callbacks_registered) { return; }
    SteamAPI_UnregisterCallback(&cb_webapi_ticket);
    SteamAPI_UnregisterCallback(&cb_net_status);
    SteamAPI_UnregisterCallback(&cb_rich_presence_join);
    SteamAPI_UnregisterCallback(&cb_lobby_join);
    SteamAPI_UnregisterCallback(&cb_lobby_data);
    SteamAPI_UnregisterCallback(&cb_lobby_chat_update);
    SteamAPI_UnregisterCallback(&cb_lobby_chat_msg);
    callbacks_registered = false;
}

PHP_FUNCTION(steam_matchmaking_get_events)
{
    ZEND_PARSE_PARAMETERS_NONE();

    array_init(return_value);
    if (!storage_initialized) {
        return;
    }

    struct mm_event *e;
    ZEND_HASH_FOREACH_PTR(&mm_events, e) {
        zval ev;
        array_init(&ev);
        switch (e->kind) {
            case CB_KIND_LOBBY_JOIN_REQUESTED:
                add_assoc_string(&ev, "type", "join_requested");
                add_assoc_long(&ev, "lobby", (zend_long)e->lobby);
                add_assoc_long(&ev, "friend", (zend_long)e->user);
                break;
            case CB_KIND_LOBBY_DATA_UPDATE:
                add_assoc_string(&ev, "type", "data_update");
                add_assoc_long(&ev, "lobby", (zend_long)e->lobby);
                add_assoc_long(&ev, "member", (zend_long)e->user);
                add_assoc_bool(&ev, "success", e->value != 0);
                break;
            case CB_KIND_LOBBY_CHAT_UPDATE:
                add_assoc_string(&ev, "type", "chat_update");
                add_assoc_long(&ev, "lobby", (zend_long)e->lobby);
                add_assoc_long(&ev, "user", (zend_long)e->user);
                add_assoc_long(&ev, "changed_by", (zend_long)e->changed_by);
                add_assoc_long(&ev, "state", (zend_long)e->state);
                break;
            default:
                add_assoc_string(&ev, "type", "chat_message");
                add_assoc_long(&ev, "lobby", (zend_long)e->lobby);
                add_assoc_long(&ev, "user", (zend_long)e->user);
                add_assoc_long(&ev, "entry_type", (zend_long)e->value);
                add_assoc_stringl(&ev, "message", e->message ? e->message : "", e->message_len);
                break;
        }
        add_next_index_zval(return_value, &ev);
    } ZEND_HASH_FOREACH_END();

    /* Consume the queue. */
    zend_hash_clean(&mm_events);
}

PHP_FUNCTION(steam_friends_get_join_requests)
{
    ZEND_PARSE_PARAMETERS_NONE();

    array_init(return_value);
    if (!storage_initialized) {
        return;
    }

    struct join_request *j;
    ZEND_HASH_FOREACH_PTR(&join_requests, j) {
        zval entry;
        array_init(&entry);
        add_assoc_long(&entry, "friend", (zend_long)j->friend_id);
        add_assoc_string(&entry, "connect", j->connect);
        add_next_index_zval(return_value, &entry);
    } ZEND_HASH_FOREACH_END();

    /* Consume the queue. */
    zend_hash_clean(&join_requests);
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
        add_assoc_long(&ev, "listen_socket", (zend_long)e->listen_socket);
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

    ISteamUser *user = steamworks_user();
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
