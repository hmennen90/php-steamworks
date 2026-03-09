#include "../php_steamworks.h"
#include <steam/steam_api_flat.h>

PHP_FUNCTION(steam_stats_set_achievement)
{
    zend_string *achievement_id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(achievement_id)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v012();
    if (!stats) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamUserStats_SetAchievement(stats, ZSTR_VAL(achievement_id)));
}

PHP_FUNCTION(steam_stats_clear_achievement)
{
    zend_string *achievement_id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(achievement_id)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v012();
    if (!stats) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamUserStats_ClearAchievement(stats, ZSTR_VAL(achievement_id)));
}

PHP_FUNCTION(steam_stats_store)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v012();
    if (!stats) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamUserStats_StoreStats(stats));
}

PHP_FUNCTION(steam_stats_get_int)
{
    zend_string *name;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v012();
    if (!stats) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    int32 value = 0;
    if (!SteamAPI_ISteamUserStats_GetStatInt32(stats, ZSTR_VAL(name), &value)) {
        php_error_docref(NULL, E_WARNING, "Failed to get stat '%s'", ZSTR_VAL(name));
        RETURN_FALSE;
    }

    RETURN_LONG((zend_long)value);
}

PHP_FUNCTION(steam_stats_set_int)
{
    zend_string *name;
    zend_long value;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(name)
        Z_PARAM_LONG(value)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v012();
    if (!stats) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamUserStats_SetStatInt32(stats, ZSTR_VAL(name), (int32)value));
}

PHP_FUNCTION(steam_stats_get_float)
{
    zend_string *name;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v012();
    if (!stats) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    float value = 0.0f;
    if (!SteamAPI_ISteamUserStats_GetStatFloat(stats, ZSTR_VAL(name), &value)) {
        php_error_docref(NULL, E_WARNING, "Failed to get stat '%s'", ZSTR_VAL(name));
        RETURN_FALSE;
    }

    RETURN_DOUBLE((double)value);
}

PHP_FUNCTION(steam_stats_set_float)
{
    zend_string *name;
    double value;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(name)
        Z_PARAM_DOUBLE(value)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v012();
    if (!stats) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamUserStats_SetStatFloat(stats, ZSTR_VAL(name), (float)value));
}
