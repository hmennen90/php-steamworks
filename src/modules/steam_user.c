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
