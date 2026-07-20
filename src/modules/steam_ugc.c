#include "../php_steamworks.h"
#include "../steam_api_c.h"

/* ── ISteamUGC (Steam Workshop) — consume path ─────────────────────────────
 *
 * Covers the "subscribe to and use Workshop content" flow that most games need:
 * subscribe/unsubscribe (async), enumerate subscribed items, inspect item state,
 * trigger a download and poll its progress, and locate the installed folder.
 *
 * Subscribe/Unsubscribe return a SteamAPICall_t handle — poll
 * steam_get_call_result($handle). Everything else is synchronous. DownloadItem
 * completion is observed by polling steam_ugc_get_item_state() for the
 * STEAM_UGC_ITEM_STATE_INSTALLED bit (no callback needed).
 *
 * Browsing/querying (CreateQuery* + GetQueryUGCResult/SteamUGCDetails_t) is a
 * documented follow-up.
 *
 * Verified against Steamworks SDK 1.64 (ISteamUGC V021).
 */

static ISteamUGC *steamworks_ugc(void)
{
    return SteamAPI_SteamUGC_v021();
}

PHP_FUNCTION(steam_ugc_subscribe_item)
{
    zend_long file_id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(file_id)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUGC *ugc = steamworks_ugc();
    if (!ugc) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPICall_t call = SteamAPI_ISteamUGC_SubscribeItem(ugc, (PublishedFileId_t)file_id);
    if (call == 0) {
        RETURN_FALSE;
    }
    steamworks_register_call(call, STEAMWORKS_CALL_UGC_SUBSCRIBE);
    RETURN_LONG((zend_long)call);
}

PHP_FUNCTION(steam_ugc_unsubscribe_item)
{
    zend_long file_id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(file_id)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUGC *ugc = steamworks_ugc();
    if (!ugc) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPICall_t call = SteamAPI_ISteamUGC_UnsubscribeItem(ugc, (PublishedFileId_t)file_id);
    if (call == 0) {
        RETURN_FALSE;
    }
    steamworks_register_call(call, STEAMWORKS_CALL_UGC_UNSUBSCRIBE);
    RETURN_LONG((zend_long)call);
}

PHP_FUNCTION(steam_ugc_get_num_subscribed_items)
{
    zend_bool include_disabled = 0;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_BOOL(include_disabled)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUGC *ugc = steamworks_ugc();
    if (!ugc) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_LONG((zend_long)SteamAPI_ISteamUGC_GetNumSubscribedItems(ugc, include_disabled ? 1 : 0));
}

PHP_FUNCTION(steam_ugc_get_subscribed_items)
{
    zend_bool include_disabled = 0;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_BOOL(include_disabled)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUGC *ugc = steamworks_ugc();
    if (!ugc) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    uint32 count = SteamAPI_ISteamUGC_GetNumSubscribedItems(ugc, include_disabled ? 1 : 0);
    array_init(return_value);
    if (count == 0) {
        return;
    }

    PublishedFileId_t *ids = emalloc((size_t)count * sizeof(PublishedFileId_t));
    uint32 written = SteamAPI_ISteamUGC_GetSubscribedItems(ugc, ids, count, include_disabled ? 1 : 0);
    for (uint32 i = 0; i < written; i++) {
        add_next_index_long(return_value, (zend_long)ids[i]);
    }
    efree(ids);
}

PHP_FUNCTION(steam_ugc_get_item_state)
{
    zend_long file_id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(file_id)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUGC *ugc = steamworks_ugc();
    if (!ugc) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    /* Bitmask of STEAM_UGC_ITEM_STATE_* — test with bitwise AND in userland. */
    RETURN_LONG((zend_long)SteamAPI_ISteamUGC_GetItemState(ugc, (PublishedFileId_t)file_id));
}

