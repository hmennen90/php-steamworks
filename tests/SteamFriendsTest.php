<?php

use PHPUnit\Framework\TestCase;

class SteamFriendsTest extends TestCase
{
    public function testGetNameExists(): void
    {
        $this->assertTrue(function_exists('steam_friends_get_name'));
    }

    public function testSetRichPresenceExists(): void
    {
        $this->assertTrue(function_exists('steam_friends_set_rich_presence'));
    }

    public function testActivateOverlayExists(): void
    {
        $this->assertTrue(function_exists('steam_friends_activate_overlay'));
    }

    public function testActivateOverlayToWebPageExists(): void
    {
        $this->assertTrue(function_exists('steam_friends_activate_overlay_to_web_page'));
    }

    public function testGetNameReturnsStringOrFalse(): void
    {
        // Mock build returns a name string; a real build without Steam returns
        // false. Either way it must never fatal.
        $name = @steam_friends_get_name();
        $this->assertTrue(is_string($name) || $name === false);
    }

    public function testSetRichPresenceReturnsBoolWithoutSteam(): void
    {
        // Mock/no-Steam environment: returns false, never fatals.
        $this->assertIsBool(@steam_friends_set_rich_presence('status', 'Testing'));
    }
}
