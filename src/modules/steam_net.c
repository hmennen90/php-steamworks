#include "../php_steamworks.h"
#include "../steam_api_c.h"
#include <string.h>

/* ── ISteamNetworkingSockets — P2P messaging core ──────────────────────────
 *
 * Peer-to-peer connections over Steam's relay network. Connection-status
 * changes arrive via a callback (SteamNetConnectionStatusChangedCallback_t,
 * handled in steam_callback.c) — drain them with steam_net_get_connection_events()
 * after steam_run_callbacks(). Received messages are polled synchronously with
 * steam_net_receive_messages().
 *
 * Verified against Steamworks SDK 1.64 (SteamNetworkingSockets_v012).
 */

static ISteamNetworkingSockets *steamworks_net(void)
{
    return SteamAPI_SteamNetworkingSockets_SteamAPI_v012();
}

PHP_FUNCTION(steam_net_init_relay_network_access)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamNetworkingUtils *utils = SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
    if (!utils) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    /* Kick off relay network access early so the first P2P connection is fast. */
    SteamAPI_ISteamNetworkingUtils_InitRelayNetworkAccess(utils);
    RETURN_TRUE;
}

PHP_FUNCTION(steam_net_create_listen_socket_p2p)
{
    zend_long virtual_port = 0;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(virtual_port)
    ZEND_PARSE_PARAMETERS_END();

    ISteamNetworkingSockets *net = steamworks_net();
    if (!net) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    HSteamListenSocket sock = SteamAPI_ISteamNetworkingSockets_CreateListenSocketP2P(
        net, (int)virtual_port, 0, NULL);
    if (sock == 0 /* k_HSteamListenSocket_Invalid */) {
        RETURN_FALSE;
    }
    RETURN_LONG((zend_long)sock);
}

PHP_FUNCTION(steam_net_connect_p2p)
{
    zend_long steam_id;
    zend_long virtual_port = 0;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_LONG(steam_id)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(virtual_port)
    ZEND_PARSE_PARAMETERS_END();

    ISteamNetworkingSockets *net = steamworks_net();
    if (!net) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    /* Build a SteamNetworkingIdentity for the remote SteamID (opaque buffer of
       the verified size; SetSteamID64 fills it). */
    unsigned char identity[STEAMWORKS_NETIDENTITY_SIZE];
    memset(identity, 0, sizeof(identity));
    SteamAPI_SteamNetworkingIdentity_SetSteamID64(identity, (uint64_steamid)steam_id);

    HSteamNetConnection conn = SteamAPI_ISteamNetworkingSockets_ConnectP2P(
        net, identity, (int)virtual_port, 0, NULL);
    if (conn == 0 /* k_HSteamNetConnection_Invalid */) {
        RETURN_FALSE;
    }
    RETURN_LONG((zend_long)conn);
}

PHP_FUNCTION(steam_net_accept_connection)
{
    zend_long conn;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(conn)
    ZEND_PARSE_PARAMETERS_END();

    ISteamNetworkingSockets *net = steamworks_net();
    if (!net) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    /* Returns EResult (1 = k_EResultOK). */
    RETURN_LONG((zend_long)SteamAPI_ISteamNetworkingSockets_AcceptConnection(
        net, (HSteamNetConnection)conn));
}

PHP_FUNCTION(steam_net_close_connection)
{
    zend_long    conn;
    zend_long    reason = 0;
    zend_string *debug = NULL;
    zend_bool    linger = 0;

    ZEND_PARSE_PARAMETERS_START(1, 4)
        Z_PARAM_LONG(conn)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(reason)
        Z_PARAM_STR_OR_NULL(debug)
        Z_PARAM_BOOL(linger)
    ZEND_PARSE_PARAMETERS_END();

    ISteamNetworkingSockets *net = steamworks_net();
    if (!net) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamNetworkingSockets_CloseConnection(
        net, (HSteamNetConnection)conn, (int)reason,
        debug ? ZSTR_VAL(debug) : NULL, linger ? 1 : 0));
}

PHP_FUNCTION(steam_net_send_message)
{
    zend_long    conn;
    zend_string *data;
    zend_bool    reliable = 1;

    ZEND_PARSE_PARAMETERS_START(2, 3)
        Z_PARAM_LONG(conn)
        Z_PARAM_STR(data)
        Z_PARAM_OPTIONAL
        Z_PARAM_BOOL(reliable)
    ZEND_PARSE_PARAMETERS_END();

    ISteamNetworkingSockets *net = steamworks_net();
    if (!net) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    int flags = reliable ? STEAMWORKS_NET_SEND_RELIABLE : STEAMWORKS_NET_SEND_UNRELIABLE;
    int64_t message_number = 0;
    int result = SteamAPI_ISteamNetworkingSockets_SendMessageToConnection(
        net, (HSteamNetConnection)conn, ZSTR_VAL(data), (uint32)ZSTR_LEN(data),
        flags, &message_number);
    /* Returns EResult (1 = OK). */
    RETURN_LONG((zend_long)result);
}

PHP_FUNCTION(steam_net_receive_messages)
{
    zend_long conn;
    zend_long max_messages = 32;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_LONG(conn)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(max_messages)
    ZEND_PARSE_PARAMETERS_END();

    ISteamNetworkingSockets *net = steamworks_net();
    if (!net) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    if (max_messages < 1)   { max_messages = 1; }
    if (max_messages > 256) { max_messages = 256; }

    void **messages = emalloc((size_t)max_messages * sizeof(void *));
    int count = SteamAPI_ISteamNetworkingSockets_ReceiveMessagesOnConnection(
        net, (HSteamNetConnection)conn, messages, (int)max_messages);

    array_init(return_value);
    if (count <= 0) {
        efree(messages);
        return;
    }

    for (int i = 0; i < count; i++) {
        void *msg = messages[i];
        if (!msg) { continue; }

        const void *data = *(const void * const *)((unsigned char *)msg + STEAMWORKS_NETMSG_DATA_OFF);
        int         size = *(const int *)((unsigned char *)msg + STEAMWORKS_NETMSG_SIZE_OFF);
        uint32      from = *(const uint32 *)((unsigned char *)msg + STEAMWORKS_NETMSG_CONN_OFF);
        int64_t     num  = *(const int64_t *)((unsigned char *)msg + STEAMWORKS_NETMSG_MSGNUM_OFF);
        int         flags = *(const int *)((unsigned char *)msg + STEAMWORKS_NETMSG_FLAGS_OFF);
        uint64_steamid peer = SteamAPI_SteamNetworkingIdentity_GetSteamID64(
            (unsigned char *)msg + STEAMWORKS_NETMSG_IDENTITY_OFF);

        zval entry;
        array_init(&entry);
        add_assoc_stringl(&entry, "data", (char *)(data ? data : ""), size > 0 ? (size_t)size : 0);
        add_assoc_long(&entry, "size", (zend_long)size);
        add_assoc_long(&entry, "connection", (zend_long)from);
        add_assoc_long(&entry, "peer", (zend_long)peer);
        add_assoc_long(&entry, "message_number", (zend_long)num);
        add_assoc_bool(&entry, "reliable", (flags & STEAMWORKS_NET_SEND_RELIABLE) != 0);
        add_next_index_zval(return_value, &entry);

        /* Release the SDK-owned message once we've copied its payload. */
        SteamAPI_SteamNetworkingMessage_t_Release(msg);
    }
    efree(messages);
}
