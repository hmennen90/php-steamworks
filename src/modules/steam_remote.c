#include "../php_steamworks.h"
#include "../steam_iface.h"

PHP_FUNCTION(steam_remote_file_write)
{
    zend_string *filename, *data;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(filename)
        Z_PARAM_STR(data)
    ZEND_PARSE_PARAMETERS_END();

    ISteamRemoteStorage *remote = steamworks_remote_storage();
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

    ISteamRemoteStorage *remote = steamworks_remote_storage();
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

    ISteamRemoteStorage *remote = steamworks_remote_storage();
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

    ISteamRemoteStorage *remote = steamworks_remote_storage();
    if (!remote) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamRemoteStorage_FileDelete(remote, ZSTR_VAL(filename)));
}

PHP_FUNCTION(steam_remote_file_list)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamRemoteStorage *remote = steamworks_remote_storage();
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

/* ── Shared files (Phase 4a) ────────────────────────────────────────────────
 * A cloud file becomes shareable once FileShare has turned it into a UGC handle.
 * The handle is what goes onto a leaderboard entry (steam_stats_attach_leaderboard_ugc)
 * and what anyone who downloads that entry can fetch with ugc_download + ugc_read.
 * UGC handles are uint64 on the SDK side; the invalid handle (all bits set)
 * reads as -1 in PHP. */

PHP_FUNCTION(steam_remote_file_share)
{
    zend_string *filename;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(filename)
    ZEND_PARSE_PARAMETERS_END();

    ISteamRemoteStorage *remote = steamworks_remote_storage();
    if (!remote) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    /* Async → steam_get_call_result() → "remote_file_shared" with the UGC handle. */
    SteamAPICall_t call = SteamAPI_ISteamRemoteStorage_FileShare(remote, ZSTR_VAL(filename));
    if (call == 0) {
        RETURN_FALSE;
    }
    steamworks_register_call(call, STEAMWORKS_CALL_REMOTE_FILE_SHARE);
    RETURN_LONG((zend_long)call);
}

PHP_FUNCTION(steam_remote_ugc_download)
{
    zend_long ugc;
    zend_long priority = 0;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_LONG(ugc)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(priority)
    ZEND_PARSE_PARAMETERS_END();

    if (priority < 0) {
        php_error_docref(NULL, E_WARNING, "Priority must be greater than or equal to 0");
        RETURN_FALSE;
    }

    ISteamRemoteStorage *remote = steamworks_remote_storage();
    if (!remote) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    /* Async → "remote_ugc_downloaded" with size and name; then ugc_read(). A file
       already in the local cache completes on the next poll. */
    SteamAPICall_t call = SteamAPI_ISteamRemoteStorage_UGCDownload(
        remote, (UGCHandle_t)ugc, (uint32)priority);
    if (call == 0) {
        RETURN_FALSE;
    }
    steamworks_register_call(call, STEAMWORKS_CALL_REMOTE_UGC_DOWNLOAD);
    RETURN_LONG((zend_long)call);
}

PHP_FUNCTION(steam_remote_ugc_read)
{
    zend_long ugc;
    zend_long size;
    zend_long offset = 0;

    ZEND_PARSE_PARAMETERS_START(2, 3)
        Z_PARAM_LONG(ugc)
        Z_PARAM_LONG(size)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(offset)
    ZEND_PARSE_PARAMETERS_END();

    if (size <= 0 || size > INT32_MAX) {
        php_error_docref(NULL, E_WARNING, "Size must be between 1 and %d", INT32_MAX);
        RETURN_FALSE;
    }
    if (offset < 0 || offset > UINT32_MAX) {
        php_error_docref(NULL, E_WARNING, "Offset must be between 0 and %u", UINT32_MAX);
        RETURN_FALSE;
    }

    ISteamRemoteStorage *remote = steamworks_remote_storage();
    if (!remote) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    /* ContinueReadingUntilFinished: reading up to the last byte closes the file,
       a partial read leaves it open for the next offset. */
    zend_string *buffer = zend_string_alloc((size_t)size, 0);
    int32 read = SteamAPI_ISteamRemoteStorage_UGCRead(
        remote, (UGCHandle_t)ugc, ZSTR_VAL(buffer), (int32)size, (uint32)offset,
        k_EUGCRead_ContinueReadingUntilFinished);
    if (read <= 0) {
        zend_string_efree(buffer);
        php_error_docref(NULL, E_WARNING, "Failed to read shared file (not downloaded yet?)");
        RETURN_FALSE;
    }
    if (read < size) {
        buffer = zend_string_truncate(buffer, (size_t)read, 0);
    }
    ZSTR_VAL(buffer)[read] = '\0';
    RETURN_NEW_STR(buffer);
}

PHP_FUNCTION(steam_remote_get_ugc_details)
{
    zend_long ugc;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(ugc)
    ZEND_PARSE_PARAMETERS_END();

    ISteamRemoteStorage *remote = steamworks_remote_storage();
    if (!remote) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    AppId_t  app_id = 0;
    char    *name   = NULL; /* owned by Steam — copied below */
    int32    size   = 0;
    uint64_t owner  = 0;
    /* Only known once the file has been downloaded (ugc_download completed). */
    if (!SteamAPI_ISteamRemoteStorage_GetUGCDetails(
            remote, (UGCHandle_t)ugc, &app_id, &name, &size, &owner)) {
        RETURN_FALSE;
    }

    array_init(return_value);
    add_assoc_long(return_value, "app_id", (zend_long)app_id);
    add_assoc_string(return_value, "name", name ? name : "");
    add_assoc_long(return_value, "size", (zend_long)size);
    add_assoc_long(return_value, "owner", (zend_long)owner);
}
