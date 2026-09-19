<?php

use PHPUnit\Framework\TestCase;

/**
 * Tests for runtime Steam interface resolution.
 *
 * Interfaces are resolved by version string at runtime instead of through the
 * SDK's version-specific accessor symbols, so one binary keeps working across
 * SDK releases: the newest supported version is requested first, with older
 * versions kept as a deprecated fallback.
 *
 * The mock SDK is the oracle — it answers only the exact version strings it
 * knows and reports anything else on stderr as "unknown interface version".
 *
 * Each case runs in its own subprocess so the once-per-process deprecation
 * notice and the mock's environment switches stay isolated.
 */
class SteamInterfaceTest extends TestCase
{
    /**
     * Interface version strings the extension must request, copied from the
     * SDK's *_INTERFACE_VERSION defines in Steamworks SDK 1.65.
     *
     * These are NOT derivable from the accessor name: ISteamApps is
     * "STEAMAPPS_INTERFACE_VERSION009", not "SteamApps009". Getting one wrong
     * makes the interface silently unavailable at runtime, which is why the
     * mock rejects anything it does not recognise.
     */
    private const EXPECTED_VERSIONS = [
        'ISteamUser'               => 'SteamUser023',
        'ISteamFriends'            => 'SteamFriends018',
        'ISteamUserStats'          => 'STEAMUSERSTATS_INTERFACE_VERSION013',
        'ISteamRemoteStorage'      => 'STEAMREMOTESTORAGE_INTERFACE_VERSION016',
        'ISteamApps'               => 'STEAMAPPS_INTERFACE_VERSION009',
        'ISteamUtils'              => 'SteamUtils011',
        'ISteamTimeline'           => 'STEAMTIMELINE_INTERFACE_V004',
        'ISteamUGC'                => 'STEAMUGC_INTERFACE_VERSION021',
        'ISteamNetworkingSockets'  => 'SteamNetworkingSockets013',
        'ISteamNetworkingUtils'    => 'SteamNetworkingUtils004',
        'ISteamMatchmaking'        => 'SteamMatchMaking009',
    ];

    /**
     * Legacy versions kept working as a deprecated fallback.
     *
     * A version may only appear here when its vtable layout is identical to
     * the current one — the flat SteamAPI_ISteamX_* functions in libsteam_api
     * call fixed vtable slots. SteamUtils010 is deliberately absent: SDK 1.65
     * removed two methods from the middle of ISteamUtils, so an old client must
     * fail cleanly rather than call the wrong slots.
     */
    private const LEGACY_VERSIONS = [
        'ISteamNetworkingSockets'  => 'SteamNetworkingSockets012',
    ];

    private static function extensionPath(): string
    {
        $fromEnv = getenv('STEAMWORKS_EXT');
        if (is_string($fromEnv) && $fromEnv !== '') {
            return $fromEnv;
        }

        $root = dirname(__DIR__);
        foreach (['modules/steamworks.so', 'x64/Release/php_steamworks.dll', 'x64/Release_TS/php_steamworks.dll'] as $candidate) {
            if (file_exists($root . '/' . $candidate)) {
                return $root . '/' . $candidate;
            }
        }

        return '';
    }

    /**
     * Runs PHP code in a fresh process with the extension loaded.
     *
     * @param array<string, string> $env extra environment variables
     * @return array{stdout: string, stderr: string, exit: int}
     */
    private static function runPhp(string $code, array $env = []): array
    {
        $process = proc_open(
            [
                PHP_BINARY,
                '-n',
                '-d', 'extension=' . self::extensionPath(),
                '-d', 'display_errors=stderr',
                '-d', 'error_reporting=' . E_ALL,
                '-r', $code,
            ],
            [1 => ['pipe', 'w'], 2 => ['pipe', 'w']],
            $pipes,
            dirname(__DIR__),
            array_merge(getenv(), $env)
        );

        if (!is_resource($process)) {
            throw new RuntimeException('Could not start PHP subprocess');
        }

        $stdout = stream_get_contents($pipes[1]);
        $stderr = stream_get_contents($pipes[2]);
        fclose($pipes[1]);
        fclose($pipes[2]);

        return ['stdout' => $stdout, 'stderr' => $stderr, 'exit' => proc_close($process)];
    }

    /** Calls at least one function per interface so every interface resolves. */
    private static function callEveryInterface(): string
    {
        return implode(' ', [
            '@steam_init();',
            '@steam_user_get_steam_id();',
            '@steam_friends_get_name();',
            '@steam_stats_store();',
            '@steam_remote_file_exists("f");',
            '@steam_apps_is_subscribed();',
            '@steam_utils_get_app_id();',
            '@steam_timeline_set_game_mode(1);',
            '@steam_ugc_subscribe_item(1);',
            '@steam_net_init_relay_network_access();',
            '@steam_net_create_listen_socket_p2p(0);',
        ]);
    }

    protected function setUp(): void
    {
        if (self::extensionPath() === '' || !file_exists(self::extensionPath())) {
            $this->markTestSkipped('Built extension not found; set STEAMWORKS_EXT to its path.');
        }
    }

