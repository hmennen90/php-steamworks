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

    public function testIsSubscribedAppExists(): void
    {
        $this->assertTrue(function_exists('steam_apps_is_subscribed_app'));
    }

    public function testGetCurrentBetaNameExists(): void
    {
        $this->assertTrue(function_exists('steam_apps_get_current_beta_name'));
    }

    public function testGetEarliestPurchaseTimeExists(): void
    {
        $this->assertTrue(function_exists('steam_apps_get_earliest_purchase_time'));
    }

    public function testGetInstalledDepotsExists(): void
    {
        $this->assertTrue(function_exists('steam_apps_get_installed_depots'));
    }

    public function testGetDlcCountExists(): void
    {
        $this->assertTrue(function_exists('steam_apps_get_dlc_count'));
    }

    public function testGetAppBuildIdExists(): void
    {
        $this->assertTrue(function_exists('steam_apps_get_app_build_id'));
    }

    public function testIsSubscribedAppReturnsBool(): void
    {
        $this->assertIsBool(@steam_apps_is_subscribed_app(480));
    }

    public function testGetCurrentBetaNameReturnsStringOrFalse(): void
    {
        // false when not on a beta branch (mock default), string otherwise.
        $beta = @steam_apps_get_current_beta_name();
        $this->assertTrue(is_string($beta) || $beta === false);
    }

    public function testGetInstalledDepotsReturnsArrayOrFalse(): void
    {
        $depots = @steam_apps_get_installed_depots(480);
        $this->assertTrue(is_array($depots) || $depots === false);
    }

    public function testGetDlcCountReturnsIntOrFalse(): void
    {
        $count = @steam_apps_get_dlc_count();
        $this->assertTrue(is_int($count) || $count === false);
    }
}
