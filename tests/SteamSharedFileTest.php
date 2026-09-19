<?php

use PHPUnit\Framework\TestCase;

/**
 * Phase 4a — a file shared through Steam Cloud and attached to a leaderboard
 * entry, so other players can download it: share → attach → download entry →
 * download file → read.
 *
 * Against the mock SDK the whole chain runs with deterministic values (shared
 * file = UGC 77, leaderboard = 42). A real build without Steam returns false at
 * the first step, and those tests skip.
 */
class SteamSharedFileTest extends TestCase
{
    public function testFunctionsExist(): void
    {
        foreach ([
            'steam_remote_file_share',
            'steam_remote_ugc_download',
            'steam_remote_ugc_read',
            'steam_remote_get_ugc_details',
            'steam_stats_attach_leaderboard_ugc',
        ] as $fn) {
            $this->assertTrue(function_exists($fn), "$fn is not registered");
        }
    }

    public function testInvalidHandleConstant(): void
    {
        $this->assertSame(-1, STEAM_UGC_HANDLE_INVALID);
    }

    public function testShareAttachDownloadReadRoundTrip(): void
    {
        $share = @steam_remote_file_share('company.json');
        if ($share === false) {
            $this->markTestSkipped('No mock Steam backend available.');
        }
        $shared = @steam_get_call_result($share);
        $this->assertIsArray($shared);
        $this->assertSame('remote_file_shared', $shared['type']);
        $this->assertTrue($shared['success']);
        $this->assertSame('company.json', $shared['name']);
        $ugc = $shared['ugc'];

        $attach = @steam_stats_attach_leaderboard_ugc(42, $ugc);
        $this->assertIsInt($attach);
        $attached = @steam_get_call_result($attach);
        $this->assertSame('leaderboard_ugc_set', $attached['type']);
        $this->assertTrue($attached['success']);
        $this->assertSame(42, $attached['leaderboard']);

        $download = @steam_remote_ugc_download($ugc);
        $this->assertIsInt($download);
        $downloaded = @steam_get_call_result($download);
        $this->assertSame('remote_ugc_downloaded', $downloaded['type']);
        $this->assertTrue($downloaded['success']);
        $this->assertSame($ugc, $downloaded['ugc']);
        $this->assertGreaterThan(0, $downloaded['size']);

        $bytes = @steam_remote_ugc_read($ugc, $downloaded['size']);
        $this->assertSame('{"company":"Mock GmbH","value":1234}', $bytes);
        $this->assertSame($downloaded['size'], strlen($bytes));
    }

    public function testReadInChunksWithOffset(): void
    {
        if (@steam_remote_ugc_download(77) === false) {
            $this->markTestSkipped('No mock Steam backend available.');
        }

        $first = @steam_remote_ugc_read(77, 10);
        $rest = @steam_remote_ugc_read(77, 1000, 10);

        $this->assertSame('{"company"', $first);
        $this->assertSame('{"company":"Mock GmbH","value":1234}', $first . $rest);
    }

    public function testUgcDetailsAfterDownload(): void
    {
        $details = @steam_remote_get_ugc_details(77);
        if ($details === false) {
            $this->markTestSkipped('No mock Steam backend available.');
        }

        $this->assertSame('company.json', $details['name']);
        $this->assertSame(480, $details['app_id']);
        $this->assertGreaterThan(0, $details['size']);
        $this->assertIsInt($details['owner']);
    }

    public function testDownloadedEntryCarriesItsAttachment(): void
    {
        $call = @steam_stats_download_leaderboard_entries(42, STEAM_LEADERBOARD_DATA_FRIENDS, 1, 2);
        if ($call === false) {
            $this->markTestSkipped('No mock Steam backend available.');
        }
        $entries = @steam_get_call_result($call)['entries'];

        $withFile = @steam_stats_get_downloaded_entry($entries, 0);
        $withoutFile = @steam_stats_get_downloaded_entry($entries, 1);

        $this->assertSame(77, $withFile['ugc']);
        $this->assertSame(STEAM_UGC_HANDLE_INVALID, $withoutFile['ugc']);
    }

    public function testUnknownFileReadsAsFalse(): void
    {
        $this->assertFalse(@steam_remote_ugc_read(12345, 16));
        $this->assertFalse(@steam_remote_get_ugc_details(12345));
    }

    public function testOutOfRangeArgumentsWarnAndReturnFalse(): void
    {
        $this->assertFalse(@steam_remote_ugc_read(77, 0));
        $this->assertFalse(@steam_remote_ugc_read(77, 16, -1));
        $this->assertFalse(@steam_remote_ugc_download(77, -1));
    }

    public function testWrongTypesThrow(): void
    {
        $this->expectException(TypeError::class);
        steam_remote_file_share([]);
    }
}
