# php-steamworks

Native PHP extension providing access to the [Steamworks SDK](https://partner.steamgames.com/) via a flat C API.

## Requirements

- PHP 8.1+
- Steamworks SDK 1.65 (download from [partner.steamgames.com](https://partner.steamgames.com/)) — see [SDK compatibility](#sdk-compatibility)
- macOS (arm64/x86_64), Linux (x86_64), or Windows (x86_64)

## Installation

### Via PIE (recommended)

[PIE](https://github.com/php/pie) is the official PHP extension installer.

```bash
pie install hmennen90/php-steamworks --with-steamworks=/path/to/steamworks-sdk
```

After installation, copy `libsteam_api.dylib` (macOS) or `libsteam_api.so` (Linux) from the SDK's `redistributable_bin/` directory next to the built extension:

```bash
# macOS
cp /path/to/steamworks-sdk/redistributable_bin/osx/libsteam_api.dylib "$(php-config --extension-dir)/"

# Linux x86_64
cp /path/to/steamworks-sdk/redistributable_bin/linux64/libsteam_api.so "$(php-config --extension-dir)/"
```

> **Note (Laravel Herd / custom PHP builds):** If PIE fails to auto-install the `.so` due to a read-only
> filesystem, the build output still exists at `~/.pie/php*/vendor/hmennen90/php-steamworks/modules/steamworks.so`.
> Copy it to your extension directory manually and use an absolute path in `php.ini`:
> ```ini
> extension=/full/path/to/steamworks.so
> ```

### Manual build

1. Download the Steamworks SDK and extract it.

2. Build the extension:

```bash
phpize
./configure --with-steamworks=/path/to/steamworks-sdk
make
make install
```

3. Copy the Steam shared library next to the extension (see PIE section above).

4. Enable the extension in `php.ini`:

```ini
extension=steamworks
```

See [INSTALL.md](INSTALL.md) for detailed platform-specific instructions.

## Quick Start

```php
<?php
// steam_appid.txt with your App-ID must exist in the working directory

if (!steam_init()) {
    die("Steam not running\n");
}

echo "Player: " . steam_friends_get_name() . "\n";
echo "App ID: " . steam_utils_get_app_id() . "\n";

// Game loop
while (true) {
    steam_run_callbacks();
    // ... game logic ...
    usleep(16667);
}

steam_shutdown();
```

## Available Functions

121 functions across 10 Steamworks interfaces. Full signatures with docblocks live in
[`stubs/steamworks.php`](stubs/steamworks.php) (IDE autocompletion).

### Core
- `steam_init(): bool`
- `steam_shutdown(): void`
- `steam_run_callbacks(): void` — call every frame

### User
- `steam_user_get_steam_id(): int|false`
- `steam_user_is_logged_on(): bool`
- `steam_user_get_player_steam_level(): int|false`
- `steam_user_get_auth_session_ticket(): array|false`
- `steam_user_begin_auth_session(string $ticket, int $steam_id): int|false`
- `steam_user_end_auth_session(int $steam_id): bool`
- `steam_user_cancel_auth_ticket(int $handle): bool`
- `steam_user_get_auth_ticket_for_web_api(?string $identity = null): int|false`
- `steam_user_get_web_api_ticket_result(int $handle): ?array` — `null` until the callback arrived

### Friends
- `steam_friends_get_name(): string|false`
- `steam_friends_set_rich_presence(string $key, ?string $value = null): bool`
- `steam_friends_activate_overlay(string $dialog): void`
- `steam_friends_activate_overlay_to_web_page(string $url, bool $modal = false): void`
- `steam_friends_get_persona_state(): int|false`
- `steam_friends_get_friend_count(int $flags = STEAM_FRIEND_FLAG_IMMEDIATE): int|false`
- `steam_friends_get_friend_by_index(int $index, int $flags = STEAM_FRIEND_FLAG_IMMEDIATE): int|false`
- `steam_friends_get_friend_relationship(int $steam_id): int|false`
- `steam_friends_get_friend_persona_state(int $steam_id): int|false`
- `steam_friends_get_friend_persona_name(int $steam_id): string|false`
- `steam_friends_request_user_information(int $steam_id, bool $name_only = false): bool`
- `steam_friends_get_friend_avatar(int $steam_id, int $size = STEAM_AVATAR_MEDIUM): ?array` — RGBA pixels

### Stats & Achievements
- `steam_stats_set_achievement(string $id): bool`
- `steam_stats_clear_achievement(string $id): bool`
- `steam_stats_store(): bool`
- `steam_stats_get_int(string $name): int|false`
- `steam_stats_set_int(string $name, int $value): bool`
- `steam_stats_get_float(string $name): float|false`
- `steam_stats_set_float(string $name, float $value): bool`
- `steam_stats_indicate_achievement_progress(string $id, int $cur, int $max): bool`
- `steam_stats_get_achievement(string $id): ?bool`
- `steam_stats_get_achievement_unlock_time(string $id): ?int`
- `steam_stats_get_num_achievements(): int|false`
- `steam_stats_get_achievement_name(int $index): string|false`
- `steam_stats_get_achievement_display_attribute(string $id, string $key): string|false`
- `steam_stats_reset_all_stats(bool $achievements_too = false): bool`

### Leaderboards (async — see below)
- `steam_stats_find_leaderboard(string $name): int|false`
- `steam_stats_find_or_create_leaderboard(string $name, int $sort, int $display): int|false`
- `steam_stats_upload_score(int $leaderboard, int $score, int $method = STEAM_LEADERBOARD_UPLOAD_KEEP_BEST, ?array $details = null): int|false` — `$details` = optional `int[]` game-specific values stored with the score (max `STEAM_LEADERBOARD_DETAILS_MAX` = 64)
- `steam_stats_download_leaderboard_entries(int $leaderboard, int $request, int $start, int $end): int|false`
- `steam_stats_get_downloaded_entry(int $entries, int $index): ?array` — returns `['steam_id' => int, 'global_rank' => int, 'score' => int, 'details' => int[], 'ugc' => int]`; `ugc` is the attached file, `STEAM_UGC_HANDLE_INVALID` if none
- `steam_stats_get_leaderboard_entry_count(int $leaderboard): int`
- `steam_stats_attach_leaderboard_ugc(int $leaderboard, int $ugc): int|false` — attach a shared file to your own entry (upload a score first) → `leaderboard_ugc_set`

### Async CallResults
- `steam_get_call_result(int $handle): array|false|null` — poll the result of an async call (`null` = pending, `false` = failed, `array` = result with a `type` field)

### Remote Storage (Cloud Saves)
- `steam_remote_file_write(string $filename, string $data): bool`
- `steam_remote_file_read(string $filename): string|false`
- `steam_remote_file_exists(string $filename): bool`
- `steam_remote_file_delete(string $filename): bool`
- `steam_remote_file_list(): array|false`

### Shared files (Remote Storage UGC)
A cloud file shared with `steam_remote_file_share` gets a UGC handle that can be attached to a leaderboard entry; anyone who downloads the entry can fetch the file.
- `steam_remote_file_share(string $filename): int|false` — async → `remote_file_shared` with `ugc`
- `steam_remote_ugc_download(int $ugc, int $priority = 0): int|false` — async → `remote_ugc_downloaded` with `size`, `name`, `owner`
- `steam_remote_ugc_read(int $ugc, int $size, int $offset = 0): string|false`
- `steam_remote_get_ugc_details(int $ugc): array|false` — `['app_id', 'name', 'size', 'owner']`, after the download completed

### Apps
- `steam_apps_is_subscribed(): bool`
- `steam_apps_is_dlc_installed(int $dlc_id): bool`
- `steam_apps_get_app_id(): int|false`
- `steam_apps_get_language(): string|false`
- `steam_apps_is_subscribed_app(int $app_id): bool`
- `steam_apps_get_current_beta_name(): string|false`
- `steam_apps_get_earliest_purchase_time(int $app_id): int|false`
- `steam_apps_get_installed_depots(int $app_id): array|false`
- `steam_apps_get_dlc_count(): int|false`
- `steam_apps_get_app_build_id(): int|false`

### Utils
- `steam_utils_get_app_id(): int|false`
- `steam_utils_is_overlay_enabled(): bool`
- `steam_utils_get_country_code(): string|false`
- `steam_utils_is_steam_deck(): bool` — **deprecated** since SDK 1.65, see below
- `steam_utils_get_hardware_type(): int|false` — `STEAM_HARDWARE_TYPE_*`
- `steam_utils_get_hardware_default_config(): int|false` — `STEAM_HARDWARE_CONFIG_*`
- `steam_utils_is_running_under_proton(): bool`
- `steam_utils_get_steam_ui_language(): string|false`
- `steam_utils_get_server_real_time(): int|false`
- `steam_utils_get_current_battery_power(): int|false` — 0–100 %, 255 = on AC power
- `steam_utils_get_seconds_since_app_active(): int|false`

### Timeline (Game Recording)

Annotate the Steam Game Recording timeline. Most calls are fire-and-forget (`bool`);
the two `does_*_recording_exist` calls are async (return a handle — poll `steam_get_call_result`).
Verified against Steamworks SDK 1.65 (`STEAMTIMELINE_INTERFACE_V004`).

- `steam_timeline_set_game_mode(int $mode): bool` — `STEAM_TIMELINE_GAME_MODE_*`
- `steam_timeline_set_tooltip(string $description, float $time_delta = 0.0): bool`
- `steam_timeline_clear_tooltip(float $time_delta = 0.0): bool`
- `steam_timeline_add_instantaneous_event(string $title, string $description, string $icon, int $icon_priority = 0, float $start_offset_seconds = 0.0, int $possible_clip = STEAM_TIMELINE_CLIP_PRIORITY_NONE): int`
- `steam_timeline_add_range_event(string $title, string $description, string $icon, int $icon_priority = 0, float $start_offset_seconds = 0.0, float $duration_seconds = 0.0, int $possible_clip = STEAM_TIMELINE_CLIP_PRIORITY_NONE): int`
- `steam_timeline_start_range_event(...): int` / `steam_timeline_update_range_event(int $event, ...): bool` / `steam_timeline_end_range_event(int $event, float $end_offset_seconds = 0.0): bool`
- `steam_timeline_remove_event(int $event): bool`
- `steam_timeline_does_event_recording_exist(int $event): int|false` — async (`type` = `timeline_event_recording_exists`)
- `steam_timeline_start_game_phase(): bool` / `steam_timeline_end_game_phase(): bool`
- `steam_timeline_set_game_phase_id(string $phase_id): bool`
- `steam_timeline_does_game_phase_recording_exist(string $phase_id): int|false` — async (`type` = `timeline_game_phase_recording_exists`)
- `steam_timeline_add_game_phase_tag(string $tag_name, string $tag_icon, string $tag_group, int $priority = 0): bool`
- `steam_timeline_set_game_phase_attribute(string $attribute_group, string $attribute_value, int $priority = 0): bool`
- `steam_timeline_open_overlay_to_game_phase(string $phase_id): bool` / `steam_timeline_open_overlay_to_event(int $event): bool`

### Workshop (UGC)
Subscribe/unsubscribe, create, submit and delete are async (poll `steam_get_call_result`).
- `steam_ugc_subscribe_item(int $file_id): int|false` / `steam_ugc_unsubscribe_item(int $file_id): int|false`
- `steam_ugc_get_num_subscribed_items(bool $include_locally_disabled = false): int|false`
- `steam_ugc_get_subscribed_items(bool $include_locally_disabled = false): array|false`
- `steam_ugc_get_item_state(int $file_id): int|false`
- `steam_ugc_get_item_install_info(int $file_id): array|false`
- `steam_ugc_get_item_download_info(int $file_id): array|false`
- `steam_ugc_download_item(int $file_id, bool $high_priority = false): bool`
- `steam_ugc_create_item(int $app_id, int $file_type = STEAM_UGC_FILE_TYPE_COMMUNITY): int|false`
- `steam_ugc_start_item_update(int $app_id, int $file_id): int`
- `steam_ugc_set_item_title(int $handle, string $title): bool`
- `steam_ugc_set_item_description(int $handle, string $description): bool`
- `steam_ugc_set_item_visibility(int $handle, int $visibility): bool`
- `steam_ugc_set_item_content(int $handle, string $content_folder): bool`
- `steam_ugc_set_item_preview(int $handle, string $preview_file): bool`
- `steam_ugc_set_item_tags(int $handle, array $tags): bool`
- `steam_ugc_submit_item_update(int $handle, ?string $change_note = null): int|false`
- `steam_ugc_get_item_update_progress(int $handle): array`
- `steam_ugc_delete_item(int $file_id): int|false`

### Networking (P2P over the Steam relay)
- `steam_net_init_relay_network_access(): bool`
- `steam_net_create_listen_socket_p2p(int $virtual_port = 0): int|false`
- `steam_net_connect_p2p(int $steam_id, int $virtual_port = 0): int|false`
- `steam_net_accept_connection(int $connection): int|false`
- `steam_net_close_connection(int $connection, int $reason = 0, ?string $debug = null, bool $linger = false): bool`
- `steam_net_send_message(int $connection, string $data, bool $reliable = true): int|false`
- `steam_net_receive_messages(int $connection, int $max = 32): array|false`
- `steam_net_get_connection_events(): array` — connection state changes since the last call

## Leaderboards & async calls

Leaderboard calls (and other `SteamAPICall_t`-based APIs) are **asynchronous**: they
return an integer handle. Poll `steam_get_call_result($handle)` each frame until it
stops returning `null`:

```php
$handle = steam_stats_find_leaderboard('HighScores');
do {
    steam_run_callbacks();
    $result = steam_get_call_result($handle);
    usleep(16667);
} while ($result === null);

// $result === ['type' => 'leaderboard_found', 'found' => true, 'leaderboard' => 42]
$leaderboard = $result['leaderboard'];
steam_stats_upload_score($leaderboard, 1500, STEAM_LEADERBOARD_UPLOAD_KEEP_BEST);
```

See [`examples/leaderboard.php`](examples/leaderboard.php) for a full round-trip.

### Constants
- Sort: `STEAM_LEADERBOARD_SORT_ASCENDING`, `STEAM_LEADERBOARD_SORT_DESCENDING`
- Display: `STEAM_LEADERBOARD_DISPLAY_NUMERIC`, `STEAM_LEADERBOARD_DISPLAY_TIME_SECONDS`, `STEAM_LEADERBOARD_DISPLAY_TIME_MILLISECONDS`
- Upload: `STEAM_LEADERBOARD_UPLOAD_KEEP_BEST`, `STEAM_LEADERBOARD_UPLOAD_FORCE_UPDATE`
- Download: `STEAM_LEADERBOARD_DATA_GLOBAL`, `STEAM_LEADERBOARD_DATA_GLOBAL_AROUND_USER`, `STEAM_LEADERBOARD_DATA_FRIENDS`
- Hardware type: `STEAM_HARDWARE_TYPE_NONE`, `STEAM_HARDWARE_TYPE_STEAM_DECK`, `STEAM_HARDWARE_TYPE_STEAM_MACHINE`, `STEAM_HARDWARE_TYPE_STEAM_FRAME`
- Hardware preset: `STEAM_HARDWARE_CONFIG_NONE`, `..._LOW`, `..._MEDIUM`, `..._HIGH`, `..._MAX`, `..._STEAM_DECK`, `..._STEAM_MACHINE`, `..._STEAM_FRAME`

## SDK compatibility

Built and verified against **Steamworks SDK 1.65**, which is the minimum: the
1.65-only `ISteamUtils` calls are link-time dependencies. `sdk/redistributable_bin/`
already ships the matching Steam runtime libraries, so a default build needs no
extra download.

Steam interfaces are resolved by version string at runtime rather than through
the SDK's versioned `SteamAPI_Steam*_vNNN()` accessor symbols. Those accessors
are version-locked link-time symbols, so every SDK release that bumps an
interface breaks the build outright — SDK 1.65 removed `SteamAPI_SteamUtils_v010`.
With the runtime lookup, an interface bump is a one-line change to a version
table instead.

### Old Steam clients

Where it is safe, an older interface version is accepted as a deprecated
fallback; using one raises `E_DEPRECATED` once per process.

| Interface | Current | Deprecated fallback |
|---|---|---|
| `ISteamNetworkingSockets` | `SteamNetworkingSockets013` (SDK 1.65+) | `SteamNetworkingSockets012` (SDK ≤ 1.64) |
| `ISteamUtils` | `SteamUtils011` (SDK 1.65+) | none — see below |

A fallback is only safe when the older interface has the **same vtable layout**:
the flat `SteamAPI_ISteamX_*` functions inside `libsteam_api` call fixed vtable
slots. SDK 1.65 removed two methods from the middle of `ISteamUtils`, so
`SteamUtils010` is not layout-compatible and is deliberately not accepted. A
Steam client that offers only the old interface gets a clear
`E_WARNING` ("the Steam client is older than this build") and a `false` return,
rather than a call into the wrong vtable slot.

Since Steam clients update themselves, this only affects clients that have not
run in a long time.

### Deprecated functions

`steam_utils_is_steam_deck()` still works — it is now implemented on
`IsRunningOnSteamHardware()` — but SDK 1.65 removed the underlying
`IsRunningOnSteamDeck()` call, and it raises `E_DEPRECATED`. Valve's guidance is
to avoid device checks entirely: use `steam_utils_get_hardware_default_config()`
for graphics defaults, or a capability-specific query, so the game keeps
behaving correctly on Steam hardware that did not exist when it shipped. Use
`steam_utils_get_hardware_type()` only for analytics and diagnostics.

## License

MIT
