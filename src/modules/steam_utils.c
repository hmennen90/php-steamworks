#include "../php_steamworks.h"
#include "../steam_iface.h"

PHP_FUNCTION(steam_utils_get_app_id)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamUtils *utils = steamworks_utils();
    if (!utils) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_LONG((zend_long)SteamAPI_ISteamUtils_GetAppID(utils));
}

PHP_FUNCTION(steam_utils_is_overlay_enabled)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamUtils *utils = steamworks_utils();
    if (!utils) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamUtils_IsOverlayEnabled(utils));
}

PHP_FUNCTION(steam_utils_get_country_code)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamUtils *utils = steamworks_utils();
    if (!utils) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    const char *code = SteamAPI_ISteamUtils_GetIPCountry(utils);
    RETURN_STRING(code);
}

/*
 * SDK 1.65 removed ISteamUtils::IsRunningOnSteamDeck(). The function is kept
 * working on top of IsRunningOnSteamHardware() so existing callers do not
 * break, but it is deprecated: Valve's guidance is to base feature decisions on
 * the specific capability (big-picture mode, VR, battery, controllers) rather
 * than on the device being a Deck, so that a game keeps behaving correctly on
 * Steam hardware that did not exist when it shipped.
 */
PHP_FUNCTION(steam_utils_is_steam_deck)
{
    ZEND_PARSE_PARAMETERS_NONE();

    php_error_docref(NULL, E_DEPRECATED,
        "steam_utils_is_steam_deck() is deprecated since Steamworks SDK 1.65; "
        "use steam_utils_get_hardware_type() for analytics, or a capability-specific "
        "call such as steam_utils_get_hardware_default_config() for game settings");

    ISteamUtils *utils = steamworks_utils();
    if (!utils) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamUtils_IsRunningOnSteamHardware(utils) == k_ESteamHardwareTypeSteamDeck);
}

/*
 * Which Steam hardware the process runs on (STEAM_HARDWARE_TYPE_*).
 * Intended for analytics, support and diagnostics — not for feature decisions.
 */
PHP_FUNCTION(steam_utils_get_hardware_type)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamUtils *utils = steamworks_utils();
    if (!utils) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_LONG((zend_long)SteamAPI_ISteamUtils_IsRunningOnSteamHardware(utils));
}

/*
 * Suggested default graphics/settings preset for the current device
 * (STEAM_HARDWARE_CONFIG_*). Valve can retarget this per device from the
 * partner site, so a shipped game picks sensible defaults on hardware that
 * did not exist at release.
 */
PHP_FUNCTION(steam_utils_get_hardware_default_config)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamUtils *utils = steamworks_utils();
    if (!utils) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_LONG((zend_long)SteamAPI_ISteamUtils_GetSteamHardwareDefaultConfig(utils));
}

/* True when the game runs under the Proton compatibility layer on Linux. */
PHP_FUNCTION(steam_utils_is_running_under_proton)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamUtils *utils = steamworks_utils();
    if (!utils) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamUtils_IsRunningUnderProton(utils));
}

PHP_FUNCTION(steam_utils_get_steam_ui_language)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamUtils *utils = steamworks_utils();
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

    ISteamUtils *utils = steamworks_utils();
    if (!utils) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_LONG((zend_long)SteamAPI_ISteamUtils_GetServerRealTime(utils));
}

PHP_FUNCTION(steam_utils_get_current_battery_power)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamUtils *utils = steamworks_utils();
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

    ISteamUtils *utils = steamworks_utils();
    if (!utils) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_LONG((zend_long)SteamAPI_ISteamUtils_GetSecondsSinceAppActive(utils));
}
