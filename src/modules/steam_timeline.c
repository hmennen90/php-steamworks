#include "../php_steamworks.h"
#include "../steam_api_c.h"

/* ── ISteamTimeline (Game Recording / Timeline, SDK V004) ──────────────────
 *
 * Lets a game annotate the Steam Game Recording timeline: mark the current
 * game mode, describe the current state (tooltip), drop instantaneous or
 * ranged events (which can trigger clips), and group play into "game phases"
 * with tags/attributes for later browsing.
 *
 * Most calls are fire-and-forget (void). Two are asynchronous and return a
 * SteamAPICall_t handle: does_event_recording_exist and
 * does_game_phase_recording_exist — poll steam_get_call_result($handle).
 *
 * Accessor (SteamTimeline_v004) and all flat signatures verified against
 * Steamworks SDK 1.64 (STEAMTIMELINE_INTERFACE_V004).
 */

static ISteamTimeline *steamworks_timeline(void)
{
    return SteamAPI_SteamTimeline_v004();
}

PHP_FUNCTION(steam_timeline_set_game_mode)
{
    zend_long mode;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(mode)
    ZEND_PARSE_PARAMETERS_END();

    ISteamTimeline *tl = steamworks_timeline();
    if (!tl) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPI_ISteamTimeline_SetTimelineGameMode(tl, (ETimelineGameMode)mode);
    RETURN_TRUE;
}

PHP_FUNCTION(steam_timeline_set_tooltip)
{
    zend_string *description;
    double       time_delta = 0.0;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STR(description)
        Z_PARAM_OPTIONAL
        Z_PARAM_DOUBLE(time_delta)
    ZEND_PARSE_PARAMETERS_END();

    ISteamTimeline *tl = steamworks_timeline();
    if (!tl) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPI_ISteamTimeline_SetTimelineTooltip(tl, ZSTR_VAL(description), (float)time_delta);
    RETURN_TRUE;
}

PHP_FUNCTION(steam_timeline_clear_tooltip)
{
    double time_delta = 0.0;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_DOUBLE(time_delta)
    ZEND_PARSE_PARAMETERS_END();

    ISteamTimeline *tl = steamworks_timeline();
    if (!tl) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPI_ISteamTimeline_ClearTimelineTooltip(tl, (float)time_delta);
    RETURN_TRUE;
}

PHP_FUNCTION(steam_timeline_add_instantaneous_event)
{
    zend_string *title;
    zend_string *description;
    zend_string *icon;
    zend_long    icon_priority        = 0;
    double       start_offset_seconds = 0.0;
    zend_long    possible_clip        = k_ETimelineEventClipPriority_None;

    ZEND_PARSE_PARAMETERS_START(3, 6)
        Z_PARAM_STR(title)
        Z_PARAM_STR(description)
        Z_PARAM_STR(icon)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(icon_priority)
        Z_PARAM_DOUBLE(start_offset_seconds)
        Z_PARAM_LONG(possible_clip)
    ZEND_PARSE_PARAMETERS_END();

    ISteamTimeline *tl = steamworks_timeline();
    if (!tl) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    TimelineEventHandle_t event = SteamAPI_ISteamTimeline_AddInstantaneousTimelineEvent(
        tl, ZSTR_VAL(title), ZSTR_VAL(description), ZSTR_VAL(icon),
        (uint32)icon_priority, (float)start_offset_seconds,
        (ETimelineEventClipPriority)possible_clip);
    RETURN_LONG((zend_long)event);
}

