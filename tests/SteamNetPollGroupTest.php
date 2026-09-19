<?php

use PHPUnit\Framework\TestCase;

/**
 * Phase 4d — networking polish for a host with several clients.
 *
 * - Poll groups: the host reads the messages of all connections with one call.
 * - Connection status: ping and quality for the player list.
 * - Accept flow: connection events say which listen socket an incoming
 *   connection arrived on (0 for outgoing ones), and a listen socket can be
 *   closed again.
 *
 * The connection-status callback is delivered by the mock in the layout of the
 * platform the tests run on — pack(8) on Windows, pack(4) on Linux and macOS,
 * where the connection info starts 4 bytes earlier. Reading it with the wrong
 * offsets turns peer and states into garbage.
 */
class SteamNetPollGroupTest extends TestCase
{
    private const PEER = 76561197960265729;

    protected function setUp(): void
    {
        @steam_net_get_connection_events();
    }

    private function requireMock(mixed $result): void
    {
        if ($result === false && !function_exists('steam_mock_fire_callback')) {
            $this->markTestSkipped('No mock Steam backend available.');
        }
    }

    public function testFunctionsExist(): void
    {
        foreach ([
            'steam_net_create_poll_group', 'steam_net_destroy_poll_group',
            'steam_net_set_connection_poll_group', 'steam_net_receive_messages_on_poll_group',
            'steam_net_get_connection_status', 'steam_net_close_listen_socket',
        ] as $fn) {
            $this->assertTrue(function_exists($fn), "$fn is not registered");
        }
    }

    public function testMessagesOfAllConnectionsArriveThroughTheGroup(): void
    {
        $group = @steam_net_create_poll_group();
        $this->requireMock($group);

        $this->assertIsInt($group);
        $this->assertTrue(steam_net_set_connection_poll_group(31, $group));

        $messages = steam_net_receive_messages_on_poll_group($group);

        $this->assertCount(1, $messages);
        $this->assertSame('hello', $messages[0]['data']);
        $this->assertSame(5, $messages[0]['size']);
        $this->assertSame(31, $messages[0]['connection']);
        $this->assertSame(self::PEER, $messages[0]['peer']);
        $this->assertSame(7, $messages[0]['message_number']);
        $this->assertTrue($messages[0]['reliable']);

        $this->assertTrue(steam_net_destroy_poll_group($group));
    }

    public function testConnectionStatusReportsPingAndQuality(): void
    {
        $status = @steam_net_get_connection_status(31);
        $this->requireMock($status);

        $this->assertSame(STEAM_NET_CONNECTION_STATE_CONNECTED, $status['state']);
        $this->assertSame(42, $status['ping']);
        $this->assertEqualsWithDelta(0.95, $status['quality_local'], 0.0001);
        $this->assertEqualsWithDelta(0.9, $status['quality_remote'], 0.0001);
        $this->assertEqualsWithDelta(1200.0, $status['out_bytes_per_sec'], 0.0001);
        $this->assertEqualsWithDelta(800.0, $status['in_bytes_per_sec'], 0.0001);
        $this->assertSame(256000, $status['send_rate']);
        $this->assertSame(0, $status['pending_unreliable']);
        $this->assertSame(64, $status['pending_reliable']);
        $this->assertSame(1500, $status['queue_time_usec']);
    }

    public function testUnknownConnectionHasNoStatus(): void
    {
        $this->assertFalse(@steam_net_get_connection_status(999));
    }

    public function testListenSocketCanBeClosed(): void
    {
        $socket = @steam_net_create_listen_socket_p2p(0);
        $this->requireMock($socket);

        $this->assertTrue(steam_net_close_listen_socket($socket));
    }

    public function testIncomingConnectionEventNamesItsListenSocket(): void
    {
        if (!function_exists('steam_mock_fire_callback')) {
            $this->markTestSkipped('Callback hook only exists in the mock build.');
        }

        steam_mock_fire_callback('net_connection_status', [
            'connection' => 33, 'peer' => self::PEER, 'listen_socket' => 30,
            'state' => STEAM_NET_CONNECTION_STATE_CONNECTING, 'old_state' => STEAM_NET_CONNECTION_STATE_NONE,
        ]);
        steam_mock_fire_callback('net_connection_status', [
            'connection' => 31, 'peer' => self::PEER, 'listen_socket' => 0,
            'state' => STEAM_NET_CONNECTION_STATE_CONNECTED, 'old_state' => STEAM_NET_CONNECTION_STATE_CONNECTING,
        ]);

        $this->assertSame([
            ['connection' => 33, 'state' => STEAM_NET_CONNECTION_STATE_CONNECTING,
             'old_state' => STEAM_NET_CONNECTION_STATE_NONE, 'peer' => self::PEER, 'listen_socket' => 30],
            ['connection' => 31, 'state' => STEAM_NET_CONNECTION_STATE_CONNECTED,
             'old_state' => STEAM_NET_CONNECTION_STATE_CONNECTING, 'peer' => self::PEER, 'listen_socket' => 0],
        ], steam_net_get_connection_events());
    }
}
