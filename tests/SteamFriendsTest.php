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

    /* ── Friends list, persona state & avatars ── */

    public function friendsFunctionNamesProvider(): array
    {
        return array_map(fn ($n) => [$n], [
            'steam_friends_get_persona_state',
            'steam_friends_get_friend_count',
            'steam_friends_get_friend_by_index',
            'steam_friends_get_friend_relationship',
            'steam_friends_get_friend_persona_state',
            'steam_friends_get_friend_persona_name',
            'steam_friends_request_user_information',
            'steam_friends_get_friend_avatar',
        ]);
    }

    /**
     * @dataProvider friendsFunctionNamesProvider
     */
    public function testFriendsFunctionExists(string $name): void
    {
        $this->assertTrue(function_exists($name), "$name should be registered");
    }

    public function testPersonaAndRelationshipConstants(): void
    {
        $this->assertSame(0, STEAM_PERSONA_STATE_OFFLINE);
        $this->assertSame(1, STEAM_PERSONA_STATE_ONLINE);
        $this->assertSame(3, STEAM_FRIEND_RELATIONSHIP_FRIEND);
        $this->assertSame(0x04, STEAM_FRIEND_FLAG_IMMEDIATE);
        $this->assertSame(0xFFFF, STEAM_FRIEND_FLAG_ALL);
    }

    public function testAvatarSizeConstants(): void
    {
        $this->assertSame(0, STEAM_AVATAR_SMALL);
        $this->assertSame(1, STEAM_AVATAR_MEDIUM);
        $this->assertSame(2, STEAM_AVATAR_LARGE);
    }

    public function testGetFriendCountReturnsIntOrFalse(): void
    {
        $result = @steam_friends_get_friend_count();
        $this->assertTrue(is_int($result) || $result === false);
    }

    public function testGetFriendByIndexReturnsIntOrFalse(): void
    {
        $result = @steam_friends_get_friend_by_index(0);
        $this->assertTrue(is_int($result) || $result === false);
    }

    public function testGetFriendPersonaNameReturnsStringOrFalse(): void
    {
        $result = @steam_friends_get_friend_persona_name(76561197960265728);
        $this->assertTrue(is_string($result) || $result === false);
    }

    public function testRequestUserInformationReturnsBool(): void
    {
        $this->assertIsBool(@steam_friends_request_user_information(76561197960265728, false));
    }

    public function testGetFriendAvatarReturnsArrayOrNull(): void
    {
        // Mock returns a 64x64 opaque avatar; real without Steam / no avatar -> null.
        $avatar = @steam_friends_get_friend_avatar(76561197960265728, STEAM_AVATAR_MEDIUM);
        $this->assertTrue($avatar === null || is_array($avatar));
        if (is_array($avatar)) {
            $this->assertArrayHasKey('width', $avatar);
            $this->assertArrayHasKey('height', $avatar);
            $this->assertArrayHasKey('rgba', $avatar);
            $this->assertIsInt($avatar['width']);
            $this->assertIsString($avatar['rgba']);
            // RGBA8888 => exactly width*height*4 bytes.
            $this->assertSame($avatar['width'] * $avatar['height'] * 4, strlen($avatar['rgba']));
        }
    }
}
