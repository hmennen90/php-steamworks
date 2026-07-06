<?php

use PHPUnit\Framework\TestCase;

class SteamNetTest extends TestCase
{
    public function functionNamesProvider(): array
    {
        return array_map(fn ($n) => [$n], [
            'steam_net_init_relay_network_access',
            'steam_net_create_listen_socket_p2p',
            'steam_net_connect_p2p',
            'steam_net_accept_connection',
            'steam_net_close_connection',
            'steam_net_send_message',
            'steam_net_receive_messages',
            'steam_net_get_connection_events',
        ]);
    }

    /**
     * @dataProvider functionNamesProvider
     */
    public function testFunctionExists(string $name): void
    {
        $this->assertTrue(function_exists($name), "$name should be registered");
    }

    public function testConnectionStateConstants(): void
    {
        $this->assertSame(0, STEAM_NET_CONNECTION_STATE_NONE);
        $this->assertSame(1, STEAM_NET_CONNECTION_STATE_CONNECTING);
        $this->assertSame(3, STEAM_NET_CONNECTION_STATE_CONNECTED);
        $this->assertSame(4, STEAM_NET_CONNECTION_STATE_CLOSED_BY_PEER);
        $this->assertSame(5, STEAM_NET_CONNECTION_STATE_PROBLEM_DETECTED_LOCALLY);
    }

    public function testSendFlagConstants(): void
    {
        $this->assertSame(0, STEAM_NET_SEND_UNRELIABLE);
        $this->assertSame(8, STEAM_NET_SEND_RELIABLE);
    }

    public function testInitRelayReturnsBool(): void
    {
        $this->assertIsBool(@steam_net_init_relay_network_access());
    }

    public function testCreateListenSocketReturnsHandleOrFalse(): void
    {
        $result = @steam_net_create_listen_socket_p2p(0);
        $this->assertTrue(is_int($result) || $result === false);
    }

    public function testConnectP2pReturnsHandleOrFalse(): void
    {
        $result = @steam_net_connect_p2p(76561197960265728, 0);
        $this->assertTrue(is_int($result) || $result === false);
    }

    public function testReceiveMessagesReturnsArrayOrFalse(): void
    {
        $result = @steam_net_receive_messages(1, 16);
        $this->assertTrue(is_array($result) || $result === false);
    }

    public function testGetConnectionEventsReturnsArray(): void
    {
        // Always an array (possibly empty); drains the internal queue.
        $this->assertIsArray(@steam_net_get_connection_events());
    }

    public function testSendMessageReturnsIntOrFalse(): void
    {
        $result = @steam_net_send_message(1, 'ping', true);
        $this->assertTrue(is_int($result) || $result === false);
    }
}
