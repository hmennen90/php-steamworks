<?php

use PHPUnit\Framework\TestCase;

class SteamStatsTest extends TestCase
{
    public function testSetAchievementExists(): void
    {
        $this->assertTrue(function_exists('steam_stats_set_achievement'));
    }

    public function testClearAchievementExists(): void
    {
        $this->assertTrue(function_exists('steam_stats_clear_achievement'));
    }

    public function testStoreExists(): void
    {
        $this->assertTrue(function_exists('steam_stats_store'));
    }

    public function testGetIntExists(): void
    {
        $this->assertTrue(function_exists('steam_stats_get_int'));
    }

    public function testSetIntExists(): void
    {
        $this->assertTrue(function_exists('steam_stats_set_int'));
    }

    public function testGetFloatExists(): void
    {
        $this->assertTrue(function_exists('steam_stats_get_float'));
    }

    public function testSetFloatExists(): void
    {
        $this->assertTrue(function_exists('steam_stats_set_float'));
    }

    public function testIndicateAchievementProgressExists(): void
    {
        $this->assertTrue(function_exists('steam_stats_indicate_achievement_progress'));
    }

    public function testIndicateAchievementProgressReturnsBoolWithoutSteam(): void
    {
        // Mock/no-Steam environment: returns false, never fatals.
        $this->assertIsBool(@steam_stats_indicate_achievement_progress('ACH_TEST', 47, 100));
    }

    /* ── Achievement read path ── */

    public function testRequestCurrentStatsExists(): void
    {
        $this->assertTrue(function_exists('steam_stats_request_current_stats'));
    }

    public function testGetAchievementExists(): void
    {
        $this->assertTrue(function_exists('steam_stats_get_achievement'));
    }

    public function testGetAchievementUnlockTimeExists(): void
    {
        $this->assertTrue(function_exists('steam_stats_get_achievement_unlock_time'));
    }

    public function testGetNumAchievementsExists(): void
    {
        $this->assertTrue(function_exists('steam_stats_get_num_achievements'));
    }

    public function testGetAchievementNameExists(): void
    {
        $this->assertTrue(function_exists('steam_stats_get_achievement_name'));
    }

    public function testGetAchievementDisplayAttributeExists(): void
    {
        $this->assertTrue(function_exists('steam_stats_get_achievement_display_attribute'));
    }

    public function testResetAllStatsExists(): void
    {
        $this->assertTrue(function_exists('steam_stats_reset_all_stats'));
    }

    public function testRequestCurrentStatsReturnsBool(): void
    {
        // Mock returns true; a real build without Steam returns false. Never fatals.
        $this->assertIsBool(@steam_stats_request_current_stats());
    }

    public function testGetAchievementReturnsBoolOrNull(): void
    {
        // Mock: call succeeds, not achieved -> false. Real without Steam / unknown
        // id -> null. Either way never fatals.
        $result = @steam_stats_get_achievement('ACH_MOCK');
        $this->assertTrue(is_bool($result) || $result === null);
    }

    public function testGetAchievementUnlockTimeReturnsIntOrNull(): void
    {
        // Locked (mock) or unknown -> null; unlocked -> int timestamp.
        $result = @steam_stats_get_achievement_unlock_time('ACH_MOCK');
        $this->assertTrue(is_int($result) || $result === null);
    }

    public function testGetNumAchievementsReturnsIntOrFalse(): void
    {
        $result = @steam_stats_get_num_achievements();
        $this->assertTrue(is_int($result) || $result === false);
    }

    public function testGetAchievementNameReturnsStringOrFalse(): void
    {
        $result = @steam_stats_get_achievement_name(0);
        $this->assertTrue(is_string($result) || $result === false);
    }

    public function testGetAchievementDisplayAttributeReturnsStringOrFalse(): void
    {
        $result = @steam_stats_get_achievement_display_attribute('ACH_MOCK', 'name');
        $this->assertTrue(is_string($result) || $result === false);
    }

    public function testResetAllStatsReturnsBool(): void
    {
        $this->assertIsBool(@steam_stats_reset_all_stats(false));
    }
}