PHP_FUNCTION(steam_ugc_get_item_install_info)
{
    zend_long file_id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(file_id)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUGC *ugc = steamworks_ugc();
    if (!ugc) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    uint64_t size_on_disk = 0;
    uint32   timestamp    = 0;
    char     folder[1024];
    folder[0] = '\0';
    if (!SteamAPI_ISteamUGC_GetItemInstallInfo(
            ugc, (PublishedFileId_t)file_id, &size_on_disk,
            folder, (uint32)sizeof(folder), &timestamp)) {
        /* Not installed. */
        RETURN_FALSE;
    }

    array_init(return_value);
    add_assoc_long(return_value, "size_on_disk", (zend_long)size_on_disk);
    add_assoc_string(return_value, "folder", folder);
    add_assoc_long(return_value, "timestamp", (zend_long)timestamp);
}

PHP_FUNCTION(steam_ugc_get_item_download_info)
{
    zend_long file_id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(file_id)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUGC *ugc = steamworks_ugc();
    if (!ugc) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    uint64_t downloaded = 0, total = 0;
    if (!SteamAPI_ISteamUGC_GetItemDownloadInfo(
            ugc, (PublishedFileId_t)file_id, &downloaded, &total)) {
        RETURN_FALSE;
    }

    array_init(return_value);
    add_assoc_long(return_value, "bytes_downloaded", (zend_long)downloaded);
    add_assoc_long(return_value, "bytes_total",      (zend_long)total);
}

PHP_FUNCTION(steam_ugc_download_item)
{
    zend_long file_id;
    zend_bool high_priority = 0;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_LONG(file_id)
        Z_PARAM_OPTIONAL
        Z_PARAM_BOOL(high_priority)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUGC *ugc = steamworks_ugc();
    if (!ugc) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    /* Starts a download; poll steam_ugc_get_item_download_info() /
       steam_ugc_get_item_state() for progress and completion. */
    RETURN_BOOL(SteamAPI_ISteamUGC_DownloadItem(ugc, (PublishedFileId_t)file_id, high_priority ? 1 : 0));
}

/* ── ISteamUGC (Steam Workshop) — publish path ─────────────────────────────
 *
 * Create + fill + submit a Workshop item straight from a content folder:
 *
 *   $call = steam_ugc_create_item($appId);          // async -> file_id
 *   // poll steam_get_call_result($call) until {success, file_id}
 *   $h = steam_ugc_start_item_update($appId, $fileId);
 *   steam_ugc_set_item_title($h, "...");
 *   steam_ugc_set_item_description($h, "...");
 *   steam_ugc_set_item_content($h, "C:\\abs\\path\\to\\mod");   // absolute
 *   steam_ugc_set_item_preview($h, "C:\\abs\\path\\preview.png");
 *   steam_ugc_set_item_visibility($h, STEAM_UGC_VISIBILITY_PUBLIC);
 *   $call = steam_ugc_submit_item_update($h, "change note"); // async
 *   // poll steam_get_call_result($call); steam_ugc_get_item_update_progress($h)
 *   // reports byte progress + STEAM_UGC_UPDATE_STATUS_* meanwhile.
 *
 * StartItemUpdate returns the update handle as an int (round-trips like a call
 * handle). The invalid handle sentinel k_UGCUpdateHandleInvalid surfaces as -1.
 * Verified against Steamworks SDK 1.64 (ISteamUGC V021).
 */

PHP_FUNCTION(steam_ugc_create_item)
{
    zend_long app_id;
    zend_long file_type = 0; /* k_EWorkshopFileTypeCommunity */

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_LONG(app_id)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(file_type)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUGC *ugc = steamworks_ugc();
    if (!ugc) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPICall_t call = SteamAPI_ISteamUGC_CreateItem(ugc, (AppId_t)app_id, (int)file_type);
    if (call == 0) {
        RETURN_FALSE;
    }
    steamworks_register_call(call, STEAMWORKS_CALL_UGC_CREATE_ITEM);
    RETURN_LONG((zend_long)call);
}