PHP_FUNCTION(steam_timeline_add_range_event)
{
    zend_string *title;
    zend_string *description;
    zend_string *icon;
    zend_long    icon_priority        = 0;
    double       start_offset_seconds = 0.0;
    double       duration_seconds     = 0.0;
    zend_long    possible_clip        = k_ETimelineEventClipPriority_None;

    ZEND_PARSE_PARAMETERS_START(3, 7)
        Z_PARAM_STR(title)
        Z_PARAM_STR(description)
        Z_PARAM_STR(icon)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(icon_priority)
        Z_PARAM_DOUBLE(start_offset_seconds)
        Z_PARAM_DOUBLE(duration_seconds)
        Z_PARAM_LONG(possible_clip)
    ZEND_PARSE_PARAMETERS_END();

    ISteamTimeline *tl = steamworks_timeline();
    if (!tl) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    TimelineEventHandle_t event = SteamAPI_ISteamTimeline_AddRangeTimelineEvent(
        tl, ZSTR_VAL(title), ZSTR_VAL(description), ZSTR_VAL(icon),
        (uint32)icon_priority, (float)start_offset_seconds, (float)duration_seconds,
        (ETimelineEventClipPriority)possible_clip);
    RETURN_LONG((zend_long)event);
}

PHP_FUNCTION(steam_timeline_start_range_event)
{
    zend_string *title;
    zend_string *description;
    zend_string *icon;
    zend_long    icon_priority        = 0;
    double       start_offset_seconds = 0.0;
    zend_long    possible_clip        = k_ETimelineEventClipPriority_None;

    ZEND_PARSE_PARAMETERS_START(3, 6)
        Z_PARAM_STR(title)
        Z_PARAM_STR(description)
        Z_PARAM_STR(icon)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(icon_priority)
        Z_PARAM_DOUBLE(start_offset_seconds)
        Z_PARAM_LONG(possible_clip)
    ZEND_PARSE_PARAMETERS_END();

    ISteamTimeline *tl = steamworks_timeline();
    if (!tl) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    TimelineEventHandle_t event = SteamAPI_ISteamTimeline_StartRangeTimelineEvent(
        tl, ZSTR_VAL(title), ZSTR_VAL(description), ZSTR_VAL(icon),
        (uint32)icon_priority, (float)start_offset_seconds,
        (ETimelineEventClipPriority)possible_clip);
    RETURN_LONG((zend_long)event);
}

PHP_FUNCTION(steam_timeline_update_range_event)
{
    zend_long    event;
    zend_string *title;
    zend_string *description;
    zend_string *icon;
    zend_long    icon_priority = 0;
    zend_long    possible_clip = k_ETimelineEventClipPriority_None;

    ZEND_PARSE_PARAMETERS_START(4, 6)
        Z_PARAM_LONG(event)
        Z_PARAM_STR(title)
        Z_PARAM_STR(description)
        Z_PARAM_STR(icon)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(icon_priority)
        Z_PARAM_LONG(possible_clip)
    ZEND_PARSE_PARAMETERS_END();

    ISteamTimeline *tl = steamworks_timeline();
    if (!tl) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPI_ISteamTimeline_UpdateRangeTimelineEvent(
        tl, (TimelineEventHandle_t)event, ZSTR_VAL(title), ZSTR_VAL(description),
        ZSTR_VAL(icon), (uint32)icon_priority,
        (ETimelineEventClipPriority)possible_clip);
    RETURN_TRUE;
}

PHP_FUNCTION(steam_timeline_end_range_event)
{
    zend_long event;
    double    end_offset_seconds = 0.0;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_LONG(event)
        Z_PARAM_OPTIONAL
        Z_PARAM_DOUBLE(end_offset_seconds)
    ZEND_PARSE_PARAMETERS_END();

    ISteamTimeline *tl = steamworks_timeline();
    if (!tl) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPI_ISteamTimeline_EndRangeTimelineEvent(
        tl, (TimelineEventHandle_t)event, (float)end_offset_seconds);
    RETURN_TRUE;
}

PHP_FUNCTION(steam_timeline_remove_event)
{
    zend_long event;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(event)
    ZEND_PARSE_PARAMETERS_END();

    ISteamTimeline *tl = steamworks_timeline();
    if (!tl) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPI_ISteamTimeline_RemoveTimelineEvent(tl, (TimelineEventHandle_t)event);
    RETURN_TRUE;
}

PHP_FUNCTION(steam_timeline_does_event_recording_exist)
{
    zend_long event;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(event)
    ZEND_PARSE_PARAMETERS_END();

    ISteamTimeline *tl = steamworks_timeline();
    if (!tl) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPICall_t call = SteamAPI_ISteamTimeline_DoesEventRecordingExist(
        tl, (TimelineEventHandle_t)event);
    if (call == 0) {
        RETURN_FALSE;
    }
    steamworks_register_call(call, STEAMWORKS_CALL_TIMELINE_EVENT_RECORDING);
    RETURN_LONG((zend_long)call);
}

