# Changelog

## [0.1.0] - 2026-03-09

### Added
- Initial project structure
- Phase 1 functions: `steam_init()`, `steam_shutdown()`, `steam_run_callbacks()`
- ISteamUser: `steam_user_get_steam_id()`
- ISteamFriends: `steam_friends_get_name()`, `steam_friends_set_rich_presence()`, `steam_friends_activate_overlay()`
- ISteamUserStats: `steam_stats_set_achievement()`, `steam_stats_clear_achievement()`, `steam_stats_store()`, `steam_stats_get_int()`, `steam_stats_set_int()`, `steam_stats_get_float()`, `steam_stats_set_float()`
- ISteamRemoteStorage: `steam_remote_file_write()`, `steam_remote_file_read()`, `steam_remote_file_exists()`, `steam_remote_file_delete()`, `steam_remote_file_list()`
- ISteamApps: `steam_apps_is_subscribed()`, `steam_apps_is_dlc_installed()`, `steam_apps_get_app_id()`, `steam_apps_get_language()`
- ISteamUtils: `steam_utils_get_app_id()`, `steam_utils_is_overlay_enabled()`, `steam_utils_get_country_code()`
- Build configuration for macOS, Linux (`config.m4`) and Windows (`config.w32`)
- PHP stubs for IDE autocompletion
- PHPUnit tests for extension loading and function existence
- Examples: basic_init, achievements, cloud_save
