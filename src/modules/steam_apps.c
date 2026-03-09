#include "../php_steamworks.h"
#include <steam/steam_api_flat.h>

PHP_FUNCTION(steam_apps_is_subscribed)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamApps *apps = SteamAPI_SteamApps_v008();
    if (!apps) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamApps_BIsSubscribed(apps));
}

PHP_FUNCTION(steam_apps_is_dlc_installed)
{
    zend_long dlc_id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(dlc_id)
    ZEND_PARSE_PARAMETERS_END();

    ISteamApps *apps = SteamAPI_SteamApps_v008();
    if (!apps) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamApps_BIsDlcInstalled(apps, (AppId_t)dlc_id));
}

PHP_FUNCTION(steam_apps_get_app_id)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamApps *apps = SteamAPI_SteamApps_v008();
    if (!apps) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_LONG((zend_long)SteamAPI_ISteamApps_GetAppOwner(apps));
}

PHP_FUNCTION(steam_apps_get_language)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamApps *apps = SteamAPI_SteamApps_v008();
    if (!apps) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    const char *lang = SteamAPI_ISteamApps_GetCurrentGameLanguage(apps);
    RETURN_STRING(lang);
}
