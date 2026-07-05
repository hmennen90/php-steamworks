#include "../php_steamworks.h"
#include "../steam_api_c.h"

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

PHP_FUNCTION(steam_utils_is_steam_deck)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamUtils *utils = SteamAPI_SteamUtils_v010();
    if (!utils) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamUtils_IsSteamRunningOnSteamDeck(utils));
}

PHP_FUNCTION(steam_utils_get_steam_ui_language)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamUtils *utils = SteamAPI_SteamUtils_v010();
    if (!utils) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    const char *lang = SteamAPI_ISteamUtils_GetSteamUILanguage(utils);
    RETURN_STRING(lang ? lang : "");
}

PHP_FUNCTION(steam_utils_get_server_real_time)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamUtils *utils = SteamAPI_SteamUtils_v010();
    if (!utils) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_LONG((zend_long)SteamAPI_ISteamUtils_GetServerRealTime(utils));
}

PHP_FUNCTION(steam_utils_get_current_battery_power)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamUtils *utils = SteamAPI_SteamUtils_v010();
    if (!utils) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    /* 0-100 = battery percentage; 255 = running on AC power. */
    RETURN_LONG((zend_long)SteamAPI_ISteamUtils_GetCurrentBatteryPower(utils));
}

PHP_FUNCTION(steam_utils_get_seconds_since_app_active)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamUtils *utils = SteamAPI_SteamUtils_v010();
    if (!utils) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_LONG((zend_long)SteamAPI_ISteamUtils_GetSecondsSinceAppActive(utils));
}
