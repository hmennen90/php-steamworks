# Changelog

## [0.11.0] - 2026-07-06 (unreleased)

Phase 3 — social, identity, Workshop & networking. All flat signatures, struct
layouts and callback IDs verified against the real Steamworks SDK 1.64, and the
whole surface runtime-verified against a live Steam client (App 480).

### Added
- **General callback subsystem** (`steam_callback.c`) — receives callbacks that
  arrive via RunCallbacks (not SteamAPICall_t results) by fabricating objects
  binary-compatible with the SDK's CCallbackBase and registering them with
  SteamAPI_RegisterCallback. Coexists with RunCallbacks, so the verified
  SteamAPICall_t path is untouched. Live-verified: the fabricated vtable dispatches.
- **ISteamUser — web-api auth ticket** (backend auth):
  `get_auth_ticket_for_web_api` + `get_web_api_ticket_result` (handle + poll;
  ticket delivered via GetTicketForWebApiResponse_t, callback id 168).
- **ISteamNetworkingSockets — P2P messaging core** (`steam_net.c`, v012):
  `init_relay_network_access`, `create_listen_socket_p2p`, `connect_p2p`,
  `accept_connection`, `close_connection`, `send_message`, `receive_messages`,
  and `get_connection_events` (drains SteamNetConnectionStatusChangedCallback_t,
  id 1221). New STEAM_NET_CONNECTION_STATE_* and STEAM_NET_SEND_* constants.
  Message/callback structs read via SDK-1.64-verified byte offsets.
- **ISteamFriends — friends list, persona state & avatars** (`steam_friends.c`):
  `get_persona_state`, `get_friend_count`/`get_friend_by_index` (EFriendFlags),
  `get_friend_relationship`, `get_friend_persona_state`/`get_friend_persona_name`,
  `request_user_information`, and `get_friend_avatar` (raw RGBA8888 bytes +
  dimensions via ISteamUtils GetImageSize/GetImageRGBA). New STEAM_PERSONA_STATE_*,
  STEAM_FRIEND_RELATIONSHIP_*, STEAM_FRIEND_FLAG_*, STEAM_AVATAR_* constants.
- **ISteamUser — auth session tickets** (`steam_user.c`):
  `get_auth_session_ticket` (returns `{handle, raw ticket bytes}`),
  `begin_auth_session` (verifies a peer ticket → EBeginAuthSessionResult),
  `end_auth_session`, `cancel_auth_ticket`. New STEAM_BEGIN_AUTH_SESSION_* constants.
- **ISteamUGC — Workshop consume path** (`steam_ugc.c`, V021):
  `subscribe_item`/`unsubscribe_item` (async → `ugc_subscribed`/`ugc_unsubscribed`),
  `get_num_subscribed_items`/`get_subscribed_items`, `get_item_state`
  (STEAM_UGC_ITEM_STATE_* bitflags), `get_item_install_info`,
  `get_item_download_info`, `download_item`.

### Deferred (documented follow-ups)
- ISteamUGC querying/browsing (CreateQuery* + GetQueryUGCResult / SteamUGCDetails_t) —
  the large SteamUGCDetails_t result struct; the consume path above covers subscribe/
  download/state.
- ISteamNetworkingSockets: listen-side accept flow beyond the basics, lanes, and
  connection real-time status/statistics.

## [0.10.0] - 2026-07-06

