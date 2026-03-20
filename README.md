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

### Core
- `steam_init(): bool`
- `steam_shutdown(): void`
- `steam_run_callbacks(): void`

### User
- `steam_user_get_steam_id(): int|false`

### Friends
- `steam_friends_get_name(): string|false`
- `steam_friends_set_rich_presence(string $key, ?string $value = null): bool`
- `steam_friends_activate_overlay(string $dialog): void`

### Stats & Achievements
- `steam_stats_set_achievement(string $id): bool`
- `steam_stats_clear_achievement(string $id): bool`
- `steam_stats_store(): bool`
- `steam_stats_get_int(string $name): int|false`
- `steam_stats_set_int(string $name, int $value): bool`
- `steam_stats_get_float(string $name): float|false`
- `steam_stats_set_float(string $name, float $value): bool`

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

### Utils
- `steam_utils_get_app_id(): int|false`
- `steam_utils_is_overlay_enabled(): bool`
- `steam_utils_get_country_code(): string|false`

## License

MIT
