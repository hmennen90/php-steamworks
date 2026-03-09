#include "../php_steamworks.h"
#include <steam/steam_api_flat.h>

PHP_FUNCTION(steam_utils_get_app_id)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamUtils *utils = SteamAPI_SteamUtils_v010();
    if (!utils) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_LONG((zend_long)SteamAPI_ISteamUtils_GetAppID(utils));
}

PHP_FUNCTION(steam_utils_is_overlay_enabled)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamUtils *utils = SteamAPI_SteamUtils_v010();
    if (!utils) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamUtils_IsOverlayEnabled(utils));
}

PHP_FUNCTION(steam_utils_get_country_code)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamUtils *utils = SteamAPI_SteamUtils_v010();
    if (!utils) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    const char *code = SteamAPI_ISteamUtils_GetIPCountry(utils);
    RETURN_STRING(code);
}
