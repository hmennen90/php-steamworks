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
    /* Register our fabricated CCallback objects now that Steam is up, so
       RunCallbacks delivers general callbacks (e.g. web-api ticket responses). */
    steamworks_callbacks_register();
    RETURN_TRUE;
}

PHP_FUNCTION(steam_shutdown)
{
    ZEND_PARSE_PARAMETERS_NONE();

    steamworks_callbacks_unregister();
    SteamAPI_Shutdown();
}

PHP_FUNCTION(steam_run_callbacks)
{
    ZEND_PARSE_PARAMETERS_NONE();

    SteamAPI_RunCallbacks();
}
