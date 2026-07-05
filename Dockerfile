# Local CI verification for php-steamworks.
#
# Builds the extension against the mock Steam SDK (ci/mock_sdk/) and runs the
# exact same verification suite as the Linux GitHub Actions job — so you can
# catch failures locally before pushing and save Action minutes.
#
#   docker build -t php-steamworks-ci .
#
# A successful build means every check passed. To poke around a built image:
#
#   docker run --rm -it php-steamworks-ci bash
#
# PHP 8.5 matches the version pinned in .github/workflows/ci.yml.
FROM php:8.5-cli

# phpize/build toolchain. $PHPIZE_DEPS pulls in autoconf, gcc, make, libc-dev.
RUN apt-get update \
    && apt-get install -y --no-install-recommends $PHPIZE_DEPS curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /ext

# Copy sources needed for the build. .dockerignore keeps the context small.
COPY . .

# Build the extension against the mock SDK (no real Steamworks SDK required).
RUN phpize \
    && ./configure --with-steamworks --enable-steamworks-mock \
    && make -j"$(nproc)"

ENV EXT=/ext/modules/steamworks.so

# 1. Extension loads (hard-fails if it does not).
RUN php -dextension=$EXT -m | grep -q steamworks \
    && php -dextension=$EXT -r "if (!extension_loaded('steamworks')) { fwrite(STDERR, 'steamworks extension failed to load' . PHP_EOL); exit(1); } echo 'steamworks extension loaded' . PHP_EOL;"

# 2. All registered functions exist.
RUN php -dextension=$EXT -r "\
    \$functions = [\
      'steam_init', 'steam_shutdown', 'steam_run_callbacks',\
      'steam_user_get_steam_id',\
      'steam_friends_get_name', 'steam_friends_set_rich_presence', 'steam_friends_activate_overlay', 'steam_friends_activate_overlay_to_web_page',\
      'steam_stats_set_achievement', 'steam_stats_clear_achievement', 'steam_stats_store',\
      'steam_stats_get_int', 'steam_stats_set_int', 'steam_stats_get_float', 'steam_stats_set_float',\
      'steam_stats_indicate_achievement_progress',\
      'steam_stats_find_leaderboard', 'steam_stats_find_or_create_leaderboard',\
      'steam_stats_upload_score', 'steam_stats_download_leaderboard_entries',\
      'steam_stats_get_downloaded_entry', 'steam_stats_get_leaderboard_entry_count',\
      'steam_get_call_result',\
      'steam_remote_file_write', 'steam_remote_file_read', 'steam_remote_file_exists',\
      'steam_remote_file_delete', 'steam_remote_file_list',\
      'steam_apps_is_subscribed', 'steam_apps_is_dlc_installed',\
      'steam_apps_get_app_id', 'steam_apps_get_language',\
      'steam_utils_get_app_id', 'steam_utils_is_overlay_enabled', 'steam_utils_get_country_code',\
      'steam_utils_is_steam_deck',\
    ];\
    \$ok = true;\
    foreach (\$functions as \$fn) { if (!function_exists(\$fn)) { echo \"MISSING: \$fn\n\"; \$ok = false; } }\
    if (\$ok) echo 'All ' . count(\$functions) . \" functions registered.\n\"; else exit(1);"

# 3. steam_init() returns false without a running Steam client (no fatal).
RUN php -dextension=$EXT -r "\
    \$result = @steam_init();\
    assert(\$result === false, 'steam_init() should return false without Steam');\
    echo \"steam_init() correctly returned false\n\";"

# 4. Mock API returns the deterministic test values.
RUN php -dextension=$EXT -r "\
    echo 'Player name: ' . steam_friends_get_name() . \"\n\";\
    echo 'App ID: ' . steam_utils_get_app_id() . \"\n\";\
    echo 'Country: ' . steam_utils_get_country_code() . \"\n\";\
    echo 'Language: ' . steam_apps_get_language() . \"\n\";\
    assert(steam_friends_get_name() === 'MockPlayer');\
    assert(steam_utils_get_app_id() === 480);\
    assert(steam_utils_get_country_code() === 'US');\
    echo \"All mock responses correct.\n\";"

# 5. PHPUnit suite.
RUN curl -sL https://phar.phpunit.de/phpunit-10.phar -o phpunit.phar \
    && php -dextension=$EXT phpunit.phar --bootstrap tests/bootstrap.php --testdox tests/

CMD ["bash"]
