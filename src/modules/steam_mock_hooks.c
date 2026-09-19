/*
 * steam_mock_hooks.c — test hook, compiled only with --enable-steamworks-mock.
 *
 * Steam delivers callbacks from SteamAPI_RunCallbacks; the mock SDK has no Steam
 * to deliver anything. steam_mock_fire_callback() makes the mock build the
 * callback struct in the SDK's layout and hand it to whatever the extension
 * registered — through the same CCallbackBase vtable Steam uses. A test that
 * fires an event therefore runs the extension's real dispatch and queue code.
 *
 * Never part of a release build: the PHP function only exists in the mock.
 */

#include "../php_steamworks.h"
#include "../steam_iface.h"

#ifdef STEAMWORKS_MOCK

/* Implemented in ci/mock_sdk/steam_api_mock.c. */
void SteamMock_FireRichPresenceJoinRequested(uint64_t friend_id, const char *connect);
void SteamMock_FireLobbyJoinRequested(uint64_t lobby, uint64_t friend_id);
void SteamMock_FireLobbyDataUpdate(uint64_t lobby, uint64_t member, bool success);
void SteamMock_FireLobbyChatUpdate(uint64_t lobby, uint64_t user, uint64_t changed_by, uint32 state);
void SteamMock_FireLobbyChatMsg(uint64_t lobby, uint64_t user, const char *message, int len);
void SteamMock_FireNetConnectionStatus(uint32 conn, uint64_t peer, uint32 listen_socket, int32 state, int32 old_state);

static zend_long hook_long(HashTable *args, const char *key)
{
    zval *v = zend_hash_str_find(args, key, strlen(key));
    return v ? zval_get_long(v) : 0;
}

/* The string argument, or '' — copied into a buffer the mock owns while firing. */
static zend_string *hook_str(HashTable *args, const char *key)
{
    zval *v = zend_hash_str_find(args, key, strlen(key));
    return v ? zval_get_string(v) : ZSTR_EMPTY_ALLOC();
}

PHP_FUNCTION(steam_mock_fire_callback)
{
    zend_string *name;
    HashTable   *args;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(name)
        Z_PARAM_ARRAY_HT(args)
    ZEND_PARSE_PARAMETERS_END();

    /* A real game registers after steam_init(); the mock's init reports "no
       Steam client", so tests register here. Idempotent. */
    steamworks_callbacks_register();

    if (zend_string_equals_literal(name, "rich_presence_join_requested")) {
        zend_string *connect = hook_str(args, "connect");
        SteamMock_FireRichPresenceJoinRequested((uint64_t)hook_long(args, "friend"), ZSTR_VAL(connect));
        zend_string_release(connect);
        RETURN_TRUE;
    }
    if (zend_string_equals_literal(name, "lobby_join_requested")) {
        SteamMock_FireLobbyJoinRequested((uint64_t)hook_long(args, "lobby"), (uint64_t)hook_long(args, "friend"));
        RETURN_TRUE;
    }
    if (zend_string_equals_literal(name, "lobby_data_update")) {
        SteamMock_FireLobbyDataUpdate((uint64_t)hook_long(args, "lobby"), (uint64_t)hook_long(args, "member"),
            hook_long(args, "success") != 0);
        RETURN_TRUE;
    }
    if (zend_string_equals_literal(name, "lobby_chat_update")) {
        SteamMock_FireLobbyChatUpdate((uint64_t)hook_long(args, "lobby"), (uint64_t)hook_long(args, "user"),
            (uint64_t)hook_long(args, "changed_by"), (uint32)hook_long(args, "state"));
        RETURN_TRUE;
    }
    if (zend_string_equals_literal(name, "lobby_chat_message")) {
        zend_string *message = hook_str(args, "message");
        SteamMock_FireLobbyChatMsg((uint64_t)hook_long(args, "lobby"), (uint64_t)hook_long(args, "user"),
            ZSTR_VAL(message), (int)ZSTR_LEN(message));
        zend_string_release(message);
        RETURN_TRUE;
    }

    if (zend_string_equals_literal(name, "net_connection_status")) {
        SteamMock_FireNetConnectionStatus((uint32)hook_long(args, "connection"), (uint64_t)hook_long(args, "peer"),
            (uint32)hook_long(args, "listen_socket"), (int32)hook_long(args, "state"),
            (int32)hook_long(args, "old_state"));
        RETURN_TRUE;
    }

    php_error_docref(NULL, E_WARNING, "Unknown mock callback \"%s\"", ZSTR_VAL(name));
    RETURN_FALSE;
}

#endif /* STEAMWORKS_MOCK */
