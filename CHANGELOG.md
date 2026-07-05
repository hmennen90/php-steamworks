# Changelog

## [0.9.0] - 2026-07-05

### Fixed
- **Cross-Platform-ABI der CallResult-Structs:** Die Leaderboard-Result-Structs
  nutzten fest `#pragma pack(8)`. Die echte SDK packt jedoch mit `pack(4)` auf
  Linux/macOS/FreeBSD und nur `pack(8)` auf Windows. Dadurch hätte
  `steam_get_call_result()` auf Linux/macOS die Felder von
  `LeaderboardScoreUploaded_t` (uint8 zuerst) an falschen Offsets gelesen
  (score/rank falsch). Jetzt plattform-abhängiges Packing wie in der SDK.
  Gefunden bei der Verifikation gegen Steamworks SDK 1.64 (Mock kann diesen
  Fall nicht fangen).

### Removed
- **`steam_stats_request_current_stats()`** — `RequestCurrentStats` wurde aus der
  Steamworks SDK entfernt (in 1.64 auskommentiert) und hätte gegen die echte SDK
  einen Linker-Fehler verursacht. Moderne `SteamAPI_Init()` lädt Stats/Achievements
  ohnehin automatisch; Achievements können direkt nach `steam_init()` gelesen werden.

### Verified
- Gesamte v0.6.0–v0.8.0-API gegen echte Steamworks SDK 1.64 gegengeprüft:
  Accessor-Versionen (v023/v018/v013/v016/v009/v010), Flat-Signaturen,
  Leaderboard-Callback-IDs (1104–1106) und Struct-Feld-Layouts stimmen überein.
- **Runtime gegen echtes Steam getestet** (Windows, ZTS/VS17-Build gegen die echte
  `steam_api64`, App 480/Spacewar): `steam_init()` verbindet, alle synchronen Getter
  liefern echte Werte, und der Async-Leaderboard-Roundtrip (find → poll → download →
  entry) dekodiert echte Live-Daten korrekt (z.B. „Quickest Win" 97k Einträge,
  Ränge 1–5) — bestätigt Callback-IDs und Struct-Packing auf Windows End-to-End.

## [0.8.0] - 2026-07-05

### Added
- **ISteamApps** (Phase 2d) — synchrone Getter:
  - `steam_apps_is_subscribed_app($app_id)` — `BIsSubscribedApp`
  - `steam_apps_get_current_beta_name()` — `GetCurrentBetaName` (string oder false)
  - `steam_apps_get_earliest_purchase_time($app_id)` — `GetEarliestPurchaseUnixTime`
  - `steam_apps_get_installed_depots($app_id)` — `GetInstalledDepots`, Array der Depot-IDs
  - `steam_apps_get_dlc_count()` — `GetDLCCount`
  - `steam_apps_get_app_build_id()` — `GetAppBuildId`
- **ISteamUtils** (Phase 2d):
  - `steam_utils_get_steam_ui_language()` — `GetSteamUILanguage`
  - `steam_utils_get_server_real_time()` — `GetServerRealTime` (Unix-Timestamp)
  - `steam_utils_get_current_battery_power()` — `GetCurrentBatteryPower` (0–100 %, 255 = Netzteil)
  - `steam_utils_get_seconds_since_app_active()` — `GetSecondsSinceAppActive`
- **ISteamUser** (Phase 2d):
  - `steam_user_is_logged_on()` — `BLoggedOn`
  - `steam_user_get_player_steam_level()` — `GetPlayerSteamLevel`

## [0.7.0] - 2026-07-05

### Added
- **ISteamUserStats: Achievement-Lesepfad** (Phase 2a) — alle synchron:
  - `steam_stats_request_current_stats()` — `RequestCurrentStats`, lädt Stats/Achievements des Nutzers (moderne Inits tun das automatisch, hier explizit)
  - `steam_stats_get_achievement($id)` — `GetAchievement`, `?bool` (true/false/null bei unbekannt)
  - `steam_stats_get_achievement_unlock_time($id)` — `GetAchievementAndUnlockTime`, Unix-Timestamp oder null
  - `steam_stats_get_num_achievements()` — `GetNumAchievements`
  - `steam_stats_get_achievement_name($index)` — `GetAchievementName`, zum Enumerieren
  - `steam_stats_get_achievement_display_attribute($id, $key)` — `GetAchievementDisplayAttribute` ("name"/"desc"/"hidden")
  - `steam_stats_reset_all_stats($achievements_too = false)` — `ResetAllStats`

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
