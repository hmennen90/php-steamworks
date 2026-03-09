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

ZEND_BEGIN_ARG_INFO_EX(arginfo_steam_remote_file_write, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry steamworks_functions[] = {
    /* steam_init.c */
    PHP_FE(steam_init,                      arginfo_steam_void)
    PHP_FE(steam_shutdown,                  arginfo_steam_void)
    PHP_FE(steam_run_callbacks,             arginfo_steam_void)

    /* steam_user.c */
    PHP_FE(steam_user_get_steam_id,         arginfo_steam_void)

    /* steam_friends.c */
    PHP_FE(steam_friends_get_name,          arginfo_steam_void)
    PHP_FE(steam_friends_set_rich_presence, arginfo_steam_two_strings)
    PHP_FE(steam_friends_activate_overlay,  arginfo_steam_one_string)

    /* steam_stats.c */
    PHP_FE(steam_stats_set_achievement,     arginfo_steam_one_string)
    PHP_FE(steam_stats_clear_achievement,   arginfo_steam_one_string)
    PHP_FE(steam_stats_store,               arginfo_steam_void)
    PHP_FE(steam_stats_get_int,             arginfo_steam_one_string)
    PHP_FE(steam_stats_set_int,             arginfo_steam_stat_int)
    PHP_FE(steam_stats_get_float,           arginfo_steam_one_string)
    PHP_FE(steam_stats_set_float,           arginfo_steam_stat_float)

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

    /* steam_utils.c */
    PHP_FE(steam_utils_get_app_id,          arginfo_steam_void)
    PHP_FE(steam_utils_is_overlay_enabled,  arginfo_steam_void)
    PHP_FE(steam_utils_get_country_code,    arginfo_steam_void)

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
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(steamworks)
{
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
