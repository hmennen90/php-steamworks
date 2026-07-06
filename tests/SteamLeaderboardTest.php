<?php

use PHPUnit\Framework\TestCase;

class SteamLeaderboardTest extends TestCase
{
    public function testFindLeaderboardExists(): void
    {
        $this->assertTrue(function_exists('steam_stats_find_leaderboard'));
    }

    public function testFindOrCreateLeaderboardExists(): void
    {
        $this->assertTrue(function_exists('steam_stats_find_or_create_leaderboard'));
    }

    public function testUploadScoreExists(): void
    {
        $this->assertTrue(function_exists('steam_stats_upload_score'));
    }

    public function testDownloadLeaderboardEntriesExists(): void
    {
        $this->assertTrue(function_exists('steam_stats_download_leaderboard_entries'));
    }

    public function testGetDownloadedEntryExists(): void
    {
        $this->assertTrue(function_exists('steam_stats_get_downloaded_entry'));
    }

    public function testGetLeaderboardEntryCountExists(): void
    {
        $this->assertTrue(function_exists('steam_stats_get_leaderboard_entry_count'));
    }

    public function testGetCallResultExists(): void
    {
        $this->assertTrue(function_exists('steam_get_call_result'));
    }

    public function testLeaderboardConstantsDefined(): void
    {
        $this->assertSame(1, STEAM_LEADERBOARD_SORT_ASCENDING);
        $this->assertSame(2, STEAM_LEADERBOARD_SORT_DESCENDING);
        $this->assertSame(1, STEAM_LEADERBOARD_DISPLAY_NUMERIC);
        $this->assertSame(2, STEAM_LEADERBOARD_DISPLAY_TIME_SECONDS);
        $this->assertSame(3, STEAM_LEADERBOARD_DISPLAY_TIME_MILLISECONDS);
        $this->assertSame(1, STEAM_LEADERBOARD_UPLOAD_KEEP_BEST);
        $this->assertSame(2, STEAM_LEADERBOARD_UPLOAD_FORCE_UPDATE);
        $this->assertSame(0, STEAM_LEADERBOARD_DATA_GLOBAL);
        $this->assertSame(1, STEAM_LEADERBOARD_DATA_GLOBAL_AROUND_USER);
        $this->assertSame(2, STEAM_LEADERBOARD_DATA_FRIENDS);
    }

    public function testFindLeaderboardReturnsHandleOrFalse(): void
    {
        // Mock build returns an int handle; a real build without Steam returns
        // false (accessor null). Either way it must never fatal.
        $handle = @steam_stats_find_leaderboard('HighScores');
        $this->assertTrue(is_int($handle) || $handle === false);
    }

    /**
     * Full async round-trip against the mock SDK: find -> poll -> decoded result.
     * The mock reports every call as immediately completed.
     */
    public function testFindThenPollReturnsLeaderboardFound(): void
    {
        $handle = @steam_stats_find_leaderboard('HighScores');
        if ($handle === false) {
            $this->markTestSkipped('No mock Steam backend available.');
        }

        $result = @steam_get_call_result($handle);
        $this->assertIsArray($result);
        $this->assertSame('leaderboard_found', $result['type']);
        $this->assertArrayHasKey('found', $result);
        $this->assertArrayHasKey('leaderboard', $result);
    }

    public function testUploadThenPollReturnsScoreUploaded(): void
    {
        $handle = @steam_stats_upload_score(42, 1500, STEAM_LEADERBOARD_UPLOAD_KEEP_BEST);
        if ($handle === false) {
            $this->markTestSkipped('No mock Steam backend available.');
        }

        $result = @steam_get_call_result($handle);
        $this->assertIsArray($result);
        $this->assertSame('score_uploaded', $result['type']);
        $this->assertArrayHasKey('success', $result);
        $this->assertArrayHasKey('rank_new', $result);
    }

    public function testDownloadThenPollAndReadEntry(): void
    {
        $handle = @steam_stats_download_leaderboard_entries(42, STEAM_LEADERBOARD_DATA_GLOBAL, 1, 5);
        if ($handle === false) {
            $this->markTestSkipped('No mock Steam backend available.');
        }

        $result = @steam_get_call_result($handle);
        $this->assertIsArray($result);
        $this->assertSame('scores_downloaded', $result['type']);
        $this->assertArrayHasKey('entries', $result);

        $entry = @steam_stats_get_downloaded_entry($result['entries'], 0);
        $this->assertTrue(is_array($entry) || $entry === null);
        if (is_array($entry)) {
            // details is always an array now (int[] of the values, empty if none).
            $this->assertArrayHasKey('details', $entry);
            $this->assertIsArray($entry['details']);
        }
    }

    public function testUploadScoreWithDetailsReturnsHandle(): void
    {
        // The optional int[] details argument must be accepted without a TypeError.
        $handle = @steam_stats_upload_score(42, 1500, STEAM_LEADERBOARD_UPLOAD_KEEP_BEST, [10, 20, 30]);
        $this->assertTrue(is_int($handle) || $handle === false);
    }

    public function testPollingConsumesHandle(): void
    {
        // A handle can only be polled to completion once; a second poll is
        // unknown and returns null (with a warning, suppressed here).
        $handle = @steam_stats_find_leaderboard('HighScores');
        if ($handle === false) {
            $this->markTestSkipped('No mock Steam backend available.');
        }

        @steam_get_call_result($handle);
        $this->assertNull(@steam_get_call_result($handle));
    }
}
