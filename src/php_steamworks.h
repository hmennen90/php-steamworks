#ifndef PHP_STEAMWORKS_H
#define PHP_STEAMWORKS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"

#define PHP_STEAMWORKS_EXTNAME  "steamworks"
#define PHP_STEAMWORKS_VERSION  "0.11.0"

extern zend_module_entry steamworks_module_entry;
#define phpext_steamworks_ptr &steamworks_module_entry

/* ── Async CallResult infrastructure (steam_async.c) ───────────────────── */
/* Kinds of pending SteamAPICall_t results the extension knows how to decode. */
enum steamworks_call_kind {
    STEAMWORKS_CALL_LEADERBOARD_FIND = 1, /* LeaderboardFindResult_t (also find_or_create) */
    STEAMWORKS_CALL_SCORE_UPLOADED,       /* LeaderboardScoreUploaded_t */
    STEAMWORKS_CALL_SCORES_DOWNLOADED,    /* LeaderboardScoresDownloaded_t */
    STEAMWORKS_CALL_TIMELINE_EVENT_RECORDING, /* SteamTimelineEventRecordingExists_t */
    STEAMWORKS_CALL_TIMELINE_PHASE_RECORDING, /* SteamTimelineGamePhaseRecordingExists_t */
    STEAMWORKS_CALL_UGC_SUBSCRIBE,            /* RemoteStorageSubscribePublishedFileResult_t */
    STEAMWORKS_CALL_UGC_UNSUBSCRIBE,          /* RemoteStorageUnsubscribePublishedFileResult_t */
};

void steamworks_async_minit(void);
void steamworks_async_mshutdown(void);
/* Track a pending async call so steam_get_call_result() can decode it later.
   handle is a SteamAPICall_t (uint64). */
void steamworks_register_call(uint64_t handle, enum steamworks_call_kind kind);

PHP_FUNCTION(steam_get_call_result);

/* ── General callback subsystem (steam_callback.c) ─────────────────────────── */
void steamworks_callbacks_minit(void);       /* init storage (MINIT) */
void steamworks_callbacks_mshutdown(void);   /* free storage (MSHUTDOWN) */
void steamworks_callbacks_register(void);    /* register CCallbacks after steam_init */
void steamworks_callbacks_unregister(void);  /* unregister before steam_shutdown */

PHP_FUNCTION(steam_user_get_auth_ticket_for_web_api);
PHP_FUNCTION(steam_user_get_web_api_ticket_result);
PHP_FUNCTION(steam_net_get_connection_events); /* steam_callback.c */

/* steam_net.c */
PHP_FUNCTION(steam_net_init_relay_network_access);
PHP_FUNCTION(steam_net_create_listen_socket_p2p);
PHP_FUNCTION(steam_net_connect_p2p);
PHP_FUNCTION(steam_net_accept_connection);
PHP_FUNCTION(steam_net_close_connection);
PHP_FUNCTION(steam_net_send_message);
PHP_FUNCTION(steam_net_receive_messages);

/* steam_init.c */
PHP_FUNCTION(steam_init);
PHP_FUNCTION(steam_shutdown);
PHP_FUNCTION(steam_run_callbacks);

/* steam_user.c */
PHP_FUNCTION(steam_user_get_steam_id);
PHP_FUNCTION(steam_user_is_logged_on);
PHP_FUNCTION(steam_user_get_player_steam_level);
PHP_FUNCTION(steam_user_get_auth_session_ticket);
PHP_FUNCTION(steam_user_begin_auth_session);
PHP_FUNCTION(steam_user_end_auth_session);
PHP_FUNCTION(steam_user_cancel_auth_ticket);

