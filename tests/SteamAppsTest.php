<?php

use PHPUnit\Framework\TestCase;

class SteamAppsTest extends TestCase
{
    public function testIsSubscribedExists(): void
    {
        $this->assertTrue(function_exists('steam_apps_is_subscribed'));
    }

    public function testIsDlcInstalledExists(): void
    {
        $this->assertTrue(function_exists('steam_apps_is_dlc_installed'));
    }

    public function testGetAppIdExists(): void
    {
        $this->assertTrue(function_exists('steam_apps_get_app_id'));
    }

    public function testGetLanguageExists(): void
    {
        $this->assertTrue(function_exists('steam_apps_get_language'));
    }

    public function testIsSubscribedReturnsBoolWithoutSteam(): void
    {
        // Mock/no-Steam environment: returns false, never fatals.
        $this->assertIsBool(@steam_apps_is_subscribed());
    }

    public function testIsDlcInstalledReturnsBoolWithoutSteam(): void
    {
        // Mock/no-Steam environment: returns false, never fatals.
        $this->assertIsBool(@steam_apps_is_dlc_installed(12345));
    }

    public function testGetLanguageReturnsStringOrFalse(): void
    {
        // Mock build returns a language string; a real build without Steam
        // returns false. Either way it must never fatal.
        $lang = @steam_apps_get_language();
        $this->assertTrue(is_string($lang) || $lang === false);
    }
}
