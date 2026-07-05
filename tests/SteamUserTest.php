<?php

use PHPUnit\Framework\TestCase;

class SteamUserTest extends TestCase
{
    public function testGetSteamIdExists(): void
    {
        $this->assertTrue(function_exists('steam_user_get_steam_id'));
    }

    public function testIsLoggedOnExists(): void
    {
        $this->assertTrue(function_exists('steam_user_is_logged_on'));
    }

    public function testGetPlayerSteamLevelExists(): void
    {
        $this->assertTrue(function_exists('steam_user_get_player_steam_level'));
    }

    public function testGetSteamIdReturnsIntOrFalse(): void
    {
        $id = @steam_user_get_steam_id();
        $this->assertTrue(is_int($id) || $id === false);
    }

    public function testIsLoggedOnReturnsBool(): void
    {
        // Mock/no-Steam environment: returns false, never fatals.
        $this->assertIsBool(@steam_user_is_logged_on());
    }

    public function testGetPlayerSteamLevelReturnsIntOrFalse(): void
    {
        $level = @steam_user_get_player_steam_level();
        $this->assertTrue(is_int($level) || $level === false);
    }
}
