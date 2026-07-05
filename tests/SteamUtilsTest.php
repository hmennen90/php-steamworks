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
}