/* steam_friends.c */
PHP_FUNCTION(steam_friends_get_name);
PHP_FUNCTION(steam_friends_set_rich_presence);
PHP_FUNCTION(steam_friends_activate_overlay);
PHP_FUNCTION(steam_friends_activate_overlay_to_web_page);
PHP_FUNCTION(steam_friends_get_persona_state);
PHP_FUNCTION(steam_friends_get_friend_count);
PHP_FUNCTION(steam_friends_get_friend_by_index);
PHP_FUNCTION(steam_friends_get_friend_relationship);
PHP_FUNCTION(steam_friends_get_friend_persona_state);
PHP_FUNCTION(steam_friends_get_friend_persona_name);
PHP_FUNCTION(steam_friends_request_user_information);
PHP_FUNCTION(steam_friends_get_friend_avatar);

/* steam_stats.c */
PHP_FUNCTION(steam_stats_set_achievement);
PHP_FUNCTION(steam_stats_clear_achievement);
PHP_FUNCTION(steam_stats_store);
PHP_FUNCTION(steam_stats_get_int);
PHP_FUNCTION(steam_stats_set_int);
PHP_FUNCTION(steam_stats_get_float);
PHP_FUNCTION(steam_stats_set_float);
PHP_FUNCTION(steam_stats_indicate_achievement_progress);
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
PHP_FUNCTION(steam_apps_is_subscribed_app);
PHP_FUNCTION(steam_apps_get_current_beta_name);
PHP_FUNCTION(steam_apps_get_earliest_purchase_time);
PHP_FUNCTION(steam_apps_get_installed_depots);
PHP_FUNCTION(steam_apps_get_dlc_count);
PHP_FUNCTION(steam_apps_get_app_build_id);

/* steam_utils.c */
PHP_FUNCTION(steam_utils_get_app_id);
PHP_FUNCTION(steam_utils_is_overlay_enabled);
PHP_FUNCTION(steam_utils_get_country_code);
PHP_FUNCTION(steam_utils_is_steam_deck);
PHP_FUNCTION(steam_utils_get_steam_ui_language);
PHP_FUNCTION(steam_utils_get_server_real_time);
PHP_FUNCTION(steam_utils_get_current_battery_power);
PHP_FUNCTION(steam_utils_get_seconds_since_app_active);

/* steam_timeline.c */
PHP_FUNCTION(steam_timeline_set_game_mode);
PHP_FUNCTION(steam_timeline_set_tooltip);
PHP_FUNCTION(steam_timeline_clear_tooltip);
PHP_FUNCTION(steam_timeline_add_instantaneous_event);
PHP_FUNCTION(steam_timeline_add_range_event);
PHP_FUNCTION(steam_timeline_start_range_event);
PHP_FUNCTION(steam_timeline_update_range_event);
PHP_FUNCTION(steam_timeline_end_range_event);
PHP_FUNCTION(steam_timeline_remove_event);
PHP_FUNCTION(steam_timeline_does_event_recording_exist);
PHP_FUNCTION(steam_timeline_start_game_phase);
PHP_FUNCTION(steam_timeline_end_game_phase);
PHP_FUNCTION(steam_timeline_set_game_phase_id);
PHP_FUNCTION(steam_timeline_does_game_phase_recording_exist);
PHP_FUNCTION(steam_timeline_add_game_phase_tag);
PHP_FUNCTION(steam_timeline_set_game_phase_attribute);
PHP_FUNCTION(steam_timeline_open_overlay_to_game_phase);
PHP_FUNCTION(steam_timeline_open_overlay_to_event);

/* steam_ugc.c */
PHP_FUNCTION(steam_ugc_subscribe_item);
PHP_FUNCTION(steam_ugc_unsubscribe_item);
PHP_FUNCTION(steam_ugc_get_num_subscribed_items);
PHP_FUNCTION(steam_ugc_get_subscribed_items);
PHP_FUNCTION(steam_ugc_get_item_state);
PHP_FUNCTION(steam_ugc_get_item_install_info);
PHP_FUNCTION(steam_ugc_get_item_download_info);
PHP_FUNCTION(steam_ugc_download_item);

#endif /* PHP_STEAMWORKS_H */