PHP_FUNCTION(steam_ugc_start_item_update)
{
    zend_long app_id, file_id;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(app_id)
        Z_PARAM_LONG(file_id)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUGC *ugc = steamworks_ugc();
    if (!ugc) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    UGCUpdateHandle_t handle = SteamAPI_ISteamUGC_StartItemUpdate(
        ugc, (AppId_t)app_id, (PublishedFileId_t)file_id);
    RETURN_LONG((zend_long)handle);
}

PHP_FUNCTION(steam_ugc_set_item_title)
{
    zend_long handle;
    char *value; size_t value_len;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(handle)
        Z_PARAM_STRING(value, value_len)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUGC *ugc = steamworks_ugc();
    if (!ugc) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamUGC_SetItemTitle(ugc, (UGCUpdateHandle_t)handle, value));
}

PHP_FUNCTION(steam_ugc_set_item_description)
{
    zend_long handle;
    char *value; size_t value_len;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(handle)
        Z_PARAM_STRING(value, value_len)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUGC *ugc = steamworks_ugc();
    if (!ugc) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamUGC_SetItemDescription(ugc, (UGCUpdateHandle_t)handle, value));
}

PHP_FUNCTION(steam_ugc_set_item_visibility)
{
    zend_long handle, visibility;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(handle)
        Z_PARAM_LONG(visibility)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUGC *ugc = steamworks_ugc();
    if (!ugc) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamUGC_SetItemVisibility(ugc, (UGCUpdateHandle_t)handle, (int)visibility));
}

PHP_FUNCTION(steam_ugc_set_item_content)
{
    zend_long handle;
    char *folder; size_t folder_len;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(handle)
        Z_PARAM_STRING(folder, folder_len)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUGC *ugc = steamworks_ugc();
    if (!ugc) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    /* Steam requires an ABSOLUTE path to the content folder. */
    RETURN_BOOL(SteamAPI_ISteamUGC_SetItemContent(ugc, (UGCUpdateHandle_t)handle, folder));
}

PHP_FUNCTION(steam_ugc_set_item_preview)
{
    zend_long handle;
    char *file; size_t file_len;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(handle)
        Z_PARAM_STRING(file, file_len)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUGC *ugc = steamworks_ugc();
    if (!ugc) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    /* Absolute path to a preview image (< 1 MB, jpg/png/gif per Steam). */
    RETURN_BOOL(SteamAPI_ISteamUGC_SetItemPreview(ugc, (UGCUpdateHandle_t)handle, file));
}

PHP_FUNCTION(steam_ugc_submit_item_update)
{
    zend_long handle;
    char *note = NULL; size_t note_len = 0;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_LONG(handle)
        Z_PARAM_OPTIONAL
        Z_PARAM_STRING_OR_NULL(note, note_len)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUGC *ugc = steamworks_ugc();
    if (!ugc) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPICall_t call = SteamAPI_ISteamUGC_SubmitItemUpdate(
        ugc, (UGCUpdateHandle_t)handle, note /* may be NULL = no change note */);
    if (call == 0) {
        RETURN_FALSE;
    }
    steamworks_register_call(call, STEAMWORKS_CALL_UGC_SUBMIT_ITEM_UPDATE);
    RETURN_LONG((zend_long)call);
}

PHP_FUNCTION(steam_ugc_get_item_update_progress)
{
    zend_long handle;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(handle)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUGC *ugc = steamworks_ugc();
    if (!ugc) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    uint64_t processed = 0, total = 0;
    int status = SteamAPI_ISteamUGC_GetItemUpdateProgress(
        ugc, (UGCUpdateHandle_t)handle, &processed, &total);

    array_init(return_value);
    add_assoc_long(return_value, "status", (zend_long)status);
    add_assoc_long(return_value, "bytes_processed", (zend_long)processed);
    add_assoc_long(return_value, "bytes_total", (zend_long)total);
}
