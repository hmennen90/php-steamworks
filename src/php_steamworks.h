#ifndef PHP_STEAMWORKS_H
#define PHP_STEAMWORKS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"

#define PHP_STEAMWORKS_EXTNAME  "steamworks"
#define PHP_STEAMWORKS_VERSION  "0.7.0"

extern zend_module_entry steamworks_module_entry;
#define phpext_steamworks_ptr &steamworks_module_entry

/* ── Async CallResult infrastructure (steam_async.c) ───────────────────── */
/* Kinds of pending SteamAPICall_t results the extension knows how to decode. */
enum steamworks_call_kind {
    STEAMWORKS_CALL_LEADERBOARD_FIND = 1, /* LeaderboardFindResult_t (also find_or_create) */
    STEAMWORKS_CALL_SCORE_UPLOADED,       /* LeaderboardScoreUploaded_t */
    STEAMWORKS_CALL_SCORES_DOWNLOADED,    /* LeaderboardScoresDownloaded_t */
};

void steamworks_async_minit(void);
void steamworks_async_mshutdown(void);
/* Track a pending async call so steam_get_call_result() can decode it later.
   handle is a SteamAPICall_t (uint64). */
void steamworks_register_call(uint64_t handle, enum steamworks_call_kind kind);

PHP_FUNCTION(steam_get_call_result);

/* steam_init.c */
PHP_FUNCTION(steam_init);
PHP_FUNCTION(steam_shutdown);
PHP_FUNCTION(steam_run_callbacks);

/* steam_user.c */
PHP_FUNCTION(steam_user_get_steam_id);

/* steam_friends.c */
PHP_FUNCTION(steam_friends_get_name);
PHP_FUNCTION(steam_friends_set_rich_presence);
PHP_FUNCTION(steam_friends_activate_overlay);
PHP_FUNCTION(steam_friends_activate_overlay_to_web_page);

/* steam_stats.c */
PHP_FUNCTION(steam_stats_set_achievement);
PHP_FUNCTION(steam_stats_clear_achievement);
PHP_FUNCTION(steam_stats_store);
PHP_FUNCTION(steam_stats_get_int);
PHP_FUNCTION(steam_stats_set_int);
PHP_FUNCTION(steam_stats_get_float);
PHP_FUNCTION(steam_stats_set_float);
PHP_FUNCTION(steam_stats_indicate_achievement_progress);
PHP_FUNCTION(steam_stats_request_current_stats);
PHP_FUNCTION(steam_stats_get_achievement);
PHP_FUNCTION(steam_stats_get_achievement_unlock_time);
PHP_FUNCTION(steam_stats_get_num_achievements);
PHP_FUNCTION(steam_stats_get_achievement_name);
PHP_FUNCTION(steam_stats_get_achievement_display_attribute);
PHP_FUNCTION(steam_stats_reset_all_stats);
PHP_FUNCTION(steam_stats_find_leaderboard);
PHP_FUNCTION(steam_stats_find_or_create_leaderboard);
PHP_FUNCTION(steam_stats_upload_score);
PHP_FUNCTION(steam_stats_download_leaderboard_entries);
PHP_FUNCTION(steam_stats_get_downloaded_entry);
PHP_FUNCTION(steam_stats_get_leaderboard_entry_count);

/* steam_remote.c */
PHP_FUNCTION(steam_remote_file_write);
PHP_FUNCTION(steam_remote_file_read);
PHP_FUNCTION(steam_remote_file_exists);
PHP_FUNCTION(steam_remote_file_delete);
PHP_FUNCTION(steam_remote_file_list);

/* steam_apps.c */
PHP_FUNCTION(steam_apps_is_subscribed);
PHP_FUNCTION(steam_apps_is_dlc_installed);
PHP_FUNCTION(steam_apps_get_app_id);
PHP_FUNCTION(steam_apps_get_language);

/* steam_utils.c */
PHP_FUNCTION(steam_utils_get_app_id);
PHP_FUNCTION(steam_utils_is_overlay_enabled);
PHP_FUNCTION(steam_utils_get_country_code);
PHP_FUNCTION(steam_utils_is_steam_deck);

#endif /* PHP_STEAMWORKS_H */
