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
