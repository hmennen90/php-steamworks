# Changelog

## [0.6.0] - 2026-07-05

### Added
- **Asynchrone CallResult-Infrastruktur (Handle + Poll):** `steam_get_call_result($handle)` holt das Ergebnis asynchroner Steam-Calls ab (`null` = pending/unbekannt, `false` = fehlgeschlagen, `array` mit `type`-Feld = fertig). Fundament für alle `SteamAPICall_t`-basierten APIs. Neues Modul `src/modules/steam_async.c`.
- **ISteamUserStats: Leaderboards** (Phase 2) — asynchron, geben ein Call-Handle zurück:
  - `steam_stats_find_leaderboard($name)` — `FindLeaderboard`
  - `steam_stats_find_or_create_leaderboard($name, $sort, $display)` — `FindOrCreateLeaderboard`
  - `steam_stats_upload_score($leaderboard, $score, $method = STEAM_LEADERBOARD_UPLOAD_KEEP_BEST)` — `UploadLeaderboardScore`
  - `steam_stats_download_leaderboard_entries($leaderboard, $request, $start, $end)` — `DownloadLeaderboardEntries`
  - `steam_stats_get_downloaded_entry($entries, $index)` — synchroner Zugriff auf einen Eintrag (`GetDownloadedLeaderboardEntry`)
  - `steam_stats_get_leaderboard_entry_count($leaderboard)` — synchron (`GetLeaderboardEntryCount`)
- **Leaderboard-Konstanten:** `STEAM_LEADERBOARD_SORT_*`, `STEAM_LEADERBOARD_DISPLAY_*`, `STEAM_LEADERBOARD_UPLOAD_*`, `STEAM_LEADERBOARD_DATA_*`.

### Notes
- Score-`details`-Arrays (`int32[]`) sind in dieser Version noch nicht unterstützt (Follow-up).
- Nicht gepollte Call-Handles verbleiben bis Prozessende im internen Registry (in CLI unkritisch).

## [0.5.0] - 2026-07-05

### Added
- ISteamUtils: `steam_utils_is_steam_deck()` — wraps `IsSteamRunningOnSteamDeck`, for handheld-friendly defaults (fullscreen, larger UI scale)
- ISteamUserStats: `steam_stats_indicate_achievement_progress($id, $cur, $max)` — wraps `IndicateAchievementProgress`, shows native "47 / 100" progress toasts for staged achievements. Requires a Progress Stat configured for the achievement in the partner backend; do not call with `$cur >= $max` (use `steam_stats_set_achievement()` there instead).

## [0.2.2] - 2026-04-04

### Added
- ISteamFriends: `steam_friends_activate_overlay_to_web_page($url, $modal)` — opens a URL in the Steam Overlay browser

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
