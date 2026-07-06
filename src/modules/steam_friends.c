#include "../php_steamworks.h"
#include "../steam_api_c.h"

PHP_FUNCTION(steam_friends_get_name)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamFriends *friends = SteamAPI_SteamFriends_v018();
    if (!friends) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    const char *name = SteamAPI_ISteamFriends_GetPersonaName(friends);
    RETURN_STRING(name);
}

PHP_FUNCTION(steam_friends_set_rich_presence)
{
    zend_string *key, *value = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STR(key)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(value)
    ZEND_PARSE_PARAMETERS_END();

    ISteamFriends *friends = SteamAPI_SteamFriends_v018();
    if (!friends) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    const char *val = value ? ZSTR_VAL(value) : "";
    RETURN_BOOL(SteamAPI_ISteamFriends_SetRichPresence(friends, ZSTR_VAL(key), val));
}

PHP_FUNCTION(steam_friends_activate_overlay)
{
    zend_string *dialog;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(dialog)
    ZEND_PARSE_PARAMETERS_END();

    ISteamFriends *friends = SteamAPI_SteamFriends_v018();
    if (!friends) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPI_ISteamFriends_ActivateGameOverlay(friends, ZSTR_VAL(dialog));
}

PHP_FUNCTION(steam_friends_activate_overlay_to_web_page)
{
    zend_string *url;
    zend_bool modal = 0;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STR(url)
        Z_PARAM_OPTIONAL
        Z_PARAM_BOOL(modal)
    ZEND_PARSE_PARAMETERS_END();

    ISteamFriends *friends = SteamAPI_SteamFriends_v018();
    if (!friends) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    SteamAPI_ISteamFriends_ActivateGameOverlayToWebPage(friends, ZSTR_VAL(url), modal ? 1 : 0);
}

/* ── Friends list, persona state & avatars (all synchronous) ────────────────
 * Persona info and avatars are only available for users Steam already has cached
 * (your friends, people in your current game, etc.). For others, first call
 * steam_friends_request_user_information(); the data becomes available a moment
 * later once Steam has downloaded it.
 */

PHP_FUNCTION(steam_friends_get_persona_state)
{
    ZEND_PARSE_PARAMETERS_NONE();

    ISteamFriends *friends = SteamAPI_SteamFriends_v018();
    if (!friends) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_LONG((zend_long)SteamAPI_ISteamFriends_GetPersonaState(friends));
}

PHP_FUNCTION(steam_friends_get_friend_count)
{
    zend_long flags = 0x04; /* k_EFriendFlagImmediate — the regular friends list. */

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(flags)
    ZEND_PARSE_PARAMETERS_END();

    ISteamFriends *friends = SteamAPI_SteamFriends_v018();
    if (!friends) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_LONG((zend_long)SteamAPI_ISteamFriends_GetFriendCount(friends, (int)flags));
}

PHP_FUNCTION(steam_friends_get_friend_by_index)
{
    zend_long index;
    zend_long flags = 0x04; /* k_EFriendFlagImmediate */

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_LONG(index)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(flags)
    ZEND_PARSE_PARAMETERS_END();

    ISteamFriends *friends = SteamAPI_SteamFriends_v018();
    if (!friends) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    uint64_steamid id = SteamAPI_ISteamFriends_GetFriendByIndex(friends, (int)index, (int)flags);
    RETURN_LONG((zend_long)id);
}

PHP_FUNCTION(steam_friends_get_friend_relationship)
{
    zend_long steam_id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(steam_id)
    ZEND_PARSE_PARAMETERS_END();

    ISteamFriends *friends = SteamAPI_SteamFriends_v018();
    if (!friends) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_LONG((zend_long)SteamAPI_ISteamFriends_GetFriendRelationship(friends, (uint64_steamid)steam_id));
}

PHP_FUNCTION(steam_friends_get_friend_persona_state)
{
    zend_long steam_id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(steam_id)
    ZEND_PARSE_PARAMETERS_END();

    ISteamFriends *friends = SteamAPI_SteamFriends_v018();
    if (!friends) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    RETURN_LONG((zend_long)SteamAPI_ISteamFriends_GetFriendPersonaState(friends, (uint64_steamid)steam_id));
}

PHP_FUNCTION(steam_friends_get_friend_persona_name)
{
    zend_long steam_id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(steam_id)
    ZEND_PARSE_PARAMETERS_END();

    ISteamFriends *friends = SteamAPI_SteamFriends_v018();
    if (!friends) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    const char *name = SteamAPI_ISteamFriends_GetFriendPersonaName(friends, (uint64_steamid)steam_id);
    RETURN_STRING(name ? name : "");
}

PHP_FUNCTION(steam_friends_request_user_information)
{
    zend_long steam_id;
    zend_bool name_only = 0;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_LONG(steam_id)
        Z_PARAM_OPTIONAL
        Z_PARAM_BOOL(name_only)
    ZEND_PARSE_PARAMETERS_END();

    ISteamFriends *friends = SteamAPI_SteamFriends_v018();
    if (!friends) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_FALSE;
    }

    /* Returns true if information is being requested (data not yet available),
       false if it is already available locally. */
    RETURN_BOOL(SteamAPI_ISteamFriends_RequestUserInformation(
        friends, (uint64_steamid)steam_id, name_only ? 1 : 0));
}

PHP_FUNCTION(steam_friends_get_friend_avatar)
{
    zend_long steam_id;
    zend_long size = 1; /* 0 = small (32), 1 = medium (64), 2 = large (128). */

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_LONG(steam_id)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(size)
    ZEND_PARSE_PARAMETERS_END();

    ISteamFriends *friends = SteamAPI_SteamFriends_v018();
    ISteamUtils   *utils   = SteamAPI_SteamUtils_v010();
    if (!friends || !utils) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_NULL();
    }

    int image;
    switch (size) {
        case 0:  image = SteamAPI_ISteamFriends_GetSmallFriendAvatar(friends, (uint64_steamid)steam_id);  break;
        case 2:  image = SteamAPI_ISteamFriends_GetLargeFriendAvatar(friends, (uint64_steamid)steam_id);  break;
        default: image = SteamAPI_ISteamFriends_GetMediumFriendAvatar(friends, (uint64_steamid)steam_id); break;
    }

    /* 0 = no avatar / not cached, -1 = still loading (request_user_information). */
    if (image <= 0) {
        RETURN_NULL();
    }

    uint32 width = 0, height = 0;
    if (!SteamAPI_ISteamUtils_GetImageSize(utils, image, &width, &height)
        || width == 0 || height == 0) {
        RETURN_NULL();
    }

    size_t bytes = (size_t)width * (size_t)height * 4;
    zend_string *buf = zend_string_alloc(bytes, 0);
    if (!SteamAPI_ISteamUtils_GetImageRGBA(utils, image, (uint8 *)ZSTR_VAL(buf), (int)bytes)) {
        zend_string_release(buf);
        RETURN_NULL();
    }
    ZSTR_VAL(buf)[bytes] = '\0';

    array_init(return_value);
    add_assoc_long(return_value, "width",  (zend_long)width);
    add_assoc_long(return_value, "height", (zend_long)height);
    add_assoc_str(return_value, "rgba", buf); /* raw RGBA8888, width*height*4 bytes */
}
