#include "../php_steamworks.h"
#include "../steam_api_c.h"

PHP_FUNCTION(steam_apps_is_subscribed)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamApps *apps = SteamAPI_SteamApps_v009();
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

    ISteamApps *apps = SteamAPI_SteamApps_v009();
    if (!apps) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamApps_BIsDlcInstalled(apps, (AppId_t)dlc_id));
}

PHP_FUNCTION(steam_apps_get_app_id)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamApps *apps = SteamAPI_SteamApps_v009();
    if (!apps) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_LONG((zend_long)SteamAPI_ISteamApps_GetAppOwner(apps));
}

PHP_FUNCTION(steam_apps_get_language)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamApps *apps = SteamAPI_SteamApps_v009();
    if (!apps) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    const char *lang = SteamAPI_ISteamApps_GetCurrentGameLanguage(apps);
    RETURN_STRING(lang);
}

PHP_FUNCTION(steam_apps_is_subscribed_app)
{
    zend_long app_id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(app_id)
    ZEND_PARSE_PARAMETERS_END();

    ISteamApps *apps = SteamAPI_SteamApps_v009();
    if (!apps) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamApps_BIsSubscribedApp(apps, (AppId_t)app_id));
}

PHP_FUNCTION(steam_apps_get_current_beta_name)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamApps *apps = SteamAPI_SteamApps_v009();
    if (!apps) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    char name[256];
    if (!SteamAPI_ISteamApps_GetCurrentBetaName(apps, name, (int)sizeof(name))) {
        /* Not running a beta branch. */
        RETURN_FALSE;
    }

    RETURN_STRING(name);
}

PHP_FUNCTION(steam_apps_get_earliest_purchase_time)
{
    zend_long app_id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(app_id)
    ZEND_PARSE_PARAMETERS_END();

    ISteamApps *apps = SteamAPI_SteamApps_v009();
    if (!apps) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_LONG((zend_long)SteamAPI_ISteamApps_GetEarliestPurchaseUnixTime(apps, (AppId_t)app_id));
}

PHP_FUNCTION(steam_apps_get_installed_depots)
{
    zend_long app_id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(app_id)
    ZEND_PARSE_PARAMETERS_END();

    ISteamApps *apps = SteamAPI_SteamApps_v009();
    if (!apps) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    DepotId_t depots[64];
    uint32 count = SteamAPI_ISteamApps_GetInstalledDepots(
        apps, (AppId_t)app_id, depots, 64);

    array_init(return_value);
    for (uint32 i = 0; i < count; i++) {
        add_next_index_long(return_value, (zend_long)depots[i]);
    }
}

PHP_FUNCTION(steam_apps_get_dlc_count)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamApps *apps = SteamAPI_SteamApps_v009();
    if (!apps) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_LONG((zend_long)SteamAPI_ISteamApps_GetDLCCount(apps));
}

PHP_FUNCTION(steam_apps_get_app_build_id)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamApps *apps = SteamAPI_SteamApps_v009();
    if (!apps) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_LONG((zend_long)SteamAPI_ISteamApps_GetAppBuildId(apps));
}
