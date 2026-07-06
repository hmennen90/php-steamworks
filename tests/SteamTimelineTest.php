<?php

use PHPUnit\Framework\TestCase;

class SteamTimelineTest extends TestCase
{
    /* ── Function existence ── */

    public function functionNamesProvider(): array
    {
        return array_map(fn ($n) => [$n], [
            'steam_timeline_set_game_mode',
            'steam_timeline_set_tooltip',
            'steam_timeline_clear_tooltip',
            'steam_timeline_add_instantaneous_event',
            'steam_timeline_add_range_event',
            'steam_timeline_start_range_event',
            'steam_timeline_update_range_event',
            'steam_timeline_end_range_event',
            'steam_timeline_remove_event',
            'steam_timeline_does_event_recording_exist',
            'steam_timeline_start_game_phase',
            'steam_timeline_end_game_phase',
            'steam_timeline_set_game_phase_id',
            'steam_timeline_does_game_phase_recording_exist',
            'steam_timeline_add_game_phase_tag',
            'steam_timeline_set_game_phase_attribute',
            'steam_timeline_open_overlay_to_game_phase',
            'steam_timeline_open_overlay_to_event',
        ]);
    }

    /**
     * @dataProvider functionNamesProvider
     */
    public function testFunctionExists(string $name): void
    {
        $this->assertTrue(function_exists($name), "$name should be registered");
    }

    /* ── Constants ── */

    public function testGameModeConstantsDefined(): void
    {
        $this->assertSame(1, STEAM_TIMELINE_GAME_MODE_PLAYING);
        $this->assertSame(2, STEAM_TIMELINE_GAME_MODE_STAGING);
        $this->assertSame(3, STEAM_TIMELINE_GAME_MODE_MENUS);
        $this->assertSame(4, STEAM_TIMELINE_GAME_MODE_LOADING_SCREEN);
    }

    public function testClipPriorityConstantsDefined(): void
    {
        $this->assertSame(1, STEAM_TIMELINE_CLIP_PRIORITY_NONE);
        $this->assertSame(2, STEAM_TIMELINE_CLIP_PRIORITY_STANDARD);
        $this->assertSame(3, STEAM_TIMELINE_CLIP_PRIORITY_FEATURED);
    }

    /* ── Fire-and-forget calls: bool, never fatal ── */

    public function testSetGameModeReturnsBool(): void
    {
        $this->assertIsBool(@steam_timeline_set_game_mode(STEAM_TIMELINE_GAME_MODE_PLAYING));
    }

    public function testSetTooltipReturnsBool(): void
    {
        $this->assertIsBool(@steam_timeline_set_tooltip('Level 3', 0.0));
    }

    public function testClearTooltipDefaultArgReturnsBool(): void
    {
        $this->assertIsBool(@steam_timeline_clear_tooltip());
    }

    public function testStartAndEndGamePhaseReturnBool(): void
    {
        $this->assertIsBool(@steam_timeline_start_game_phase());
        $this->assertIsBool(@steam_timeline_end_game_phase());
    }

    public function testAddGamePhaseTagOptionalPriority(): void
    {
        $this->assertIsBool(@steam_timeline_add_game_phase_tag('Boss', 'icon', 'Encounters'));
    }

    /* ── Event handle returning calls: int ── */

    public function testAddInstantaneousEventReturnsInt(): void
    {
        $handle = @steam_timeline_add_instantaneous_event('Kill', 'Boss down', 'steam_death');
        $this->assertIsInt($handle);
    }

    public function testAddRangeEventReturnsInt(): void
    {
        $handle = @steam_timeline_add_range_event('Fight', 'Boss fight', 'steam_combat', 0, 0.0, 30.0);
        $this->assertIsInt($handle);
    }

    /* ── Async recording-exists queries: handle or false ── */

    public function testDoesEventRecordingExistReturnsHandleOrFalse(): void
    {
        $result = @steam_timeline_does_event_recording_exist(7);
        $this->assertTrue(is_int($result) || $result === false);
    }

    public function testDoesGamePhaseRecordingExistReturnsHandleOrFalse(): void
    {
        $result = @steam_timeline_does_game_phase_recording_exist('phase-1');
        $this->assertTrue(is_int($result) || $result === false);
    }
}
