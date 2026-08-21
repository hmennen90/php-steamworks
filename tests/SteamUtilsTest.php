<?php

use PHPUnit\Framework\TestCase;

class SteamUtilsTest extends TestCase
{
    public function testGetAppIdExists(): void
    {
        $this->assertTrue(function_exists('steam_utils_get_app_id'));
    }

    public function testIsOverlayEnabledExists(): void
    {
        $this->assertTrue(function_exists('steam_utils_is_overlay_enabled'));
    }

    public function testGetCountryCodeExists(): void
    {
        $this->assertTrue(function_exists('steam_utils_get_country_code'));
    }

    public function testIsSteamDeckExists(): void
    {
        $this->assertTrue(function_exists('steam_utils_is_steam_deck'));
    }

    public function testIsSteamDeckReturnsBoolWithoutSteam(): void
    {
        // Mock/no-Steam environment: returns false, never fatals.
        $this->assertIsBool(@steam_utils_is_steam_deck());
    }

    public function testGetSteamUiLanguageExists(): void
    {
        $this->assertTrue(function_exists('steam_utils_get_steam_ui_language'));
    }

    public function testGetServerRealTimeExists(): void
    {
        $this->assertTrue(function_exists('steam_utils_get_server_real_time'));
    }

    public function testGetCurrentBatteryPowerExists(): void
    {
        $this->assertTrue(function_exists('steam_utils_get_current_battery_power'));
    }

    public function testGetSecondsSinceAppActiveExists(): void
    {
        $this->assertTrue(function_exists('steam_utils_get_seconds_since_app_active'));
    }

    public function testGetSteamUiLanguageReturnsStringOrFalse(): void
    {
        $lang = @steam_utils_get_steam_ui_language();
        $this->assertTrue(is_string($lang) || $lang === false);
    }

    public function testGetCurrentBatteryPowerReturnsIntOrFalse(): void
    {
        // 0-100 = percentage, 255 = on AC power.
        $power = @steam_utils_get_current_battery_power();
        $this->assertTrue(is_int($power) || $power === false);
    }

    public function testGetServerRealTimeReturnsIntOrFalse(): void
    {
        $time = @steam_utils_get_server_real_time();
        $this->assertTrue(is_int($time) || $time === false);
    }

    /**
     * SDK 1.65 removed ISteamUtils::IsRunningOnSteamDeck(). The PHP function
     * stays, reimplemented on IsRunningOnSteamHardware(), but is deprecated in
     * favour of the more specific hardware queries Valve now recommends.
     */
    public function testIsSteamDeckIsDeprecated(): void
    {
        $raised = null;
        set_error_handler(static function (int $errno, string $errstr) use (&$raised): bool {
            $raised = $errstr;
            return true;
        }, E_DEPRECATED);

        try {
            steam_utils_is_steam_deck();
        } finally {
            restore_error_handler();
        }

        $this->assertNotNull($raised, 'steam_utils_is_steam_deck() must raise E_DEPRECATED');
        $this->assertStringContainsString('steam_utils_get_hardware_type', (string) $raised);
    }

    public function testIsSteamDeckReflectsHardwareType(): void
    {
        // The mock reports a Steam Deck, so the reimplementation must agree.
        $this->assertTrue(@steam_utils_is_steam_deck());
        $this->assertSame(STEAM_HARDWARE_TYPE_STEAM_DECK, @steam_utils_get_hardware_type());
    }

    public function testGetHardwareTypeExists(): void
    {
        $this->assertTrue(function_exists('steam_utils_get_hardware_type'));
    }

    public function testGetHardwareTypeReturnsIntOrFalse(): void
    {
        $type = @steam_utils_get_hardware_type();
        $this->assertTrue(is_int($type) || $type === false);
    }

    public function testIsRunningUnderProtonExists(): void
    {
        $this->assertTrue(function_exists('steam_utils_is_running_under_proton'));
    }

    public function testIsRunningUnderProtonReturnsBool(): void
    {
        $this->assertIsBool(@steam_utils_is_running_under_proton());
    }

    public function testGetHardwareDefaultConfigExists(): void
    {
        $this->assertTrue(function_exists('steam_utils_get_hardware_default_config'));
    }

    public function testGetHardwareDefaultConfigReturnsIntOrFalse(): void
    {
        $config = @steam_utils_get_hardware_default_config();
        $this->assertTrue(is_int($config) || $config === false);
    }

    public function testHardwareConstantsAreRegistered(): void
    {
        $this->assertSame(0, STEAM_HARDWARE_TYPE_NONE);
        $this->assertSame(1, STEAM_HARDWARE_TYPE_STEAM_DECK);
        $this->assertSame(2, STEAM_HARDWARE_TYPE_STEAM_MACHINE);
        $this->assertSame(3, STEAM_HARDWARE_TYPE_STEAM_FRAME);

        $this->assertSame(0, STEAM_HARDWARE_CONFIG_NONE);
        $this->assertSame(1, STEAM_HARDWARE_CONFIG_LOW);
        $this->assertSame(2, STEAM_HARDWARE_CONFIG_MEDIUM);
        $this->assertSame(3, STEAM_HARDWARE_CONFIG_HIGH);
        $this->assertSame(4, STEAM_HARDWARE_CONFIG_MAX);
        $this->assertSame(5, STEAM_HARDWARE_CONFIG_STEAM_DECK);
        $this->assertSame(6, STEAM_HARDWARE_CONFIG_STEAM_MACHINE);
        $this->assertSame(7, STEAM_HARDWARE_CONFIG_STEAM_FRAME);
    }
}
