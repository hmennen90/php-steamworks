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
}
