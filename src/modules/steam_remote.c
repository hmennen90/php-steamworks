#include "../php_steamworks.h"
#include "../steam_api_c.h"

PHP_FUNCTION(steam_remote_file_write)
{
    zend_string *filename, *data;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(filename)
        Z_PARAM_STR(data)
    ZEND_PARSE_PARAMETERS_END();

    ISteamRemoteStorage *remote = SteamAPI_SteamRemoteStorage_v016();
    if (!remote) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamRemoteStorage_FileWrite(
        remote, ZSTR_VAL(filename), ZSTR_VAL(data), (int32)ZSTR_LEN(data)
    ));
}

PHP_FUNCTION(steam_remote_file_read)
{
    zend_string *filename;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(filename)
    ZEND_PARSE_PARAMETERS_END();

    ISteamRemoteStorage *remote = SteamAPI_SteamRemoteStorage_v016();
    if (!remote) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    int32 size = SteamAPI_ISteamRemoteStorage_GetFileSize(remote, ZSTR_VAL(filename));
    if (size <= 0) {
        php_error_docref(NULL, E_WARNING, "File '%s' not found or empty", ZSTR_VAL(filename));
        RETURN_FALSE;
    }

    char *buffer = emalloc(size + 1);
    int32 read = SteamAPI_ISteamRemoteStorage_FileRead(remote, ZSTR_VAL(filename), buffer, size);
    if (read <= 0) {
        efree(buffer);
        php_error_docref(NULL, E_WARNING, "Failed to read file '%s'", ZSTR_VAL(filename));
        RETURN_FALSE;
    }

    buffer[read] = '\0';
    RETVAL_STRINGL(buffer, read);
    efree(buffer);
}

PHP_FUNCTION(steam_remote_file_exists)
{
    zend_string *filename;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(filename)
    ZEND_PARSE_PARAMETERS_END();

    ISteamRemoteStorage *remote = SteamAPI_SteamRemoteStorage_v016();
    if (!remote) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamRemoteStorage_FileExists(remote, ZSTR_VAL(filename)));
}

PHP_FUNCTION(steam_remote_file_delete)
{
    zend_string *filename;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(filename)
    ZEND_PARSE_PARAMETERS_END();

    ISteamRemoteStorage *remote = SteamAPI_SteamRemoteStorage_v016();
    if (!remote) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamRemoteStorage_FileDelete(remote, ZSTR_VAL(filename)));
}

PHP_FUNCTION(steam_remote_file_list)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamRemoteStorage *remote = SteamAPI_SteamRemoteStorage_v016();
    if (!remote) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    array_init(return_value);

    int32 count = SteamAPI_ISteamRemoteStorage_GetFileCount(remote);
    for (int32 i = 0; i < count; i++) {
        int32 size = 0;
        const char *name = SteamAPI_ISteamRemoteStorage_GetFileNameAndSize(remote, i, &size);
        add_next_index_string(return_value, name);
    }
}
