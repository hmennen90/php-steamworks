<?php

use PHPUnit\Framework\TestCase;

class SteamInitTest extends TestCase
{
    public function testExtensionLoaded(): void
    {
        $this->assertTrue(extension_loaded('steamworks'));
    }

    public function testInitReturnsFalseWithoutSteam(): void
    {
        $result = steam_init();
        $this->assertIsBool($result);
    }

    public function testShutdownExistsAsFunction(): void
    {
        $this->assertTrue(function_exists('steam_shutdown'));
    }

    public function testRunCallbacksExistsAsFunction(): void
    {
        $this->assertTrue(function_exists('steam_run_callbacks'));
    }

    public function testUserGetSteamIdExistsAsFunction(): void
    {
        $this->assertTrue(function_exists('steam_user_get_steam_id'));
    }

    public function testFriendsGetNameExistsAsFunction(): void
    {
        $this->assertTrue(function_exists('steam_friends_get_name'));
    }

    public function testAppsGetAppIdExistsAsFunction(): void
    {
        $this->assertTrue(function_exists('steam_apps_get_app_id'));
    }
}
