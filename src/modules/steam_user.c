#include "../php_steamworks.h"
#include <steam/steam_api_flat.h>

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
