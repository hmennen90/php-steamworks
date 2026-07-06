<?php

use PHPUnit\Framework\TestCase;

class SteamUgcTest extends TestCase
{
    public function functionNamesProvider(): array
    {
        return array_map(fn ($n) => [$n], [
            'steam_ugc_subscribe_item',
            'steam_ugc_unsubscribe_item',
            'steam_ugc_get_num_subscribed_items',
            'steam_ugc_get_subscribed_items',
            'steam_ugc_get_item_state',
            'steam_ugc_get_item_install_info',
            'steam_ugc_get_item_download_info',
            'steam_ugc_download_item',
        ]);
    }

    /**
     * @dataProvider functionNamesProvider
     */
    public function testFunctionExists(string $name): void
    {
        $this->assertTrue(function_exists($name), "$name should be registered");
    }

    public function testItemStateConstants(): void
    {
        $this->assertSame(0, STEAM_UGC_ITEM_STATE_NONE);
        $this->assertSame(1, STEAM_UGC_ITEM_STATE_SUBSCRIBED);
        $this->assertSame(4, STEAM_UGC_ITEM_STATE_INSTALLED);
        $this->assertSame(8, STEAM_UGC_ITEM_STATE_NEEDS_UPDATE);
        $this->assertSame(16, STEAM_UGC_ITEM_STATE_DOWNLOADING);
    }

    public function testSubscribeReturnsHandleOrFalse(): void
    {
        $result = @steam_ugc_subscribe_item(123456);
        $this->assertTrue(is_int($result) || $result === false);
    }

    public function testGetNumSubscribedItemsReturnsIntOrFalse(): void
    {
        $result = @steam_ugc_get_num_subscribed_items();
        $this->assertTrue(is_int($result) || $result === false);
    }

    public function testGetSubscribedItemsReturnsArrayOrFalse(): void
    {
        $result = @steam_ugc_get_subscribed_items();
        $this->assertTrue(is_array($result) || $result === false);
        if (is_array($result)) {
            foreach ($result as $id) {
                $this->assertIsInt($id);
            }
        }
    }

    public function testGetItemStateReturnsIntOrFalse(): void
    {
        $state = @steam_ugc_get_item_state(123456);
        $this->assertTrue(is_int($state) || $state === false);
    }

    public function testGetItemInstallInfoReturnsArrayOrFalse(): void
    {
        $info = @steam_ugc_get_item_install_info(123456);
        $this->assertTrue(is_array($info) || $info === false);
        if (is_array($info)) {
            $this->assertArrayHasKey('size_on_disk', $info);
            $this->assertArrayHasKey('folder', $info);
            $this->assertArrayHasKey('timestamp', $info);
            $this->assertIsString($info['folder']);
        }
    }

    public function testDownloadItemReturnsBool(): void
    {
        $this->assertIsBool(@steam_ugc_download_item(123456, false));
    }
}