PHP_FUNCTION(steam_timeline_start_game_phase)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamTimeline *tl = steamworks_timeline();
    if (!tl) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPI_ISteamTimeline_StartGamePhase(tl);
    RETURN_TRUE;
}

PHP_FUNCTION(steam_timeline_end_game_phase)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamTimeline *tl = steamworks_timeline();
    if (!tl) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPI_ISteamTimeline_EndGamePhase(tl);
    RETURN_TRUE;
}

PHP_FUNCTION(steam_timeline_set_game_phase_id)
{
    zend_string *phase_id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(phase_id)
    ZEND_PARSE_PARAMETERS_END();

    ISteamTimeline *tl = steamworks_timeline();
    if (!tl) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPI_ISteamTimeline_SetGamePhaseID(tl, ZSTR_VAL(phase_id));
    RETURN_TRUE;
}

PHP_FUNCTION(steam_timeline_does_game_phase_recording_exist)
{
    zend_string *phase_id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(phase_id)
    ZEND_PARSE_PARAMETERS_END();

    ISteamTimeline *tl = steamworks_timeline();
    if (!tl) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPICall_t call = SteamAPI_ISteamTimeline_DoesGamePhaseRecordingExist(
        tl, ZSTR_VAL(phase_id));
    if (call == 0) {
        RETURN_FALSE;
    }
    steamworks_register_call(call, STEAMWORKS_CALL_TIMELINE_PHASE_RECORDING);
    RETURN_LONG((zend_long)call);
}

PHP_FUNCTION(steam_timeline_add_game_phase_tag)
{
    zend_string *tag_name;
    zend_string *tag_icon;
    zend_string *tag_group;
    zend_long    priority = 0;

    ZEND_PARSE_PARAMETERS_START(3, 4)
        Z_PARAM_STR(tag_name)
        Z_PARAM_STR(tag_icon)
        Z_PARAM_STR(tag_group)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(priority)
    ZEND_PARSE_PARAMETERS_END();

    ISteamTimeline *tl = steamworks_timeline();
    if (!tl) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPI_ISteamTimeline_AddGamePhaseTag(
        tl, ZSTR_VAL(tag_name), ZSTR_VAL(tag_icon), ZSTR_VAL(tag_group),
        (uint32)priority);
    RETURN_TRUE;
}

PHP_FUNCTION(steam_timeline_set_game_phase_attribute)
{
    zend_string *attribute_group;
    zend_string *attribute_value;
    zend_long    priority = 0;

    ZEND_PARSE_PARAMETERS_START(2, 3)
        Z_PARAM_STR(attribute_group)
        Z_PARAM_STR(attribute_value)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(priority)
    ZEND_PARSE_PARAMETERS_END();

    ISteamTimeline *tl = steamworks_timeline();
    if (!tl) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPI_ISteamTimeline_SetGamePhaseAttribute(
        tl, ZSTR_VAL(attribute_group), ZSTR_VAL(attribute_value), (uint32)priority);
    RETURN_TRUE;
}

PHP_FUNCTION(steam_timeline_open_overlay_to_game_phase)
{
    zend_string *phase_id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(phase_id)
    ZEND_PARSE_PARAMETERS_END();

    ISteamTimeline *tl = steamworks_timeline();
    if (!tl) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPI_ISteamTimeline_OpenOverlayToGamePhase(tl, ZSTR_VAL(phase_id));
    RETURN_TRUE;
}

PHP_FUNCTION(steam_timeline_open_overlay_to_event)
{
    zend_long event;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(event)
    ZEND_PARSE_PARAMETERS_END();

    ISteamTimeline *tl = steamworks_timeline();
    if (!tl) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPI_ISteamTimeline_OpenOverlayToTimelineEvent(tl, (TimelineEventHandle_t)event);
    RETURN_TRUE;
}
