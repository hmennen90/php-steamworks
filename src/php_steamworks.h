#ifndef PHP_STEAMWORKS_H
#define PHP_STEAMWORKS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"

#define PHP_STEAMWORKS_EXTNAME  "steamworks"
#define PHP_STEAMWORKS_VERSION  "0.1.0"

extern zend_module_entry steamworks_module_entry;
#define phpext_steamworks_ptr &steamworks_module_entry

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

/* steam_stats.c */
PHP_FUNCTION(steam_stats_set_achievement);
PHP_FUNCTION(steam_stats_clear_achievement);
PHP_FUNCTION(steam_stats_store);
PHP_FUNCTION(steam_stats_get_int);
PHP_FUNCTION(steam_stats_set_int);
PHP_FUNCTION(steam_stats_get_float);
PHP_FUNCTION(steam_stats_set_float);

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

#endif /* PHP_STEAMWORKS_H */
