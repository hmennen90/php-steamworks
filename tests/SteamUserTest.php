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

    /* ── Auth tickets ── */

    public function authFunctionNamesProvider(): array
    {
        return array_map(fn ($n) => [$n], [
            'steam_user_get_auth_session_ticket',
            'steam_user_begin_auth_session',
            'steam_user_end_auth_session',
            'steam_user_cancel_auth_ticket',
        ]);
    }

    /**
     * @dataProvider authFunctionNamesProvider
     */
    public function testAuthFunctionExists(string $name): void
    {
        $this->assertTrue(function_exists($name), "$name should be registered");
    }

    public function testBeginAuthSessionConstants(): void
    {
        $this->assertSame(0, STEAM_BEGIN_AUTH_SESSION_OK);
        $this->assertSame(1, STEAM_BEGIN_AUTH_SESSION_INVALID_TICKET);
        $this->assertSame(5, STEAM_BEGIN_AUTH_SESSION_EXPIRED_TICKET);
    }

    public function testGetAuthSessionTicketReturnsArrayOrFalse(): void
    {
        // Mock returns a fake ticket; real without Steam -> false.
        $ticket = @steam_user_get_auth_session_ticket();
        $this->assertTrue($ticket === false || is_array($ticket));
        if (is_array($ticket)) {
            $this->assertArrayHasKey('handle', $ticket);
            $this->assertArrayHasKey('ticket', $ticket);
            $this->assertIsInt($ticket['handle']);
            $this->assertIsString($ticket['ticket']);
            $this->assertGreaterThan(0, strlen($ticket['ticket']));
        }
    }

    public function testBeginAuthSessionReturnsIntOrFalse(): void
    {
        $result = @steam_user_begin_auth_session("\x01\x02\x03\x04", 76561197960265728);
        $this->assertTrue(is_int($result) || $result === false);
    }

    public function testEndAndCancelReturnBool(): void
    {
        $this->assertIsBool(@steam_user_end_auth_session(76561197960265728));
        $this->assertIsBool(@steam_user_cancel_auth_ticket(1));
    }
}
