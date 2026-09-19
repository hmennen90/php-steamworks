#include "../php_steamworks.h"
#include "../steam_iface.h"

/* ── ISteamMatchmaking — lobbies (Phase 4c, "SteamMatchMaking009") ──────────
 *
 * A lobby is the room before a match: members, per-member data ("ready"), the
 * lobby's own data (settings) and a chat, all hosted by Steam. Creating and
 * joining are CallResults (steam_get_call_result → "lobby_created" /
 * "lobby_entered"); what happens in the lobby afterwards arrives as callbacks,
 * drained with steam_matchmaking_get_events() (steam_callback.c).
 *
 * Signatures, callback ids and struct layouts from Steamworks SDK 1.64
 * (unchanged in 1.65).
 */

#define MM_OR_FALSE(mm) \
    ISteamMatchmaking *mm = steamworks_matchmaking(); \
    if (!mm) { \
        php_error_docref(NULL, E_WARNING, "Steam not initialized"); \
        RETURN_FALSE; \
    }

/* Lobby metadata keys and values have hard limits; Steam drops an oversized
   write without saying so, so refuse it here where the caller can see it. */
static bool lobby_key_value_ok(zend_string *key, zend_string *value)
{
    if (ZSTR_LEN(key) == 0 || ZSTR_LEN(key) > k_nMaxLobbyKeyLength) {
        php_error_docref(NULL, E_WARNING, "Lobby data key must be 1 to %d characters", k_nMaxLobbyKeyLength);
        return false;
    }
    if (ZSTR_LEN(value) >= k_cubChatMetadataMax) {
        php_error_docref(NULL, E_WARNING, "Lobby data value must be shorter than %d bytes", k_cubChatMetadataMax);
        return false;
    }
    return true;
}

PHP_FUNCTION(steam_matchmaking_create_lobby)
{
    zend_long type;
    zend_long max_members;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(type)
        Z_PARAM_LONG(max_members)
    ZEND_PARSE_PARAMETERS_END();

    if (max_members < 1) {
        php_error_docref(NULL, E_WARNING, "A lobby needs room for at least one member");
        RETURN_FALSE;
    }

    MM_OR_FALSE(mm);

    /* Async → "lobby_created". The creator is in the lobby once it succeeded. */
    SteamAPICall_t call = SteamAPI_ISteamMatchmaking_CreateLobby(mm, (int)type, (int)max_members);
    if (call == 0) {
        RETURN_FALSE;
    }
    steamworks_register_call(call, STEAMWORKS_CALL_LOBBY_CREATED);
    RETURN_LONG((zend_long)call);
}

PHP_FUNCTION(steam_matchmaking_join_lobby)
{
    zend_long lobby;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(lobby)
    ZEND_PARSE_PARAMETERS_END();

    MM_OR_FALSE(mm);

    /* Async → "lobby_entered"; 'success' is false when the lobby was full, gone
       or locked (see 'response'). */
    SteamAPICall_t call = SteamAPI_ISteamMatchmaking_JoinLobby(mm, (uint64_steamid)lobby);
    if (call == 0) {
        RETURN_FALSE;
    }
    steamworks_register_call(call, STEAMWORKS_CALL_LOBBY_ENTERED);
    RETURN_LONG((zend_long)call);
}

PHP_FUNCTION(steam_matchmaking_leave_lobby)
{
    zend_long lobby;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(lobby)
    ZEND_PARSE_PARAMETERS_END();

    MM_OR_FALSE(mm);
    SteamAPI_ISteamMatchmaking_LeaveLobby(mm, (uint64_steamid)lobby);
    RETURN_TRUE;
}

PHP_FUNCTION(steam_matchmaking_invite_user_to_lobby)
{
    zend_long lobby;
    zend_long user;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(lobby)
        Z_PARAM_LONG(user)
    ZEND_PARSE_PARAMETERS_END();

    MM_OR_FALSE(mm);
    /* The invitee gets GameLobbyJoinRequested_t ("join_requested") on accepting. */
    RETURN_BOOL(SteamAPI_ISteamMatchmaking_InviteUserToLobby(
        mm, (uint64_steamid)lobby, (uint64_steamid)user));
}

PHP_FUNCTION(steam_matchmaking_get_num_lobby_members)
{
    zend_long lobby;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(lobby)
    ZEND_PARSE_PARAMETERS_END();

    MM_OR_FALSE(mm);
    /* Only known for a lobby the user is in. */
    RETURN_LONG((zend_long)SteamAPI_ISteamMatchmaking_GetNumLobbyMembers(mm, (uint64_steamid)lobby));
}

PHP_FUNCTION(steam_matchmaking_get_lobby_member_by_index)
{
    zend_long lobby;
    zend_long index;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(lobby)
        Z_PARAM_LONG(index)
    ZEND_PARSE_PARAMETERS_END();

    MM_OR_FALSE(mm);
    uint64_steamid member = SteamAPI_ISteamMatchmaking_GetLobbyMemberByIndex(
        mm, (uint64_steamid)lobby, (int)index);
    if (member == 0) {
        RETURN_FALSE; /* index out of range, or not in the lobby */
    }
    RETURN_LONG((zend_long)member);
}

PHP_FUNCTION(steam_matchmaking_get_lobby_owner)
{
    zend_long lobby;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(lobby)
    ZEND_PARSE_PARAMETERS_END();

    MM_OR_FALSE(mm);
    uint64_steamid owner = SteamAPI_ISteamMatchmaking_GetLobbyOwner(mm, (uint64_steamid)lobby);
    if (owner == 0) {
        RETURN_FALSE;
    }
    RETURN_LONG((zend_long)owner);
}