    public function testExtensionLoadsInSubprocess(): void
    {
        $result = self::runPhp('echo extension_loaded("steamworks") ? "yes" : "no";');

        $this->assertSame('yes', $result['stdout'], 'stderr: ' . $result['stderr']);
    }

    /**
     * Catches a wrong or stale version string. Without this the mistake is
     * silent: resolution returns NULL and the function returns its normal
     * failure value, exactly as it would when Steam is not running.
     */
    public function testAllInterfacesResolveToKnownVersions(): void
    {
        $result = self::runPhp(self::callEveryInterface());

        $this->assertStringNotContainsString(
            'unknown interface version',
            $result['stderr'],
            'Extension requested an interface version the SDK does not provide'
        );
    }

    /**
     * The version strings live in src/steam_iface.c. They must be copied from
     * the SDK headers verbatim — see EXPECTED_VERSIONS.
     */
    public function testVersionTablesMatchTheSdkStrings(): void
    {
        $source = file_get_contents(dirname(__DIR__) . '/src/steam_iface.c');
        $this->assertIsString($source, 'src/steam_iface.c is missing');

        foreach (self::EXPECTED_VERSIONS as $interface => $version) {
            $this->assertStringContainsString(
                '"' . $version . '"',
                $source,
                "$interface must request \"$version\""
            );
        }

        foreach (self::LEGACY_VERSIONS as $interface => $version) {
            $this->assertStringContainsString(
                '"' . $version . '"',
                $source,
                "$interface must keep \"$version\" as a deprecated fallback"
            );
        }
    }

    public function testModernUtilsInterfaceIsPreferred(): void
    {
        $result = self::runPhp('echo steam_utils_get_app_id();');

        $this->assertSame('480', $result['stdout'], 'stderr: ' . $result['stderr']);
        $this->assertStringNotContainsString('Deprecated', $result['stderr']);
    }

    /**
     * SteamUtils010 is not vtable-compatible with the flat functions in
     * libsteam_api 1.65, so a client that offers only the old interface must
     * fail cleanly — never resolve it and call the wrong vtable slots.
     */
    public function testClientWithOnlyLegacyUtilsFailsCleanly(): void
    {
        $result = self::runPhp(
            'var_dump(@steam_utils_get_app_id());',
            ['STEAMWORKS_MOCK_LEGACY_UTILS' => '1']
        );

        $this->assertSame(0, $result['exit'], 'Must not crash. stderr: ' . $result['stderr']);
        $this->assertSame("bool(false)\n", $result['stdout']);
    }

    public function testClientWithOnlyLegacyUtilsExplainsWhy(): void
    {
        $result = self::runPhp(
            'steam_utils_get_app_id();',
            ['STEAMWORKS_MOCK_LEGACY_UTILS' => '1']
        );

        $this->assertMatchesRegularExpression(
            '/Steam client is older than this build/s',
            $result['stderr'],
            'An outdated client must be reported as such, not as "Steam not initialized"'
        );
    }

    public function testLegacyNetworkingSocketsInterfaceStillWorks(): void
    {
        $result = self::runPhp(
            '@steam_net_create_listen_socket_p2p(0); echo "done";',
            ['STEAMWORKS_MOCK_LEGACY_NET' => '1']
        );

        $this->assertSame('done', $result['stdout'], 'stderr: ' . $result['stderr']);
    }

    public function testLegacyNetworkingSocketsInterfaceIsMarkedDeprecated(): void
    {
        $result = self::runPhp(
            'steam_net_create_listen_socket_p2p(0);',
            ['STEAMWORKS_MOCK_LEGACY_NET' => '1']
        );

        $this->assertMatchesRegularExpression(
            '/Deprecated:.*SteamNetworkingSockets012/s',
            $result['stderr']
        );
    }

    /**
     * A per-call notice would fire every frame from the game loop, so the
     * deprecation is raised only once per process.
     */
    public function testLegacyDeprecationIsRaisedOnlyOncePerProcess(): void
    {
        $result = self::runPhp(
            'steam_net_create_listen_socket_p2p(0); steam_net_create_listen_socket_p2p(1); steam_net_create_listen_socket_p2p(2);',
            ['STEAMWORKS_MOCK_LEGACY_NET' => '1']
        );

        $this->assertSame(
            1,
            substr_count($result['stderr'], 'Deprecated:'),
            'Expected exactly one deprecation notice, got: ' . $result['stderr']
        );
    }

    /**
     * Optional cross-check against a real SDK checkout. Skipped in CI, where
     * the NDA-protected headers are not available.
     */
    public function testVersionStringsMatchARealSdkWhenAvailable(): void
    {
        $sdk = getenv('STEAMWORKS_SDK_DIR');
        if (!is_string($sdk) || $sdk === '' || !is_dir($sdk . '/public/steam')) {
            $this->markTestSkipped('Set STEAMWORKS_SDK_DIR to a Steamworks SDK to run this check.');
        }

        $headers = implode("\n", array_map(
            static fn (string $f): string => (string) file_get_contents($f),
            glob($sdk . '/public/steam/*.h') ?: []
        ));

        foreach (self::EXPECTED_VERSIONS as $interface => $version) {
            $this->assertStringContainsString(
                '"' . $version . '"',
                $headers,
                "$interface version \"$version\" not found in the SDK at $sdk"
            );
        }
    }
}
