# php-steamworks

Native PHP extension providing access to the [Steamworks SDK](https://partner.steamgames.com/) via a flat C API.

## Requirements

- PHP 8.1+
- Steamworks SDK 1.58+ (download from [partner.steamgames.com](https://partner.steamgames.com/))
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

54 functions across 6 Steamworks interfaces. Full signatures with docblocks live in
[`stubs/steamworks.php`](stubs/steamworks.php) (IDE autocompletion).

### Core
- `steam_init(): bool`
- `steam_shutdown(): void`
- `steam_run_callbacks(): void` — call every frame

### User
- `steam_user_get_steam_id(): int|false`
- `steam_user_is_logged_on(): bool`
- `steam_user_get_player_steam_level(): int|false`

### Friends
- `steam_friends_get_name(): string|false`
- `steam_friends_set_rich_presence(string $key, ?string $value = null): bool`
- `steam_friends_activate_overlay(string $dialog): void`
- `steam_friends_activate_overlay_to_web_page(string $url, bool $modal = false): void`

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
- `steam_stats_upload_score(int $leaderboard, int $score, int $method = STEAM_LEADERBOARD_UPLOAD_KEEP_BEST): int|false`
- `steam_stats_download_leaderboard_entries(int $leaderboard, int $request, int $start, int $end): int|false`
- `steam_stats_get_downloaded_entry(int $entries, int $index): ?array`
- `steam_stats_get_leaderboard_entry_count(int $leaderboard): int`

### Async CallResults
- `steam_get_call_result(int $handle): array|false|null` — poll the result of an async call (`null` = pending, `false` = failed, `array` = result with a `type` field)

### Remote Storage (Cloud Saves)
- `steam_remote_file_write(string $filename, string $data): bool`
- `steam_remote_file_read(string $filename): string|false`
- `steam_remote_file_exists(string $filename): bool`
- `steam_remote_file_delete(string $filename): bool`
- `steam_remote_file_list(): array|false`

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
- `steam_utils_is_steam_deck(): bool`
- `steam_utils_get_steam_ui_language(): string|false`
- `steam_utils_get_server_real_time(): int|false`
- `steam_utils_get_current_battery_power(): int|false` — 0–100 %, 255 = on AC power
- `steam_utils_get_seconds_since_app_active(): int|false`

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

## License

MIT
