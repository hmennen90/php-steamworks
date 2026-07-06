#include "../php_steamworks.h"
#include "../steam_api_c.h"

PHP_FUNCTION(steam_stats_set_achievement)
{
    zend_string *achievement_id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(achievement_id)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v013();
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

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v013();
    if (!stats) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamUserStats_ClearAchievement(stats, ZSTR_VAL(achievement_id)));
}

PHP_FUNCTION(steam_stats_store)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v013();
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

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v013();
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

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v013();
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

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v013();
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

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v013();
    if (!stats) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamUserStats_SetStatFloat(stats, ZSTR_VAL(name), (float)value));
}

PHP_FUNCTION(steam_stats_indicate_achievement_progress)
{
    zend_string *name;
    zend_long cur_progress;
    zend_long max_progress;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_STR(name)
        Z_PARAM_LONG(cur_progress)
        Z_PARAM_LONG(max_progress)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v013();
    if (!stats) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamUserStats_IndicateAchievementProgress(
        stats, ZSTR_VAL(name), (uint32)cur_progress, (uint32)max_progress));
}

/* ── Achievement read path ─────────────────────────────────────────────
 * All synchronous. Modern SteamAPI_Init() loads the current user's stats and
 * achievements automatically (the old RequestCurrentStats() was removed from
 * the SDK), so these can be called directly after a successful steam_init().
 */

PHP_FUNCTION(steam_stats_get_achievement)
{
    zend_string *name;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v013();
    if (!stats) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_NULL();
    }

    bool achieved = false;
    if (!SteamAPI_ISteamUserStats_GetAchievement(stats, ZSTR_VAL(name), &achieved)) {
        /* Unknown achievement id or stats not loaded yet. */
        RETURN_NULL();
    }

    RETURN_BOOL(achieved);
}

PHP_FUNCTION(steam_stats_get_achievement_unlock_time)
{
    zend_string *name;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v013();
    if (!stats) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_NULL();
    }

    bool   achieved = false;
    uint32 unlock_time = 0;
    if (!SteamAPI_ISteamUserStats_GetAchievementAndUnlockTime(
            stats, ZSTR_VAL(name), &achieved, &unlock_time) || !achieved) {
        /* Not unlocked (or unknown) → no unlock time. */
        RETURN_NULL();
    }

    RETURN_LONG((zend_long)unlock_time);
}

PHP_FUNCTION(steam_stats_get_num_achievements)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v013();
    if (!stats) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_LONG((zend_long)SteamAPI_ISteamUserStats_GetNumAchievements(stats));
}

PHP_FUNCTION(steam_stats_get_achievement_name)
{
    zend_long index;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(index)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v013();
    if (!stats) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    const char *name = SteamAPI_ISteamUserStats_GetAchievementName(stats, (uint32)index);
    if (!name || name[0] == '\0') {
        /* Out-of-range index. */
        RETURN_FALSE;
    }

    RETURN_STRING(name);
}

PHP_FUNCTION(steam_stats_get_achievement_display_attribute)
{
    zend_string *name;
    zend_string *key;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(name)
        Z_PARAM_STR(key)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v013();
    if (!stats) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    const char *value = SteamAPI_ISteamUserStats_GetAchievementDisplayAttribute(
        stats, ZSTR_VAL(name), ZSTR_VAL(key));
    RETURN_STRING(value ? value : "");
}

PHP_FUNCTION(steam_stats_reset_all_stats)
{
    zend_bool achievements_too = 0;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_BOOL(achievements_too)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v013();
    if (!stats) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_BOOL(SteamAPI_ISteamUserStats_ResetAllStats(stats, (bool)achievements_too));
}

/* ── Leaderboards ──────────────────────────────────────────────────────
 * These are asynchronous: each returns a SteamAPICall_t handle (as int).
 * Poll steam_get_call_result($handle) each frame until it stops returning null.
 */

PHP_FUNCTION(steam_stats_find_leaderboard)
{
    zend_string *name;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v013();
    if (!stats) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPICall_t call = SteamAPI_ISteamUserStats_FindLeaderboard(stats, ZSTR_VAL(name));
    if (call == 0) {
        RETURN_FALSE;
    }
    steamworks_register_call(call, STEAMWORKS_CALL_LEADERBOARD_FIND);
    RETURN_LONG((zend_long)call);
}

PHP_FUNCTION(steam_stats_find_or_create_leaderboard)
{
    zend_string *name;
    zend_long    sort_method;
    zend_long    display_type;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_STR(name)
        Z_PARAM_LONG(sort_method)
        Z_PARAM_LONG(display_type)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v013();
    if (!stats) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPICall_t call = SteamAPI_ISteamUserStats_FindOrCreateLeaderboard(
        stats, ZSTR_VAL(name),
        (ELeaderboardSortMethod)sort_method,
        (ELeaderboardDisplayType)display_type);
    if (call == 0) {
        RETURN_FALSE;
    }
    /* Same result struct as FindLeaderboard. */
    steamworks_register_call(call, STEAMWORKS_CALL_LEADERBOARD_FIND);
    RETURN_LONG((zend_long)call);
}

