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
}
