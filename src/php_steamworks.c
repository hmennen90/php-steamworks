#include "php_steamworks.h"

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

static const zend_function_entry steamworks_functions[] = {
    /* steam_init.c */
    PHP_FE(steam_init,                      arginfo_steam_void)
    PHP_FE(steam_shutdown,                  arginfo_steam_void)
    PHP_FE(steam_run_callbacks,             arginfo_steam_void)

    /* steam_user.c */
    PHP_FE(steam_user_get_steam_id,         arginfo_steam_void)
    PHP_FE(steam_user_is_logged_on,         arginfo_steam_void)
    PHP_FE(steam_user_get_player_steam_level, arginfo_steam_void)

    /* steam_friends.c */
    PHP_FE(steam_friends_get_name,          arginfo_steam_void)
    PHP_FE(steam_friends_set_rich_presence, arginfo_steam_two_strings)
    PHP_FE(steam_friends_activate_overlay,  arginfo_steam_one_string)
    PHP_FE(steam_friends_activate_overlay_to_web_page, arginfo_steam_string_optional_bool)

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

    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(steamworks)
{
    steamworks_async_mshutdown();
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
