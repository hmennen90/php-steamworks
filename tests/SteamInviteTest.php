<?php

use PHPUnit\Framework\TestCase;

/**
 * Phase 4b — inviting friends into a running game and joining one over rich
 * presence.
 *
 * A player sets the rich presence key "connect"; a friend who clicks "Join" in
 * the Steam friends list, or accepts an invite, makes Steam post
 * GameRichPresenceJoinRequested_t (id 337) with that connect string. A game that
 * was not running is started with it on the command line instead.
 *
 * The callback tests fire the event through the mock SDK's test hook, so they
 * run the extension's real dispatch and queue code; they skip on a build
 * without the hook.
 */
class SteamInviteTest extends TestCase
{
    private const FRIEND_IN_GAME = 76561197960265729; // mock: plays app 480 in a lobby
    private const FRIEND_OFFLINE = 76561197960265730; // mock: not in a game

    protected function setUp(): void
    {
        // Drain whatever an earlier test left behind.
        if (function_exists('steam_friends_get_join_requests')) {
            @steam_friends_get_join_requests();
        }
    }

    private function requireHook(): void
    {
        if (!function_exists('steam_mock_fire_callback')) {
            $this->markTestSkipped('Callback hook only exists in the mock build.');
        }
    }

    public function testFunctionsExist(): void
    {
        foreach ([
            'steam_friends_invite_user_to_game',
            'steam_friends_activate_invite_dialog_connect_string',
            'steam_friends_get_friend_rich_presence',
            'steam_friends_get_friend_game_played',
            'steam_friends_get_join_requests',
            'steam_apps_get_launch_command_line',
        ] as $fn) {
            $this->assertTrue(function_exists($fn), "$fn is not registered");
        }
    }

    public function testInviteSendsTheConnectString(): void
    {
        $sent = @steam_friends_invite_user_to_game(self::FRIEND_OFFLINE, '+connect_lobby 42');
        if ($sent === false && !function_exists('steam_mock_fire_callback')) {
            $this->markTestSkipped('No mock Steam backend available.');
        }
        $this->assertTrue($sent);
    }

    public function testAnEmptyConnectStringIsRefused(): void
    {
        $this->assertFalse(@steam_friends_invite_user_to_game(self::FRIEND_OFFLINE, ''));
    }

    public function testInviteDialogOpensWithoutError(): void
    {
        $opened = @steam_friends_activate_invite_dialog_connect_string('+connect_lobby 42');
        $this->assertIsBool($opened);
    }

    public function testFriendRichPresenceIsReadable(): void
    {
        $connect = @steam_friends_get_friend_rich_presence(self::FRIEND_IN_GAME, 'connect');
        if ($connect === false) {
            $this->markTestSkipped('No mock Steam backend available.');
        }
        $this->assertSame('+connect_lobby 109775240910000001', $connect);
        // A key the friend never set reads as an empty string, not false.
        $this->assertSame('', steam_friends_get_friend_rich_presence(self::FRIEND_IN_GAME, 'nope'));
    }

    public function testFriendInAGameReportsGameAndLobby(): void
    {
        $game = @steam_friends_get_friend_game_played(self::FRIEND_IN_GAME);
        if ($game === false) {
            $this->markTestSkipped('No mock Steam backend available.');
        }
        $this->assertSame(480, $game['app_id']);
        $this->assertSame(109775240910000001, $game['lobby']);
        $this->assertArrayHasKey('game_id', $game);
        $this->assertArrayHasKey('ip', $game);
        $this->assertArrayHasKey('port', $game);
        $this->assertArrayHasKey('query_port', $game);
    }

    public function testFriendNotInAGameReadsAsFalse(): void
    {
        $this->assertFalse(@steam_friends_get_friend_game_played(self::FRIEND_OFFLINE));
    }

    public function testJoinRequestArrivesInTheQueue(): void
    {
        $this->requireHook();

        steam_mock_fire_callback('rich_presence_join_requested', [
            'friend'  => self::FRIEND_IN_GAME,
            'connect' => '+connect_lobby 109775240910000001',
        ]);

        $this->assertSame([
            ['friend' => self::FRIEND_IN_GAME, 'connect' => '+connect_lobby 109775240910000001'],
        ], steam_friends_get_join_requests());
    }

    public function testTheQueueIsDrainedByReading(): void
    {
        $this->requireHook();

        steam_mock_fire_callback('rich_presence_join_requested', ['friend' => 1, 'connect' => 'a']);
        steam_mock_fire_callback('rich_presence_join_requested', ['friend' => 2, 'connect' => 'b']);

        $this->assertCount(2, steam_friends_get_join_requests());
        $this->assertSame([], steam_friends_get_join_requests());
    }

    public function testAConnectStringOfMaximumLengthSurvives(): void
    {
        $this->requireHook();
        // k_cchMaxRichPresenceValueLength = 256 including the terminator.
        $long = str_repeat('x', 255);

        steam_mock_fire_callback('rich_presence_join_requested', ['friend' => 3, 'connect' => $long]);

        $this->assertSame($long, steam_friends_get_join_requests()[0]['connect']);
    }

    public function testLaunchCommandLineCarriesAnInvite(): void
    {
        $line = @steam_apps_get_launch_command_line();
        if ($line === false) {
            $this->markTestSkipped('No mock Steam backend available.');
        }
        $this->assertSame('+connect_lobby 109775240910000001', $line);
    }
}