### Added
- **ISteamTimeline (Game Recording / Timeline, Phase 3)** — neues Modul
  `src/modules/steam_timeline.c` mit 18 Funktionen (voller Umfang inkl. Game Phases),
  modelliert auf SDK V004 (1.62/1.64):
  - Zustand: `steam_timeline_set_game_mode()`, `steam_timeline_set_tooltip()`,
    `steam_timeline_clear_tooltip()`.
  - Events: `steam_timeline_add_instantaneous_event()`, `steam_timeline_add_range_event()`,
    `steam_timeline_start_range_event()` / `steam_timeline_update_range_event()` /
    `steam_timeline_end_range_event()`, `steam_timeline_remove_event()` (geben ein
    Event-Handle zurück), plus asynchron `steam_timeline_does_event_recording_exist()`.
  - Game Phases: `steam_timeline_start_game_phase()` / `steam_timeline_end_game_phase()`,
    `steam_timeline_set_game_phase_id()`, `steam_timeline_add_game_phase_tag()`,
    `steam_timeline_set_game_phase_attribute()`, asynchron
    `steam_timeline_does_game_phase_recording_exist()`.
  - Overlay: `steam_timeline_open_overlay_to_game_phase()`, `steam_timeline_open_overlay_to_event()`.
  - Neue Konstanten `STEAM_TIMELINE_GAME_MODE_*` und `STEAM_TIMELINE_CLIP_PRIORITY_*`.
  - Die beiden asynchronen `does_*_recording_exist`-Calls sind in die vorhandene
    Handle+Poll-Infrastruktur eingehängt; `steam_get_call_result()` liefert die Typen
    `timeline_event_recording_exists` und `timeline_game_phase_recording_exists`.
- **Leaderboard Score-Details (`int32[]`)** — der zuvor zurückgestellte Follow-up:
  - `steam_stats_upload_score($leaderboard, $score, $method, $details = null)` — optionaler
    vierter Parameter `$details` (Array von ints), das mit dem Score gespeichert wird
    (z. B. Split-Zeiten, Waffen-IDs). Wird intern in einen `int32`-Buffer kopiert;
    mehr als `STEAM_LEADERBOARD_DETAILS_MAX` (64) Werte werden mit Warning verworfen.
  - `steam_stats_get_downloaded_entry()` liest die Details jetzt tatsächlich aus:
    Der Rückgabe-Schlüssel `details` ist nun ein **Array** der gespeicherten int32-Werte
    (vorher fälschlich nur die Anzahl `m_cDetails`). Leeres Array, wenn keine Details.
  - Neue Konstante `STEAM_LEADERBOARD_DETAILS_MAX` (= 64).

### Changed
- **BREAKING (nur Leaderboard-Details):** `steam_stats_get_downloaded_entry()` liefert unter
  `details` statt eines Integers (Detail-*Anzahl*) jetzt ein Integer-*Array* der Detailwerte.

### Verified
- **Score-Details:** Flat-Signaturen (`UploadLeaderboardScore`/`GetDownloadedLeaderboardEntry`
  mit `details`/`details_count`) und die Struct-Größen (`LeaderboardEntry_t`,
  `LeaderboardScoreUploaded_t`) gegen den Mock unter MSVC gebaut und per ABI-Cross-Check
  zwischen `src/steam_api_c.h` und dem Mock-Header bestätigt (pack(8) *und* pack(4)).
- **ISteamTimeline gegen echte Steamworks SDK 1.64 verifiziert** (STEAMTIMELINE_INTERFACE_V004),
  per C++-Probe gegen die realen Header (`isteamtimeline.h`, `steam_api_flat.h`,
  `steam_api_internal.h`) belegt:
  1. Accessor `SteamAPI_SteamTimeline_v004` ✓.
  2. Alle 18 Flat-Signaturen + Parameterreihenfolge stimmen 1:1 (steam_api_flat.h Z. 900–920).
  3. Struct-Layouts von `SteamTimelineEventRecordingExists_t` (16 B) /
     `SteamTimelineGamePhaseRecordingExists_t` (88 B) sowie `k_cchMaxPhaseIDLength` = 64 ✓.
  4. **Callback-IDs korrigiert:** Basis `k_iSteamTimelineCallbacks` ist **6000** (nicht 6600),
     also 6001 (GamePhase) / 6002 (Event) — beim Schätzen zunächst falsch, gegen die echte SDK
     gefixt (klassische ABI-Abweichung, die der Mock nicht fängt).

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
