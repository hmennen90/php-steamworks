#include "../php_steamworks.h"
#include <steam/steam_api_flat.h>

PHP_FUNCTION(steam_friends_get_name)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamFriends *friends = SteamAPI_SteamFriends_v017();
    if (!friends) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    const char *name = SteamAPI_ISteamFriends_GetPersonaName(friends);
    RETURN_STRING(name);
}

PHP_FUNCTION(steam_friends_set_rich_presence)
{
    zend_string *key, *value = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STR(key)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(value)
    ZEND_PARSE_PARAMETERS_END();

    ISteamFriends *friends = SteamAPI_SteamFriends_v017();
    if (!friends) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    const char *val = value ? ZSTR_VAL(value) : "";
    RETURN_BOOL(SteamAPI_ISteamFriends_SetRichPresence(friends, ZSTR_VAL(key), val));
}

PHP_FUNCTION(steam_friends_activate_overlay)
{
    zend_string *dialog;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(dialog)
    ZEND_PARSE_PARAMETERS_END();

    ISteamFriends *friends = SteamAPI_SteamFriends_v017();
    if (!friends) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPI_ISteamFriends_ActivateGameOverlay(friends, ZSTR_VAL(dialog));
}
