#include "../php_steamworks.h"
#include "../steam_api_c.h"

PHP_FUNCTION(steam_user_get_steam_id)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamUser *user = SteamAPI_SteamUser_v023();
    if (!user) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    uint64_steamid id = SteamAPI_ISteamUser_GetSteamID(user);
    RETURN_LONG((zend_long)id);
}

PHP_FUNCTION(steam_user_is_logged_on)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamUser *user = SteamAPI_SteamUser_v023();
    if (!user) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamUser_BLoggedOn(user));
}

PHP_FUNCTION(steam_user_get_player_steam_level)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamUser *user = SteamAPI_SteamUser_v023();
    if (!user) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_LONG((zend_long)SteamAPI_ISteamUser_GetPlayerSteamLevel(user));
}

/* ── Auth tickets (session tickets) ─────────────────────────────────────────
 * For authenticating a player with a game server / peer via BeginAuthSession.
 *
 * NOTE: The GetAuthSessionTicket ticket buffer is filled synchronously, but the
 * ticket is only guaranteed valid to *send* once Steam posts the
 * GetAuthSessionTicketResponse_t callback. This extension does not yet dispatch
 * general callbacks (only SteamAPICall_t results), so callers should allow a
 * short delay before use. For pure backend/web auth, prefer
 * steam_user_get_auth_ticket_for_web_api() (callback-based — coming with the
 * callback-dispatch subsystem).
 */

PHP_FUNCTION(steam_user_get_auth_session_ticket)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamUser *user = SteamAPI_SteamUser_v023();
    if (!user) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    /* 1024 bytes is the documented maximum auth ticket size. */
    unsigned char ticket[1024];
    uint32 ticket_size = 0;
    HAuthTicket handle = SteamAPI_ISteamUser_GetAuthSessionTicket(
        user, ticket, (int)sizeof(ticket), &ticket_size, NULL);

    if (handle == 0 /* k_HAuthTicketInvalid */ || ticket_size == 0) {
        RETURN_FALSE;
    }

    array_init(return_value);
    add_assoc_long(return_value, "handle", (zend_long)handle);
    /* Raw ticket bytes — send to the game server / backend as-is. */
    add_assoc_stringl(return_value, "ticket", (char *)ticket, ticket_size);
}

PHP_FUNCTION(steam_user_begin_auth_session)
{
    zend_string *ticket;
    zend_long    steam_id;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(ticket)
        Z_PARAM_LONG(steam_id)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUser *user = SteamAPI_SteamUser_v023();
    if (!user) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    /* Returns EBeginAuthSessionResult (0 = OK). */
    int result = SteamAPI_ISteamUser_BeginAuthSession(
        user, ZSTR_VAL(ticket), (int)ZSTR_LEN(ticket), (uint64_steamid)steam_id);
    RETURN_LONG((zend_long)result);
}

PHP_FUNCTION(steam_user_end_auth_session)
{
    zend_long steam_id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(steam_id)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUser *user = SteamAPI_SteamUser_v023();
    if (!user) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPI_ISteamUser_EndAuthSession(user, (uint64_steamid)steam_id);
    RETURN_TRUE;
}

PHP_FUNCTION(steam_user_cancel_auth_ticket)
{
    zend_long handle;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(handle)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUser *user = SteamAPI_SteamUser_v023();
    if (!user) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPI_ISteamUser_CancelAuthTicket(user, (HAuthTicket)handle);
    RETURN_TRUE;
}
