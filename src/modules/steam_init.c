#include "../php_steamworks.h"
#include "../steam_api_c.h"

PHP_FUNCTION(steam_init)
{
    ZEND_PARSE_PARAMETERS_NONE();

    if (!SteamAPI_Init()) {
        php_error_docref(NULL, E_WARNING, "SteamAPI_Init failed. Is Steam running?");
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_FUNCTION(steam_shutdown)
{
    ZEND_PARSE_PARAMETERS_NONE();

    SteamAPI_Shutdown();
}

PHP_FUNCTION(steam_run_callbacks)
{
    ZEND_PARSE_PARAMETERS_NONE();

    SteamAPI_RunCallbacks();
}
