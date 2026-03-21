#include "../php_steamworks.h"
#include "../steam_api_c.h"

PHP_FUNCTION(steam_init)
{
    ZEND_PARSE_PARAMETERS_NONE();

    SteamErrMsg errMsg = {0};
    ESteamAPIInitResult result = SteamAPI_InitFlat(&errMsg);
    if (result != k_ESteamAPIInitResult_OK) {
        php_error_docref(NULL, E_WARNING, "SteamAPI_InitFlat failed (%d): %s", (int)result, errMsg);
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