PHP_FUNCTION(steam_stats_upload_score)
{
    zend_long  leaderboard;
    zend_long  score;
    zend_long  method = k_ELeaderboardUploadScoreMethodKeepBest;
    HashTable *details_ht = NULL;

    ZEND_PARSE_PARAMETERS_START(2, 4)
        Z_PARAM_LONG(leaderboard)
        Z_PARAM_LONG(score)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(method)
        Z_PARAM_ARRAY_HT_OR_NULL(details_ht)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v013();
    if (!stats) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    /* Optional game-specific detail values (int32[]). Steam caps this at
       k_cLeaderboardDetailsMax; extra values are dropped with a warning. */
    int32 details[k_cLeaderboardDetailsMax];
    int   details_count = 0;
    if (details_ht) {
        zval *val;
        ZEND_HASH_FOREACH_VAL(details_ht, val) {
            if (details_count >= k_cLeaderboardDetailsMax) {
                php_error_docref(NULL, E_WARNING,
                    "Leaderboard score details truncated to %d values",
                    k_cLeaderboardDetailsMax);
                break;
            }
            details[details_count++] = (int32)zval_get_long(val);
        } ZEND_HASH_FOREACH_END();
    }

    SteamAPICall_t call = SteamAPI_ISteamUserStats_UploadLeaderboardScore(
        stats, (SteamLeaderboard_t)leaderboard,
        (ELeaderboardUploadScoreMethod)method,
        (int32)score,
        details_count > 0 ? details : NULL, details_count);
    if (call == 0) {
        RETURN_FALSE;
    }
    steamworks_register_call(call, STEAMWORKS_CALL_SCORE_UPLOADED);
    RETURN_LONG((zend_long)call);
}

PHP_FUNCTION(steam_stats_download_leaderboard_entries)
{
    zend_long leaderboard;
    zend_long request;
    zend_long range_start;
    zend_long range_end;

    ZEND_PARSE_PARAMETERS_START(4, 4)
        Z_PARAM_LONG(leaderboard)
        Z_PARAM_LONG(request)
        Z_PARAM_LONG(range_start)
        Z_PARAM_LONG(range_end)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v013();
    if (!stats) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPICall_t call = SteamAPI_ISteamUserStats_DownloadLeaderboardEntries(
        stats, (SteamLeaderboard_t)leaderboard,
        (ELeaderboardDataRequest)request,
        (int)range_start, (int)range_end);
    if (call == 0) {
        RETURN_FALSE;
    }
    steamworks_register_call(call, STEAMWORKS_CALL_SCORES_DOWNLOADED);
    RETURN_LONG((zend_long)call);
}

PHP_FUNCTION(steam_stats_get_downloaded_entry)
{
    zend_long entries;
    zend_long index;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(entries)
        Z_PARAM_LONG(index)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v013();
    if (!stats) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    LeaderboardEntry_t entry;
    int32              details[k_cLeaderboardDetailsMax];
    memset(&entry, 0, sizeof(entry));
    if (!SteamAPI_ISteamUserStats_GetDownloadedLeaderboardEntry(
            stats, (SteamLeaderboardEntries_t)entries, (int)index, &entry,
            details, k_cLeaderboardDetailsMax)) {
        RETURN_NULL();
    }

    array_init(return_value);
    add_assoc_long(return_value, "steam_id",    (zend_long)entry.m_steamIDUser);
    add_assoc_long(return_value, "global_rank", (zend_long)entry.m_nGlobalRank);
    add_assoc_long(return_value, "score",       (zend_long)entry.m_nScore);

    /* The int32 detail values uploaded alongside the score (empty array if none).
       m_cDetails reports how many were written into the buffer. */
    zval details_arr;
    array_init(&details_arr);
    int32 detail_count = entry.m_cDetails;
    if (detail_count > k_cLeaderboardDetailsMax) {
        detail_count = k_cLeaderboardDetailsMax;
    }
    for (int32 i = 0; i < detail_count; i++) {
        add_next_index_long(&details_arr, (zend_long)details[i]);
    }
    add_assoc_zval(return_value, "details", &details_arr);
}

PHP_FUNCTION(steam_stats_get_leaderboard_entry_count)
{
    zend_long leaderboard;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(leaderboard)
    ZEND_PARSE_PARAMETERS_END();

    ISteamUserStats *stats = SteamAPI_SteamUserStats_v013();
    if (!stats) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_LONG((zend_long)SteamAPI_ISteamUserStats_GetLeaderboardEntryCount(
        stats, (SteamLeaderboard_t)leaderboard));
}
