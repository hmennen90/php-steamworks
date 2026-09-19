<?php

use PHPUnit\Framework\TestCase;

/**
 * Phase 4c — lobbies (ISteamMatchmaking, "SteamMatchMaking009").
 *
 * A lobby is the room before a match: its members, their per-member data
 * ("ready"), the lobby's own data (era, difficulty) and a chat. Creating and
 * joining are CallResults; everything that happens to the lobby afterwards
 * arrives as callbacks, drained with steam_matchmaking_get_events().
 *
 * Mock world: the local user 76561197960265728 owns lobby 109775240910000001,
 * friend 76561197960265729 is the second member. Callback tests fire events
 * through the mock SDK's test hook and skip on a build without it.
 */
class SteamMatchmakingTest extends TestCase
{
    private const LOBBY  = 109775240910000001;
    private const ME     = 76561197960265728;
    private const FRIEND = 76561197960265729;

    protected function setUp(): void
    {
        if (function_exists('steam_matchmaking_get_events')) {
            @steam_matchmaking_get_events();
        }
    }

    private function requireMock(mixed $result): void
    {
        if ($result === false && !function_exists('steam_mock_fire_callback')) {
            $this->markTestSkipped('No mock Steam backend available.');
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
            'steam_matchmaking_create_lobby', 'steam_matchmaking_join_lobby',
            'steam_matchmaking_leave_lobby', 'steam_matchmaking_invite_user_to_lobby',
            'steam_matchmaking_get_num_lobby_members', 'steam_matchmaking_get_lobby_member_by_index',
            'steam_matchmaking_get_lobby_owner', 'steam_matchmaking_set_lobby_owner',
            'steam_matchmaking_get_lobby_data', 'steam_matchmaking_set_lobby_data',
            'steam_matchmaking_get_lobby_member_data', 'steam_matchmaking_set_lobby_member_data',
            'steam_matchmaking_set_lobby_joinable', 'steam_matchmaking_set_lobby_type',
            'steam_matchmaking_send_lobby_chat_msg', 'steam_matchmaking_get_events',
            'steam_friends_activate_overlay_invite_dialog',
        ] as $fn) {
            $this->assertTrue(function_exists($fn), "$fn is not registered");
        }
    }

    public function testConstants(): void
    {
        $this->assertSame(0, STEAM_LOBBY_TYPE_PRIVATE);
        $this->assertSame(1, STEAM_LOBBY_TYPE_FRIENDS_ONLY);
        $this->assertSame(2, STEAM_LOBBY_TYPE_PUBLIC);
        $this->assertSame(3, STEAM_LOBBY_TYPE_INVISIBLE);
        $this->assertSame(4, STEAM_LOBBY_TYPE_PRIVATE_UNIQUE);
        $this->assertSame(1, STEAM_CHAT_MEMBER_STATE_ENTERED);
        $this->assertSame(2, STEAM_CHAT_MEMBER_STATE_LEFT);
        $this->assertSame(4, STEAM_CHAT_MEMBER_STATE_DISCONNECTED);
        $this->assertSame(8, STEAM_CHAT_MEMBER_STATE_KICKED);
        $this->assertSame(16, STEAM_CHAT_MEMBER_STATE_BANNED);
        $this->assertSame(1, STEAM_CHAT_ROOM_ENTER_SUCCESS);
        $this->assertSame(1, STEAM_CHAT_ENTRY_TYPE_CHAT_MSG);
    }

    public function testCreateLobbyReportsTheNewLobby(): void
    {
        $call = @steam_matchmaking_create_lobby(STEAM_LOBBY_TYPE_FRIENDS_ONLY, 4);
        $this->requireMock($call);

        $result = steam_get_call_result($call);

        $this->assertSame('lobby_created', $result['type']);
        $this->assertTrue($result['success']);
        $this->assertSame(1, $result['result']);
        $this->assertSame(self::LOBBY, $result['lobby']);
    }

    public function testALobbyNeedsRoomForSomeone(): void
    {
        $this->assertFalse(@steam_matchmaking_create_lobby(STEAM_LOBBY_TYPE_PUBLIC, 0));
    }

    public function testJoinLobbyReportsEntry(): void
    {
        $call = @steam_matchmaking_join_lobby(self::LOBBY);
        $this->requireMock($call);

        $result = steam_get_call_result($call);

        $this->assertSame('lobby_entered', $result['type']);
        $this->assertSame(self::LOBBY, $result['lobby']);
        $this->assertTrue($result['success']);
        $this->assertSame(STEAM_CHAT_ROOM_ENTER_SUCCESS, $result['response']);
        $this->assertFalse($result['locked']);
        $this->assertArrayHasKey('permissions', $result);
    }

    public function testMembersAndOwner(): void
    {
        $count = @steam_matchmaking_get_num_lobby_members(self::LOBBY);
        $this->requireMock($count);

        $this->assertSame(2, $count);
        $this->assertSame(self::ME, steam_matchmaking_get_lobby_member_by_index(self::LOBBY, 0));
        $this->assertSame(self::FRIEND, steam_matchmaking_get_lobby_member_by_index(self::LOBBY, 1));
        $this->assertFalse(@steam_matchmaking_get_lobby_member_by_index(self::LOBBY, 2));
        $this->assertSame(self::ME, steam_matchmaking_get_lobby_owner(self::LOBBY));
        $this->assertTrue(steam_matchmaking_set_lobby_owner(self::LOBBY, self::FRIEND));
    }

    public function testLobbyDataRoundTrip(): void
    {
        $set = @steam_matchmaking_set_lobby_data(self::LOBBY, 'era', '1990');
        $this->requireMock($set);

        $this->assertTrue($set);
        $this->assertSame('1990', steam_matchmaking_get_lobby_data(self::LOBBY, 'era'));
        $this->assertSame('', steam_matchmaking_get_lobby_data(self::LOBBY, 'never_set'));
    }

    public function testMemberDataRoundTrip(): void
    {
        $set = @steam_matchmaking_set_lobby_member_data(self::LOBBY, 'ready', '1');
        $this->requireMock($set);

        $this->assertTrue($set);
        $this->assertSame('1', steam_matchmaking_get_lobby_member_data(self::LOBBY, self::ME, 'ready'));
        $this->assertSame('', steam_matchmaking_get_lobby_member_data(self::LOBBY, self::FRIEND, 'ready'));
    }

    public function testOverlongKeysAreRefused(): void
    {
        // k_nMaxLobbyKeyLength = 255 characters.
        $key = str_repeat('k', 256);
        $this->assertFalse(@steam_matchmaking_set_lobby_data(self::LOBBY, $key, 'v'));
        $this->assertFalse(@steam_matchmaking_set_lobby_member_data(self::LOBBY, $key, 'v'));
    }

    public function testOverlongValuesAreRefused(): void
    {
        // k_cubChatMetadataMax = 8192 bytes including the terminator.
        $value = str_repeat('v', 8192);
        $this->assertFalse(@steam_matchmaking_set_lobby_data(self::LOBBY, 'k', $value));
    }

    public function testLobbySettingsAndInvites(): void
    {
        $joinable = @steam_matchmaking_set_lobby_joinable(self::LOBBY, false);
        $this->requireMock($joinable);

        $this->assertTrue($joinable);
        $this->assertTrue(steam_matchmaking_set_lobby_type(self::LOBBY, STEAM_LOBBY_TYPE_PRIVATE));
        $this->assertTrue(steam_matchmaking_invite_user_to_lobby(self::LOBBY, self::FRIEND));
        $this->assertTrue(steam_friends_activate_overlay_invite_dialog(self::LOBBY));
        $this->assertTrue(steam_matchmaking_leave_lobby(self::LOBBY));
    }

    public function testChatMessagesHaveALimit(): void
    {
        $sent = @steam_matchmaking_send_lobby_chat_msg(self::LOBBY, 'ready?');
        $this->requireMock($sent);

        $this->assertTrue($sent);
        $this->assertFalse(@steam_matchmaking_send_lobby_chat_msg(self::LOBBY, ''));
        // Up to 4 KB per message.
        $this->assertTrue(steam_matchmaking_send_lobby_chat_msg(self::LOBBY, str_repeat('x', 4096)));
        $this->assertFalse(@steam_matchmaking_send_lobby_chat_msg(self::LOBBY, str_repeat('x', 4097)));
    }

    public function testAcceptedLobbyInviteArrivesAsEvent(): void
    {
        $this->requireHook();

        steam_mock_fire_callback('lobby_join_requested', ['lobby' => self::LOBBY, 'friend' => self::FRIEND]);

        $this->assertSame([
            ['type' => 'join_requested', 'lobby' => self::LOBBY, 'friend' => self::FRIEND],
        ], steam_matchmaking_get_events());
    }

    public function testMembersComingAndGoingArriveAsEvents(): void
    {
        $this->requireHook();

        steam_mock_fire_callback('lobby_chat_update', [
            'lobby' => self::LOBBY, 'user' => self::FRIEND, 'changed_by' => self::FRIEND,
            'state' => STEAM_CHAT_MEMBER_STATE_ENTERED,
        ]);
        steam_mock_fire_callback('lobby_chat_update', [
            'lobby' => self::LOBBY, 'user' => self::FRIEND, 'changed_by' => self::ME,
            'state' => STEAM_CHAT_MEMBER_STATE_LEFT | STEAM_CHAT_MEMBER_STATE_KICKED,
        ]);

        $this->assertSame([
            ['type' => 'chat_update', 'lobby' => self::LOBBY, 'user' => self::FRIEND,
             'changed_by' => self::FRIEND, 'state' => STEAM_CHAT_MEMBER_STATE_ENTERED],
            ['type' => 'chat_update', 'lobby' => self::LOBBY, 'user' => self::FRIEND,
             'changed_by' => self::ME, 'state' => STEAM_CHAT_MEMBER_STATE_LEFT | STEAM_CHAT_MEMBER_STATE_KICKED],
        ], steam_matchmaking_get_events());
    }

    public function testDataChangesArriveAsEvents(): void
    {
        $this->requireHook();

        steam_mock_fire_callback('lobby_data_update', ['lobby' => self::LOBBY, 'member' => self::LOBBY, 'success' => true]);
        steam_mock_fire_callback('lobby_data_update', ['lobby' => self::LOBBY, 'member' => self::FRIEND, 'success' => false]);

        $this->assertSame([
            ['type' => 'data_update', 'lobby' => self::LOBBY, 'member' => self::LOBBY, 'success' => true],
            ['type' => 'data_update', 'lobby' => self::LOBBY, 'member' => self::FRIEND, 'success' => false],
        ], steam_matchmaking_get_events());
    }

    public function testChatMessagesArriveWithTheirText(): void
    {
        $this->requireHook();

        steam_mock_fire_callback('lobby_chat_message', ['lobby' => self::LOBBY, 'user' => self::FRIEND, 'message' => 'bereit']);
        // Binary-safe: a NUL inside the message is kept.
        steam_mock_fire_callback('lobby_chat_message', ['lobby' => self::LOBBY, 'user' => self::ME, 'message' => "a\0b"]);

        $this->assertSame([
            ['type' => 'chat_message', 'lobby' => self::LOBBY, 'user' => self::FRIEND,
             'entry_type' => STEAM_CHAT_ENTRY_TYPE_CHAT_MSG, 'message' => 'bereit'],
            ['type' => 'chat_message', 'lobby' => self::LOBBY, 'user' => self::ME,
             'entry_type' => STEAM_CHAT_ENTRY_TYPE_CHAT_MSG, 'message' => "a\0b"],
        ], steam_matchmaking_get_events());
    }

    public function testEventsKeepTheirOrderAcrossKinds(): void
    {
        $this->requireHook();

        steam_mock_fire_callback('lobby_chat_update', ['lobby' => self::LOBBY, 'user' => self::FRIEND, 'changed_by' => self::FRIEND, 'state' => 1]);
        steam_mock_fire_callback('lobby_chat_message', ['lobby' => self::LOBBY, 'user' => self::FRIEND, 'message' => 'hi']);
        steam_mock_fire_callback('lobby_data_update', ['lobby' => self::LOBBY, 'member' => self::FRIEND, 'success' => true]);

        $this->assertSame(['chat_update', 'chat_message', 'data_update'],
            array_column(steam_matchmaking_get_events(), 'type'));
        $this->assertSame([], steam_matchmaking_get_events());
    }
}