PHP_FUNCTION(steam_matchmaking_set_lobby_owner)
{
    zend_long lobby;
    zend_long owner;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(lobby)
        Z_PARAM_LONG(owner)
    ZEND_PARSE_PARAMETERS_END();

    MM_OR_FALSE(mm);
    /* Only the current owner may hand the lobby on. */
    RETURN_BOOL(SteamAPI_ISteamMatchmaking_SetLobbyOwner(
        mm, (uint64_steamid)lobby, (uint64_steamid)owner));
}

PHP_FUNCTION(steam_matchmaking_get_lobby_data)
{
    zend_long    lobby;
    zend_string *key;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(lobby)
        Z_PARAM_STR(key)
    ZEND_PARSE_PARAMETERS_END();

    MM_OR_FALSE(mm);
    const char *value = SteamAPI_ISteamMatchmaking_GetLobbyData(mm, (uint64_steamid)lobby, ZSTR_VAL(key));
    RETURN_STRING(value ? value : "");
}

PHP_FUNCTION(steam_matchmaking_set_lobby_data)
{
    zend_long    lobby;
    zend_string *key;
    zend_string *value;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_LONG(lobby)
        Z_PARAM_STR(key)
        Z_PARAM_STR(value)
    ZEND_PARSE_PARAMETERS_END();

    if (!lobby_key_value_ok(key, value)) {
        RETURN_FALSE;
    }
    MM_OR_FALSE(mm);
    /* Owner only. Every member gets "data_update" with member == lobby. */
    RETURN_BOOL(SteamAPI_ISteamMatchmaking_SetLobbyData(
        mm, (uint64_steamid)lobby, ZSTR_VAL(key), ZSTR_VAL(value)));
}

PHP_FUNCTION(steam_matchmaking_get_lobby_member_data)
{
    zend_long    lobby;
    zend_long    user;
    zend_string *key;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_LONG(lobby)
        Z_PARAM_LONG(user)
        Z_PARAM_STR(key)
    ZEND_PARSE_PARAMETERS_END();

    MM_OR_FALSE(mm);
    const char *value = SteamAPI_ISteamMatchmaking_GetLobbyMemberData(
        mm, (uint64_steamid)lobby, (uint64_steamid)user, ZSTR_VAL(key));
    RETURN_STRING(value ? value : "");
}

PHP_FUNCTION(steam_matchmaking_set_lobby_member_data)
{
    zend_long    lobby;
    zend_string *key;
    zend_string *value;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_LONG(lobby)
        Z_PARAM_STR(key)
        Z_PARAM_STR(value)
    ZEND_PARSE_PARAMETERS_END();

    if (!lobby_key_value_ok(key, value)) {
        RETURN_FALSE;
    }
    MM_OR_FALSE(mm);
    /* The local user's own data; every member gets "data_update". */
    SteamAPI_ISteamMatchmaking_SetLobbyMemberData(
        mm, (uint64_steamid)lobby, ZSTR_VAL(key), ZSTR_VAL(value));
    RETURN_TRUE;
}

PHP_FUNCTION(steam_matchmaking_set_lobby_joinable)
{
    zend_long lobby;
    zend_bool joinable;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(lobby)
        Z_PARAM_BOOL(joinable)
    ZEND_PARSE_PARAMETERS_END();

    MM_OR_FALSE(mm);
    /* Closing the lobby once the match starts keeps latecomers out. */
    RETURN_BOOL(SteamAPI_ISteamMatchmaking_SetLobbyJoinable(mm, (uint64_steamid)lobby, joinable != 0));
}

PHP_FUNCTION(steam_matchmaking_set_lobby_type)
{
    zend_long lobby;
    zend_long type;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(lobby)
        Z_PARAM_LONG(type)
    ZEND_PARSE_PARAMETERS_END();

    MM_OR_FALSE(mm);
    RETURN_BOOL(SteamAPI_ISteamMatchmaking_SetLobbyType(mm, (uint64_steamid)lobby, (int)type));
}

PHP_FUNCTION(steam_matchmaking_send_lobby_chat_msg)
{
    zend_long    lobby;
    zend_string *message;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(lobby)
        Z_PARAM_STR(message)
    ZEND_PARSE_PARAMETERS_END();

    if (ZSTR_LEN(message) == 0 || ZSTR_LEN(message) > STEAMWORKS_LOBBY_CHAT_MAX) {
        php_error_docref(NULL, E_WARNING, "Lobby chat message must be 1 to %d bytes", STEAMWORKS_LOBBY_CHAT_MAX);
        RETURN_FALSE;
    }
    MM_OR_FALSE(mm);
    /* Sent as the exact bytes, binary-safe - no terminator is added, so what
       arrives in "chat_message" is what was passed here. */
    RETURN_BOOL(SteamAPI_ISteamMatchmaking_SendLobbyChatMsg(
        mm, (uint64_steamid)lobby, ZSTR_VAL(message), (int)ZSTR_LEN(message)));
}

PHP_FUNCTION(steam_friends_activate_overlay_invite_dialog)
{
    zend_long lobby;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(lobby)
    ZEND_PARSE_PARAMETERS_END();

    ISteamFriends *friends = steamworks_friends();
    if (!friends) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }
    /* Overlay invite dialog for this lobby. */
    SteamAPI_ISteamFriends_ActivateGameOverlayInviteDialog(friends, (uint64_steamid)lobby);
    RETURN_TRUE;
}
