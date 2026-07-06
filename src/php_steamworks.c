#include "php_steamworks.h"
#include "steam_api_c.h"  /* k_cLeaderboardDetailsMax for STEAM_LEADERBOARD_DETAILS_MAX */

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_void, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_one_string, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_two_strings, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_string_optional_bool, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, url, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, modal, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_one_long, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, id, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_stat_int, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_stat_float, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, value, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_indicate_progress, 0, 0, 3)
    ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, cur_progress, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, max_progress, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_remote_file_write, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_two_strings_req, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_optional_bool, 0, 0, 0)
    ZEND_ARG_TYPE_INFO(0, flag, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_lb_find_or_create, 0, 0, 3)
    ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, sort_method, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, display_type, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_lb_upload, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, leaderboard, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, score, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, method, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, details, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_lb_download, 0, 0, 4)
    ZEND_ARG_TYPE_INFO(0, leaderboard, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, request, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, range_start, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, range_end, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_two_longs, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, a, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, b, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_optional_long, 0, 0, 0)
    ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_long_optional_long, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, a, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, b, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_long_optional_bool, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, steam_id, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, flag, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_string_long, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, ticket, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, steam_id, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_optional_string, 0, 0, 0)
    ZEND_ARG_TYPE_INFO(0, identity, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_net_close, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, connection, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, reason, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, debug, IS_STRING, 1)
    ZEND_ARG_TYPE_INFO(0, linger, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_net_send, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, connection, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, reliable, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

/* ── ISteamTimeline arginfo ── */
ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_tl_set_tooltip, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, description, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, time_delta, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_tl_clear_tooltip, 0, 0, 0)
    ZEND_ARG_TYPE_INFO(0, time_delta, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_tl_add_instant, 0, 0, 3)
    ZEND_ARG_TYPE_INFO(0, title, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, description, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, icon, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, icon_priority, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, start_offset_seconds, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO(0, possible_clip, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_tl_add_range, 0, 0, 3)
    ZEND_ARG_TYPE_INFO(0, title, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, description, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, icon, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, icon_priority, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, start_offset_seconds, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO(0, duration_seconds, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO(0, possible_clip, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_tl_start_range, 0, 0, 3)
    ZEND_ARG_TYPE_INFO(0, title, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, description, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, icon, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, icon_priority, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, start_offset_seconds, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO(0, possible_clip, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_tl_update_range, 0, 0, 4)
    ZEND_ARG_TYPE_INFO(0, event, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, title, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, description, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, icon, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, icon_priority, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, possible_clip, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_tl_end_range, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, event, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, end_offset_seconds, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_tl_phase_tag, 0, 0, 3)
    ZEND_ARG_TYPE_INFO(0, tag_name, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, tag_icon, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, tag_group, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, priority, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_tl_phase_attr, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, attribute_group, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, attribute_value, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, priority, IS_LONG, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry steamworks_functions[] = {
    /* steam_init.c */
    PHP_FE(steam_init,                      arginfo_steam_void)
    PHP_FE(steam_shutdown,                  arginfo_steam_void)
    PHP_FE(steam_run_callbacks,             arginfo_steam_void)

    /* steam_user.c */
    PHP_FE(steam_user_get_steam_id,         arginfo_steam_void)
    PHP_FE(steam_user_is_logged_on,         arginfo_steam_void)
    PHP_FE(steam_user_get_player_steam_level, arginfo_steam_void)
    PHP_FE(steam_user_get_auth_session_ticket, arginfo_steam_void)
    PHP_FE(steam_user_begin_auth_session,   arginfo_steam_string_long)
    PHP_FE(steam_user_end_auth_session,     arginfo_steam_one_long)
    PHP_FE(steam_user_cancel_auth_ticket,   arginfo_steam_one_long)
    PHP_FE(steam_user_get_auth_ticket_for_web_api, arginfo_steam_optional_string)
    PHP_FE(steam_user_get_web_api_ticket_result, arginfo_steam_one_long)

    /* steam_friends.c */
    PHP_FE(steam_friends_get_name,          arginfo_steam_void)
    PHP_FE(steam_friends_set_rich_presence, arginfo_steam_two_strings)
    PHP_FE(steam_friends_activate_overlay,  arginfo_steam_one_string)
    PHP_FE(steam_friends_activate_overlay_to_web_page, arginfo_steam_string_optional_bool)
    PHP_FE(steam_friends_get_persona_state, arginfo_steam_void)
    PHP_FE(steam_friends_get_friend_count,  arginfo_steam_optional_long)
    PHP_FE(steam_friends_get_friend_by_index, arginfo_steam_long_optional_long)
    PHP_FE(steam_friends_get_friend_relationship, arginfo_steam_one_long)
    PHP_FE(steam_friends_get_friend_persona_state, arginfo_steam_one_long)
    PHP_FE(steam_friends_get_friend_persona_name, arginfo_steam_one_long)
    PHP_FE(steam_friends_request_user_information, arginfo_steam_long_optional_bool)
    PHP_FE(steam_friends_get_friend_avatar, arginfo_steam_long_optional_long)

    /* steam_stats.c */
    PHP_FE(steam_stats_set_achievement,     arginfo_steam_one_string)
    PHP_FE(steam_stats_clear_achievement,   arginfo_steam_one_string)
    PHP_FE(steam_stats_store,               arginfo_steam_void)
    PHP_FE(steam_stats_get_int,             arginfo_steam_one_string)
    PHP_FE(steam_stats_set_int,             arginfo_steam_stat_int)
    PHP_FE(steam_stats_get_float,           arginfo_steam_one_string)
    PHP_FE(steam_stats_set_float,           arginfo_steam_stat_float)
    PHP_FE(steam_stats_indicate_achievement_progress, arginfo_steam_indicate_progress)
    PHP_FE(steam_stats_get_achievement,     arginfo_steam_one_string)
    PHP_FE(steam_stats_get_achievement_unlock_time, arginfo_steam_one_string)
    PHP_FE(steam_stats_get_num_achievements, arginfo_steam_void)
    PHP_FE(steam_stats_get_achievement_name, arginfo_steam_one_long)
    PHP_FE(steam_stats_get_achievement_display_attribute, arginfo_steam_two_strings_req)
    PHP_FE(steam_stats_reset_all_stats,     arginfo_steam_optional_bool)
    PHP_FE(steam_stats_find_leaderboard,    arginfo_steam_one_string)
    PHP_FE(steam_stats_find_or_create_leaderboard, arginfo_steam_lb_find_or_create)
    PHP_FE(steam_stats_upload_score,        arginfo_steam_lb_upload)
    PHP_FE(steam_stats_download_leaderboard_entries, arginfo_steam_lb_download)
    PHP_FE(steam_stats_get_downloaded_entry, arginfo_steam_two_longs)
    PHP_FE(steam_stats_get_leaderboard_entry_count, arginfo_steam_one_long)

    /* steam_async.c */
    PHP_FE(steam_get_call_result,           arginfo_steam_one_long)

    /* steam_remote.c */
    PHP_FE(steam_remote_file_write,         arginfo_steam_remote_file_write)
    PHP_FE(steam_remote_file_read,          arginfo_steam_one_string)
    PHP_FE(steam_remote_file_exists,        arginfo_steam_one_string)
    PHP_FE(steam_remote_file_delete,        arginfo_steam_one_string)
    PHP_FE(steam_remote_file_list,          arginfo_steam_void)

    /* steam_apps.c */
    PHP_FE(steam_apps_is_subscribed,        arginfo_steam_void)
    PHP_FE(steam_apps_is_dlc_installed,     arginfo_steam_one_long)
    PHP_FE(steam_apps_get_app_id,           arginfo_steam_void)
    PHP_FE(steam_apps_get_language,         arginfo_steam_void)
    PHP_FE(steam_apps_is_subscribed_app,    arginfo_steam_one_long)
    PHP_FE(steam_apps_get_current_beta_name, arginfo_steam_void)
    PHP_FE(steam_apps_get_earliest_purchase_time, arginfo_steam_one_long)
    PHP_FE(steam_apps_get_installed_depots, arginfo_steam_one_long)
    PHP_FE(steam_apps_get_dlc_count,        arginfo_steam_void)
    PHP_FE(steam_apps_get_app_build_id,     arginfo_steam_void)

    /* steam_utils.c */
    PHP_FE(steam_utils_get_app_id,          arginfo_steam_void)
    PHP_FE(steam_utils_is_overlay_enabled,  arginfo_steam_void)
    PHP_FE(steam_utils_get_country_code,    arginfo_steam_void)
    PHP_FE(steam_utils_is_steam_deck,       arginfo_steam_void)
    PHP_FE(steam_utils_get_steam_ui_language, arginfo_steam_void)
    PHP_FE(steam_utils_get_server_real_time, arginfo_steam_void)
    PHP_FE(steam_utils_get_current_battery_power, arginfo_steam_void)
    PHP_FE(steam_utils_get_seconds_since_app_active, arginfo_steam_void)

    /* steam_timeline.c */
    PHP_FE(steam_timeline_set_game_mode,    arginfo_steam_one_long)
    PHP_FE(steam_timeline_set_tooltip,      arginfo_steam_tl_set_tooltip)
    PHP_FE(steam_timeline_clear_tooltip,    arginfo_steam_tl_clear_tooltip)
    PHP_FE(steam_timeline_add_instantaneous_event, arginfo_steam_tl_add_instant)
    PHP_FE(steam_timeline_add_range_event,  arginfo_steam_tl_add_range)
    PHP_FE(steam_timeline_start_range_event, arginfo_steam_tl_start_range)
    PHP_FE(steam_timeline_update_range_event, arginfo_steam_tl_update_range)
    PHP_FE(steam_timeline_end_range_event,  arginfo_steam_tl_end_range)
    PHP_FE(steam_timeline_remove_event,     arginfo_steam_one_long)
    PHP_FE(steam_timeline_does_event_recording_exist, arginfo_steam_one_long)
    PHP_FE(steam_timeline_start_game_phase, arginfo_steam_void)
    PHP_FE(steam_timeline_end_game_phase,   arginfo_steam_void)
    PHP_FE(steam_timeline_set_game_phase_id, arginfo_steam_one_string)
    PHP_FE(steam_timeline_does_game_phase_recording_exist, arginfo_steam_one_string)
    PHP_FE(steam_timeline_add_game_phase_tag, arginfo_steam_tl_phase_tag)
    PHP_FE(steam_timeline_set_game_phase_attribute, arginfo_steam_tl_phase_attr)
    PHP_FE(steam_timeline_open_overlay_to_game_phase, arginfo_steam_one_string)
    PHP_FE(steam_timeline_open_overlay_to_event, arginfo_steam_one_long)

    /* steam_ugc.c */
    PHP_FE(steam_ugc_subscribe_item,        arginfo_steam_one_long)
    PHP_FE(steam_ugc_unsubscribe_item,      arginfo_steam_one_long)
    PHP_FE(steam_ugc_get_num_subscribed_items, arginfo_steam_optional_bool)
    PHP_FE(steam_ugc_get_subscribed_items,  arginfo_steam_optional_bool)
    PHP_FE(steam_ugc_get_item_state,        arginfo_steam_one_long)
    PHP_FE(steam_ugc_get_item_install_info, arginfo_steam_one_long)
    PHP_FE(steam_ugc_get_item_download_info, arginfo_steam_one_long)
    PHP_FE(steam_ugc_download_item,         arginfo_steam_long_optional_bool)

    /* steam_callback.c / steam_net.c */
    PHP_FE(steam_net_get_connection_events, arginfo_steam_void)
    PHP_FE(steam_net_init_relay_network_access, arginfo_steam_void)
    PHP_FE(steam_net_create_listen_socket_p2p, arginfo_steam_optional_long)
    PHP_FE(steam_net_connect_p2p,           arginfo_steam_long_optional_long)
    PHP_FE(steam_net_accept_connection,     arginfo_steam_one_long)
    PHP_FE(steam_net_close_connection,      arginfo_steam_net_close)
    PHP_FE(steam_net_send_message,          arginfo_steam_net_send)
    PHP_FE(steam_net_receive_messages,      arginfo_steam_long_optional_long)

    PHP_FE_END
};

PHP_MINFO_FUNCTION(steamworks)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "Steamworks SDK Support", "enabled");
    php_info_print_table_row(2, "Extension Version", PHP_STEAMWORKS_VERSION);
    php_info_print_table_end();
}

PHP_MINIT_FUNCTION(steamworks)
{
    steamworks_async_minit();
    steamworks_callbacks_minit();

    /* Leaderboard enum constants (mirror ELeaderboard* in the SDK). */
    REGISTER_LONG_CONSTANT("STEAM_LEADERBOARD_SORT_ASCENDING",  1, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_LEADERBOARD_SORT_DESCENDING", 2, CONST_CS | CONST_PERSISTENT);

    REGISTER_LONG_CONSTANT("STEAM_LEADERBOARD_DISPLAY_NUMERIC",           1, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_LEADERBOARD_DISPLAY_TIME_SECONDS",      2, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_LEADERBOARD_DISPLAY_TIME_MILLISECONDS", 3, CONST_CS | CONST_PERSISTENT);

    REGISTER_LONG_CONSTANT("STEAM_LEADERBOARD_UPLOAD_KEEP_BEST",    1, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_LEADERBOARD_UPLOAD_FORCE_UPDATE", 2, CONST_CS | CONST_PERSISTENT);

    REGISTER_LONG_CONSTANT("STEAM_LEADERBOARD_DATA_GLOBAL",             0, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_LEADERBOARD_DATA_GLOBAL_AROUND_USER", 1, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_LEADERBOARD_DATA_FRIENDS",            2, CONST_CS | CONST_PERSISTENT);

    /* Max int32 detail values per leaderboard entry (SDK k_cLeaderboardDetailsMax). */
    REGISTER_LONG_CONSTANT("STEAM_LEADERBOARD_DETAILS_MAX", k_cLeaderboardDetailsMax, CONST_CS | CONST_PERSISTENT);

    /* ISteamTimeline: game modes (ETimelineGameMode). */
    REGISTER_LONG_CONSTANT("STEAM_TIMELINE_GAME_MODE_INVALID",        k_ETimelineGameMode_Invalid,       CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_TIMELINE_GAME_MODE_PLAYING",        k_ETimelineGameMode_Playing,       CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_TIMELINE_GAME_MODE_STAGING",        k_ETimelineGameMode_Staging,       CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_TIMELINE_GAME_MODE_MENUS",          k_ETimelineGameMode_Menus,         CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_TIMELINE_GAME_MODE_LOADING_SCREEN", k_ETimelineGameMode_LoadingScreen, CONST_CS | CONST_PERSISTENT);

    /* ISteamTimeline: event clip priority (ETimelineEventClipPriority). */
    REGISTER_LONG_CONSTANT("STEAM_TIMELINE_CLIP_PRIORITY_INVALID",  k_ETimelineEventClipPriority_Invalid,  CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_TIMELINE_CLIP_PRIORITY_NONE",     k_ETimelineEventClipPriority_None,     CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_TIMELINE_CLIP_PRIORITY_STANDARD", k_ETimelineEventClipPriority_Standard, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_TIMELINE_CLIP_PRIORITY_FEATURED", k_ETimelineEventClipPriority_Featured, CONST_CS | CONST_PERSISTENT);

    /* ISteamFriends: persona state (EPersonaState). */
    REGISTER_LONG_CONSTANT("STEAM_PERSONA_STATE_OFFLINE",          0, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_PERSONA_STATE_ONLINE",           1, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_PERSONA_STATE_BUSY",             2, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_PERSONA_STATE_AWAY",             3, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_PERSONA_STATE_SNOOZE",           4, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_PERSONA_STATE_LOOKING_TO_TRADE", 5, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_PERSONA_STATE_LOOKING_TO_PLAY",  6, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_PERSONA_STATE_INVISIBLE",        7, CONST_CS | CONST_PERSISTENT);

    /* ISteamFriends: friend relationship (EFriendRelationship). */
    REGISTER_LONG_CONSTANT("STEAM_FRIEND_RELATIONSHIP_NONE",              0, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_FRIEND_RELATIONSHIP_BLOCKED",           1, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_FRIEND_RELATIONSHIP_REQUEST_RECIPIENT", 2, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_FRIEND_RELATIONSHIP_FRIEND",            3, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_FRIEND_RELATIONSHIP_REQUEST_INITIATOR", 4, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_FRIEND_RELATIONSHIP_IGNORED",           5, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_FRIEND_RELATIONSHIP_IGNORED_FRIEND",    6, CONST_CS | CONST_PERSISTENT);

    /* ISteamFriends: friend flags for get_friend_count/get_friend_by_index (EFriendFlags). */
    REGISTER_LONG_CONSTANT("STEAM_FRIEND_FLAG_NONE",           0x00,   CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_FRIEND_FLAG_IMMEDIATE",      0x04,   CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_FRIEND_FLAG_CLAN_MEMBER",    0x08,   CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_FRIEND_FLAG_ON_GAME_SERVER", 0x10,   CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_FRIEND_FLAG_ALL",            0xFFFF, CONST_CS | CONST_PERSISTENT);

    /* Avatar sizes for steam_friends_get_friend_avatar(). */
    REGISTER_LONG_CONSTANT("STEAM_AVATAR_SMALL",  0, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_AVATAR_MEDIUM", 1, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_AVATAR_LARGE",  2, CONST_CS | CONST_PERSISTENT);

    /* ISteamUser: BeginAuthSession result (EBeginAuthSessionResult). */
    REGISTER_LONG_CONSTANT("STEAM_BEGIN_AUTH_SESSION_OK",                0, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_BEGIN_AUTH_SESSION_INVALID_TICKET",    1, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_BEGIN_AUTH_SESSION_DUPLICATE_REQUEST", 2, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_BEGIN_AUTH_SESSION_INVALID_VERSION",   3, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_BEGIN_AUTH_SESSION_GAME_MISMATCH",     4, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_BEGIN_AUTH_SESSION_EXPIRED_TICKET",    5, CONST_CS | CONST_PERSISTENT);

    /* ISteamUGC: item state bitflags for steam_ugc_get_item_state() (EItemState). */
    REGISTER_LONG_CONSTANT("STEAM_UGC_ITEM_STATE_NONE",             0,  CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_UGC_ITEM_STATE_SUBSCRIBED",       1,  CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_UGC_ITEM_STATE_LEGACY_ITEM",      2,  CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_UGC_ITEM_STATE_INSTALLED",        4,  CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_UGC_ITEM_STATE_NEEDS_UPDATE",     8,  CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_UGC_ITEM_STATE_DOWNLOADING",      16, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_UGC_ITEM_STATE_DOWNLOAD_PENDING", 32, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_UGC_ITEM_STATE_DISABLED_LOCALLY", 64, CONST_CS | CONST_PERSISTENT);

    /* ISteamNetworkingSockets: connection state (ESteamNetworkingConnectionState). */
    REGISTER_LONG_CONSTANT("STEAM_NET_CONNECTION_STATE_NONE",                     0, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_NET_CONNECTION_STATE_CONNECTING",               1, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_NET_CONNECTION_STATE_FINDING_ROUTE",            2, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_NET_CONNECTION_STATE_CONNECTED",                3, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_NET_CONNECTION_STATE_CLOSED_BY_PEER",           4, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_NET_CONNECTION_STATE_PROBLEM_DETECTED_LOCALLY", 5, CONST_CS | CONST_PERSISTENT);

    /* Send flags for steam_net_send_message(). */
    REGISTER_LONG_CONSTANT("STEAM_NET_SEND_UNRELIABLE", 0, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("STEAM_NET_SEND_RELIABLE",   8, CONST_CS | CONST_PERSISTENT);

    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(steamworks)
{
    steamworks_async_mshutdown();
    steamworks_callbacks_mshutdown();
    return SUCCESS;
}

zend_module_entry steamworks_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_STEAMWORKS_EXTNAME,
    steamworks_functions,
    PHP_MINIT(steamworks),
    PHP_MSHUTDOWN(steamworks),
    NULL, /* RINIT */
    NULL, /* RSHUTDOWN */
    PHP_MINFO(steamworks),
    PHP_STEAMWORKS_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_STEAMWORKS
ZEND_GET_MODULE(steamworks)
#endif
