/*
 * Mock implementation of the Steamworks flat API for CI testing.
 * All functions return safe default/failure values.
 */

#include "public/steam/steam_api_flat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int mock_instance = 0;

/* Core */
ESteamAPIInitResult SteamAPI_InitFlat(SteamErrMsg *pOutErrMsg) {
    if (pOutErrMsg) { (*pOutErrMsg)[0] = '\0'; }
    return k_ESteamAPIInitResult_NoSteamClient;
}
void SteamAPI_Shutdown(void) { }
void SteamAPI_RunCallbacks(void) { }
/* Registered callbacks, so the test hooks below can deliver to them. */
#define MOCK_MAX_CALLBACKS 32
static struct { void *cb; int id; } mock_callbacks[MOCK_MAX_CALLBACKS];
static int mock_callback_count = 0;

void SteamAPI_RegisterCallback(void *callback, int iCallback) {
    if (mock_callback_count < MOCK_MAX_CALLBACKS) {
        mock_callbacks[mock_callback_count].cb = callback;
        mock_callbacks[mock_callback_count].id = iCallback;
        mock_callback_count++;
    }
}
void SteamAPI_UnregisterCallback(void *callback) {
    for (int i = 0; i < mock_callback_count; i++) {
        if (mock_callbacks[i].cb == callback) {
            mock_callbacks[i] = mock_callbacks[--mock_callback_count];
            return;
        }
    }
}

/* Deliver like SteamAPI_RunCallbacks: CCallbackBase's first vtable slot is
   Run(this, pvParam). */
typedef void (*mock_run_fn)(void *self, void *param);
static void mock_dispatch(int id, void *param) {
    for (int i = 0; i < mock_callback_count; i++) {
        if (mock_callbacks[i].id == id) {
            mock_run_fn run = (*(mock_run_fn **)mock_callbacks[i].cb)[0];
            run(mock_callbacks[i].cb, param);
        }
    }
}

void SteamMock_FireRichPresenceJoinRequested(uint64_t friend_id, const char *connect) {
    GameRichPresenceJoinRequested_t r;
    memset(&r, 0, sizeof(r));
    r.m_steamIDFriend = friend_id;
    snprintf(r.m_rgchConnect, sizeof(r.m_rgchConnect), "%s", connect ? connect : "");
    mock_dispatch(k_iCallback_GameRichPresenceJoinRequested, &r);
}

void SteamMock_FireLobbyJoinRequested(uint64_t lobby, uint64_t friend_id) {
    GameLobbyJoinRequested_t r;
    memset(&r, 0, sizeof(r));
    r.m_steamIDLobby  = lobby;
    r.m_steamIDFriend = friend_id;
    mock_dispatch(k_iCallback_GameLobbyJoinRequested, &r);
}

void SteamMock_FireLobbyDataUpdate(uint64_t lobby, uint64_t member, bool success) {
    LobbyDataUpdate_t r;
    memset(&r, 0, sizeof(r));
    r.m_ulSteamIDLobby  = lobby;
    r.m_ulSteamIDMember = member;
    r.m_bSuccess        = success ? 1 : 0;
    mock_dispatch(k_iCallback_LobbyDataUpdate, &r);
}

void SteamMock_FireLobbyChatUpdate(uint64_t lobby, uint64_t user, uint64_t changed_by, uint32 state) {
    LobbyChatUpdate_t r;
    memset(&r, 0, sizeof(r));
    r.m_ulSteamIDLobby           = lobby;
    r.m_ulSteamIDUserChanged     = user;
    r.m_ulSteamIDMakingChange    = changed_by;
    r.m_rgfChatMemberStateChange = state;
    mock_dispatch(k_iCallback_LobbyChatUpdate, &r);
}

/* Lobby chat: messages are stored by chat id, the callback carries only the id
   and GetLobbyChatEntry reads the bytes back — as with Steam. */
#define MOCK_CHAT_SLOTS 16
static struct { uint64_t user; int len; char data[4096]; } mock_chat[MOCK_CHAT_SLOTS];
static uint32 mock_chat_next = 0;

void SteamMock_FireLobbyChatMsg(uint64_t lobby, uint64_t user, const char *message, int len) {
    uint32 id = mock_chat_next++;
    int slot = (int)(id % MOCK_CHAT_SLOTS);
    if (len < 0) { len = 0; }
    if (len > (int)sizeof(mock_chat[slot].data)) { len = (int)sizeof(mock_chat[slot].data); }
    mock_chat[slot].user = user;
    mock_chat[slot].len  = len;
    if (len > 0) { memcpy(mock_chat[slot].data, message, (size_t)len); }

    LobbyChatMsg_t r;
    memset(&r, 0, sizeof(r));
    r.m_ulSteamIDLobby = lobby;
    r.m_ulSteamIDUser  = user;
    r.m_eChatEntryType = 1; /* k_EChatEntryTypeChatMsg */
    r.m_iChatID        = id;
    mock_dispatch(k_iCallback_LobbyChatMsg, &r);
}

/*
 * Runtime interface lookup.
 *
 * Mirrors libsteam_api: the caller asks for an interface by version string and
 * gets NULL when the Steam client does not implement it. Keeping the mock
 * strict is what lets CI catch a wrong or stale version string — with a
 * permissive mock the interface would simply fail to resolve and every function
 * would return its normal failure value, indistinguishable from "Steam is not
 * running".
 *
 * The strings below are copied from the SDK's *_INTERFACE_VERSION defines and
 * must stay in sync with the tables in src/steam_iface.c.
 */

/* Versions this mock implements — Steamworks SDK 1.65 */
static const char *const mock_offered[] = {
    "SteamUser023",
    "SteamFriends018",
    "STEAMUSERSTATS_INTERFACE_VERSION013",
    "STEAMREMOTESTORAGE_INTERFACE_VERSION016",
    "STEAMAPPS_INTERFACE_VERSION009",
    "SteamUtils011",
    "STEAMTIMELINE_INTERFACE_V004",
    "STEAMUGC_INTERFACE_VERSION021",
    "SteamNetworkingSockets013",
    "SteamNetworkingUtils004",
    "SteamMatchMaking009",
    NULL
};

/* Legacy versions the extension may probe as a fallback — recognised, not offered. */
static const char *const mock_legacy[] = {
    "SteamUtils010",              /* SDK 1.61–1.64 */
    "SteamNetworkingSockets012",  /* SDK <= 1.64 */
    NULL
};

static int mock_in_list(const char *const *list, const char *version) {
    for (int i = 0; list[i] != NULL; i++) {
        if (strcmp(list[i], version) == 0) { return 1; }
    }
    return 0;
}

/* Simulates a Steam client that predates SDK 1.65, to exercise the fallback. */
static int mock_env_flag(const char *name) {
    const char *flag = getenv(name);
    return flag != NULL && flag[0] == '1';
}

HSteamUser SteamAPI_GetHSteamUser(void) { return 1; }

void *SteamInternal_FindOrCreateUserInterface(HSteamUser hSteamUser, const char *pszVersion) {
    (void)hSteamUser;

    if (pszVersion == NULL) { return NULL; }

    if (strncmp(pszVersion, "SteamUtils", 10) == 0) {
        const char *available = mock_env_flag("STEAMWORKS_MOCK_LEGACY_UTILS") ? "SteamUtils010" : "SteamUtils011";
        return strcmp(pszVersion, available) == 0 ? (void*)&mock_instance : NULL;
    }

    if (strncmp(pszVersion, "SteamNetworkingSockets", 22) == 0) {
        const char *available = mock_env_flag("STEAMWORKS_MOCK_LEGACY_NET")
            ? "SteamNetworkingSockets012" : "SteamNetworkingSockets013";
        return strcmp(pszVersion, available) == 0 ? (void*)&mock_instance : NULL;
    }

    if (mock_in_list(mock_offered, pszVersion)) { return (void*)&mock_instance; }
    if (mock_in_list(mock_legacy,  pszVersion)) { return NULL; }

    fprintf(stderr, "MOCK: unknown interface version \"%s\"\n", pszVersion);
    return NULL;
}

/* ISteamUser */
uint64_steamid SteamAPI_ISteamUser_GetSteamID(ISteamUser *self) { return 0; }
bool           SteamAPI_ISteamUser_BLoggedOn(ISteamUser *self) { return false; }
int            SteamAPI_ISteamUser_GetPlayerSteamLevel(ISteamUser *self) { return 0; }
HAuthTicket    SteamAPI_ISteamUser_GetAuthSessionTicket(ISteamUser *self, void *ticket, int max_ticket, uint32 *ticket_size, const void *identity) {
    /* Return a fake 8-byte ticket + handle so the encode path is exercised. */
    int n = max_ticket < 8 ? max_ticket : 8;
    if (ticket) { for (int i = 0; i < n; i++) { ((unsigned char *)ticket)[i] = (unsigned char)(0xA0 + i); } }
    if (ticket_size) { *ticket_size = (uint32)n; }
    return 1; /* fake HAuthTicket */
}
HAuthTicket    SteamAPI_ISteamUser_GetAuthTicketForWebApi(ISteamUser *self, const char *identity) { return 2; } /* fake handle; response callback never fires in mock */
int            SteamAPI_ISteamUser_BeginAuthSession(ISteamUser *self, const void *auth_ticket, int ticket_size, uint64_steamid steam_id) { return 0; } /* k_EBeginAuthSessionResultOK */
void           SteamAPI_ISteamUser_EndAuthSession(ISteamUser *self, uint64_steamid steam_id) { }
void           SteamAPI_ISteamUser_CancelAuthTicket(ISteamUser *self, HAuthTicket handle) { }

/* ISteamFriends */
const char* SteamAPI_ISteamFriends_GetPersonaName(ISteamFriends *self) { return "MockPlayer"; }
bool        SteamAPI_ISteamFriends_SetRichPresence(ISteamFriends *self, const char *key, const char *value) { return false; }
void        SteamAPI_ISteamFriends_ActivateGameOverlay(ISteamFriends *self, const char *dialog) { }
void        SteamAPI_ISteamFriends_ActivateGameOverlayToWebPage(ISteamFriends *self, const char *url, int eMode) { }
int         SteamAPI_ISteamFriends_GetPersonaState(ISteamFriends *self) { return 1; } /* online */
int         SteamAPI_ISteamFriends_GetFriendCount(ISteamFriends *self, int friend_flags) { return 2; }
uint64_steamid SteamAPI_ISteamFriends_GetFriendByIndex(ISteamFriends *self, int index, int friend_flags) { return 76561197960265728ULL + (uint64_steamid)index; }
int         SteamAPI_ISteamFriends_GetFriendRelationship(ISteamFriends *self, uint64_steamid steam_id) { return 3; } /* friend */
int         SteamAPI_ISteamFriends_GetFriendPersonaState(ISteamFriends *self, uint64_steamid steam_id) { return 1; }
const char* SteamAPI_ISteamFriends_GetFriendPersonaName(ISteamFriends *self, uint64_steamid steam_id) { return "MockFriend"; }
int         SteamAPI_ISteamFriends_GetSmallFriendAvatar(ISteamFriends *self, uint64_steamid steam_id) { return 5; }  /* fake image handle */
int         SteamAPI_ISteamFriends_GetMediumFriendAvatar(ISteamFriends *self, uint64_steamid steam_id) { return 5; }
int         SteamAPI_ISteamFriends_GetLargeFriendAvatar(ISteamFriends *self, uint64_steamid steam_id) { return 5; }
bool        SteamAPI_ISteamFriends_RequestUserInformation(ISteamFriends *self, uint64_steamid steam_id, bool require_name_only) { return false; } /* already available */

/* Invites and joins. Friend ...729 plays app 480 in lobby 109775240910000001
   and advertises it as rich presence "connect"; everyone else is offline. */
#define MOCK_FRIEND_IN_GAME 76561197960265729ULL
#define MOCK_LOBBY          109775240910000001ULL
bool SteamAPI_ISteamFriends_InviteUserToGame(ISteamFriends *self, uint64_steamid friend_id, const char *connect) {
    return connect != NULL && connect[0] != '\0';
}
void SteamAPI_ISteamFriends_ActivateGameOverlayInviteDialogConnectString(ISteamFriends *self, const char *connect) { }
const char *SteamAPI_ISteamFriends_GetFriendRichPresence(ISteamFriends *self, uint64_steamid friend_id, const char *key) {
    if (friend_id == MOCK_FRIEND_IN_GAME && key && strcmp(key, "connect") == 0) {
        return "+connect_lobby 109775240910000001";
    }
    return "";
}
bool SteamAPI_ISteamFriends_GetFriendGamePlayed(ISteamFriends *self, uint64_steamid friend_id, FriendGameInfo_t *info) {
    if (friend_id != MOCK_FRIEND_IN_GAME || !info) { return false; }
    memset(info, 0, sizeof(*info));
    info->m_gameID       = 480;
    info->m_usQueryPort  = 0xFFFF; /* k_usFriendGameInfoQueryPort_NotInitialized */
    info->m_steamIDLobby = MOCK_LOBBY;
    return true;
}

/* ISteamUserStats */
bool SteamAPI_ISteamUserStats_SetAchievement(ISteamUserStats *self, const char *name)   { return false; }
bool SteamAPI_ISteamUserStats_ClearAchievement(ISteamUserStats *self, const char *name) { return false; }
bool SteamAPI_ISteamUserStats_StoreStats(ISteamUserStats *self)                          { return false; }
bool SteamAPI_ISteamUserStats_GetStatInt32(ISteamUserStats *self, const char *name, int32 *data)  { *data = 0; return false; }
bool SteamAPI_ISteamUserStats_SetStatInt32(ISteamUserStats *self, const char *name, int32 data)   { return false; }
bool SteamAPI_ISteamUserStats_GetStatFloat(ISteamUserStats *self, const char *name, float *data)  { *data = 0.0f; return false; }
bool SteamAPI_ISteamUserStats_SetStatFloat(ISteamUserStats *self, const char *name, float data)   { return false; }
bool SteamAPI_ISteamUserStats_IndicateAchievementProgress(ISteamUserStats *self, const char *name, uint32 cur_progress, uint32 max_progress) { return false; }

/* ISteamUserStats — Achievement read path.
 * Deterministic values: 3 achievements, none unlocked, fixed display strings. */
bool SteamAPI_ISteamUserStats_GetAchievement(ISteamUserStats *self, const char *name, bool *achieved) { if (achieved) { *achieved = false; } return true; }
bool SteamAPI_ISteamUserStats_GetAchievementAndUnlockTime(ISteamUserStats *self, const char *name, bool *achieved, uint32 *unlock_time) { if (achieved) { *achieved = false; } if (unlock_time) { *unlock_time = 0; } return true; }
uint32 SteamAPI_ISteamUserStats_GetNumAchievements(ISteamUserStats *self) { return 3; }
const char* SteamAPI_ISteamUserStats_GetAchievementName(ISteamUserStats *self, uint32 achievement) { return achievement < 3 ? "ACH_MOCK" : ""; }
const char* SteamAPI_ISteamUserStats_GetAchievementDisplayAttribute(ISteamUserStats *self, const char *name, const char *key) { return "Mock Achievement"; }
bool SteamAPI_ISteamUserStats_ResetAllStats(ISteamUserStats *self, bool achievements_too) { return true; }

/* ISteamUserStats — Leaderboards (async).
 * Return deterministic fake handles so the find->poll->result flow is testable:
 *   1 = find/find_or_create, 2 = upload, 3 = download. Leaderboard handle = 42. */
SteamAPICall_t SteamAPI_ISteamUserStats_FindLeaderboard(ISteamUserStats *self, const char *name) { return 1; }
SteamAPICall_t SteamAPI_ISteamUserStats_FindOrCreateLeaderboard(ISteamUserStats *self, const char *name, ELeaderboardSortMethod sort, ELeaderboardDisplayType display) { return 1; }
SteamAPICall_t SteamAPI_ISteamUserStats_UploadLeaderboardScore(ISteamUserStats *self, SteamLeaderboard_t leaderboard, ELeaderboardUploadScoreMethod method, int32 score, const int32 *details, int details_count) { return 2; }
SteamAPICall_t SteamAPI_ISteamUserStats_DownloadLeaderboardEntries(ISteamUserStats *self, SteamLeaderboard_t leaderboard, ELeaderboardDataRequest request, int range_start, int range_end) { return 3; }
bool SteamAPI_ISteamUserStats_GetDownloadedLeaderboardEntry(ISteamUserStats *self, SteamLeaderboardEntries_t entries, int index, LeaderboardEntry_t *entry, int32 *details, int details_max) {
    /* Emit two fake detail values so the details read path is exercised. */
    int32 want = 2;
    int32 n = (details_max < want) ? details_max : want;
    if (entry) {
        entry->m_steamIDUser = 76561197960265728ULL;
        entry->m_nGlobalRank = index + 1;
        entry->m_nScore      = 1000 - index;
        entry->m_cDetails    = n;
        /* Entry 0 carries an attached file, the rest none (k_UGCHandleInvalid),
           so both sides of the "ugc" key are exercised. */
        entry->m_hUGC        = (index == 0) ? 77 : 0xffffffffffffffffULL;
    }
    if (details) {
        for (int32 i = 0; i < n; i++) {
            details[i] = (index + 1) * 100 + i;
        }
    }
    return true;
}
int SteamAPI_ISteamUserStats_GetLeaderboardEntryCount(ISteamUserStats *self, SteamLeaderboard_t leaderboard) { return 5; }
/* Phase 4a handles: 4 = attach, 5 = file share, 6 = ugc download. Shared file = UGC 77. */
SteamAPICall_t SteamAPI_ISteamUserStats_AttachLeaderboardUGC(ISteamUserStats *self, SteamLeaderboard_t leaderboard, UGCHandle_t ugc) { return 4; }

/* ISteamRemoteStorage */
bool        SteamAPI_ISteamRemoteStorage_FileWrite(ISteamRemoteStorage *self, const char *file, const void *data, int32 size) { return false; }
int32       SteamAPI_ISteamRemoteStorage_GetFileSize(ISteamRemoteStorage *self, const char *file) { return 0; }
int32       SteamAPI_ISteamRemoteStorage_FileRead(ISteamRemoteStorage *self, const char *file, void *data, int32 size) { return 0; }
bool        SteamAPI_ISteamRemoteStorage_FileExists(ISteamRemoteStorage *self, const char *file) { return false; }
bool        SteamAPI_ISteamRemoteStorage_FileDelete(ISteamRemoteStorage *self, const char *file) { return false; }
int32       SteamAPI_ISteamRemoteStorage_GetFileCount(ISteamRemoteStorage *self) { return 0; }
const char* SteamAPI_ISteamRemoteStorage_GetFileNameAndSize(ISteamRemoteStorage *self, int32 index, int32 *size) { *size = 0; return ""; }

/* Shared file 77 holds this payload, so share → attach → download → read runs end to end. */
static const char mock_ugc_payload[] = "{\"company\":\"Mock GmbH\",\"value\":1234}";
SteamAPICall_t SteamAPI_ISteamRemoteStorage_FileShare(ISteamRemoteStorage *self, const char *file) { return 5; }
SteamAPICall_t SteamAPI_ISteamRemoteStorage_UGCDownload(ISteamRemoteStorage *self, UGCHandle_t content, uint32 priority) { return 6; }
bool SteamAPI_ISteamRemoteStorage_GetUGCDetails(ISteamRemoteStorage *self, UGCHandle_t content, AppId_t *app_id, char **name, int32 *size, uint64_t *owner) {
    if (content != 77) { return false; }
    if (app_id) { *app_id = 480; }
    if (name)   { *name = (char *)"company.json"; }
    if (size)   { *size = (int32)(sizeof(mock_ugc_payload) - 1); }
    if (owner)  { *owner = 76561197960265728ULL; }
    return true;
}
int32 SteamAPI_ISteamRemoteStorage_UGCRead(ISteamRemoteStorage *self, UGCHandle_t content, void *data, int32 size, uint32 offset, EUGCReadAction action) {
    int32 total = (int32)(sizeof(mock_ugc_payload) - 1);
    if (content != 77 || !data || (int32)offset >= total) { return 0; }
    int32 n = total - (int32)offset;
    if (n > size) { n = size; }
    memcpy(data, mock_ugc_payload + offset, (size_t)n);
    return n;
}

/* ISteamApps */
bool           SteamAPI_ISteamApps_BIsSubscribed(ISteamApps *self) { return false; }
bool           SteamAPI_ISteamApps_BIsDlcInstalled(ISteamApps *self, AppId_t dlc_id) { return false; }
uint64_steamid SteamAPI_ISteamApps_GetAppOwner(ISteamApps *self) { return 0; }
const char*    SteamAPI_ISteamApps_GetCurrentGameLanguage(ISteamApps *self) { return "english"; }
bool           SteamAPI_ISteamApps_BIsSubscribedApp(ISteamApps *self, AppId_t appid) { return false; }
bool           SteamAPI_ISteamApps_GetCurrentBetaName(ISteamApps *self, char *name, int name_buffer_size) { return false; }
uint32         SteamAPI_ISteamApps_GetEarliestPurchaseUnixTime(ISteamApps *self, AppId_t appid) { return 0; }
uint32         SteamAPI_ISteamApps_GetInstalledDepots(ISteamApps *self, AppId_t appid, DepotId_t *depots, uint32 max_depots) {
    /* Two deterministic depots so the array path is testable. */
    uint32 n = 0;
    if (depots && max_depots > 0) { depots[n++] = 1001; }
    if (depots && max_depots > 1) { depots[n++] = 1002; }
    return n;
}
int            SteamAPI_ISteamApps_GetDLCCount(ISteamApps *self) { return 0; }
int            SteamAPI_ISteamApps_GetAppBuildId(ISteamApps *self) { return 10; }
/* Started through an accepted invite. */
int            SteamAPI_ISteamApps_GetLaunchCommandLine(ISteamApps *self, char *command_line, int size) {
    return command_line && size > 0
        ? snprintf(command_line, (size_t)size, "%s", "+connect_lobby 109775240910000001")
        : 0;
}

/* ISteamUtils */
AppId_t     SteamAPI_ISteamUtils_GetAppID(ISteamUtils *self) { return 480; }
bool        SteamAPI_ISteamUtils_IsOverlayEnabled(ISteamUtils *self) { return false; }
const char* SteamAPI_ISteamUtils_GetIPCountry(ISteamUtils *self) { return "US"; }
int         SteamAPI_ISteamUtils_IsRunningOnSteamHardware(ISteamUtils *self) { return 1; }        /* SteamDeck */
int         SteamAPI_ISteamUtils_GetSteamHardwareDefaultConfig(ISteamUtils *self) { return 5; }   /* SteamDeck preset */
bool        SteamAPI_ISteamUtils_IsRunningUnderProton(ISteamUtils *self) { return false; }
const char* SteamAPI_ISteamUtils_GetSteamUILanguage(ISteamUtils *self) { return "english"; }
uint32      SteamAPI_ISteamUtils_GetServerRealTime(ISteamUtils *self) { return 1609459200; } /* 2021-01-01 */
uint8_t     SteamAPI_ISteamUtils_GetCurrentBatteryPower(ISteamUtils *self) { return 255; }   /* on AC power */
uint32      SteamAPI_ISteamUtils_GetSecondsSinceAppActive(ISteamUtils *self) { return 0; }
bool        SteamAPI_ISteamUtils_GetImageSize(ISteamUtils *self, int image, uint32 *width, uint32 *height) {
    if (image <= 0) { return false; }
    if (width)  { *width  = 64; }
    if (height) { *height = 64; }
    return true;
}
bool        SteamAPI_ISteamUtils_GetImageRGBA(ISteamUtils *self, int image, uint8_t *dest, int dest_size) {
    if (image <= 0 || !dest) { return false; }
    for (int i = 0; i < dest_size; i++) { dest[i] = 0xFF; } /* opaque white */
    return true;
}

/* Async CallResult plumbing — pretend every call completed successfully and
 * hand back a deterministic result struct matching the expected callback id. */
bool SteamAPI_ISteamUtils_IsAPICallCompleted(ISteamUtils *self, SteamAPICall_t call, bool *failed) {
    if (failed) { *failed = false; }
    return true;
}
bool SteamAPI_ISteamUtils_GetAPICallResult(ISteamUtils *self, SteamAPICall_t call, void *callback, int callback_size, int callback_expected, bool *failed) {
    if (failed) { *failed = false; }
    if (!callback) { return false; }

    if (callback_expected == k_iCallback_LeaderboardFindResult
        && callback_size >= (int)sizeof(LeaderboardFindResult_t)) {
        LeaderboardFindResult_t *r = (LeaderboardFindResult_t *)callback;
        r->m_hSteamLeaderboard  = 42;
        r->m_bLeaderboardFound  = 1;
        return true;
    }
    if (callback_expected == k_iCallback_LeaderboardScoreUploaded
        && callback_size >= (int)sizeof(LeaderboardScoreUploaded_t)) {
        LeaderboardScoreUploaded_t *r = (LeaderboardScoreUploaded_t *)callback;
        r->m_bSuccess            = 1;
        r->m_hSteamLeaderboard   = 42;
        r->m_nScore              = 1000;
        r->m_bScoreChanged       = 1;
        r->m_nGlobalRankNew      = 1;
        r->m_nGlobalRankPrevious = 0;
        return true;
    }
    if (callback_expected == k_iCallback_LeaderboardScoresDownloaded
        && callback_size >= (int)sizeof(LeaderboardScoresDownloaded_t)) {
        LeaderboardScoresDownloaded_t *r = (LeaderboardScoresDownloaded_t *)callback;
        r->m_hSteamLeaderboard        = 42;
        r->m_hSteamLeaderboardEntries = 99;
        r->m_cEntryCount              = 5;
        return true;
    }
    if (callback_expected == k_iCallback_SteamTimelineEventRecordingExists
        && callback_size >= (int)sizeof(SteamTimelineEventRecordingExists_t)) {
        SteamTimelineEventRecordingExists_t *r = (SteamTimelineEventRecordingExists_t *)callback;
        r->m_ulEventID        = 7;
        r->m_bRecordingExists = 1;
        return true;
    }
    if (callback_expected == k_iCallback_SteamTimelineGamePhaseRecordingExists
        && callback_size >= (int)sizeof(SteamTimelineGamePhaseRecordingExists_t)) {
        SteamTimelineGamePhaseRecordingExists_t *r = (SteamTimelineGamePhaseRecordingExists_t *)callback;
        const char *id = "phase-1";
        int i = 0;
        for (; id[i] && i < k_cGamePhaseIDStrMaxLen - 1; i++) { r->m_rgchPhaseID[i] = id[i]; }
        r->m_rgchPhaseID[i]    = '\0';
        r->m_ulRecordingMS     = 60000;
        r->m_ulLongestClipMS   = 15000;
        r->m_unClipCount       = 2;
        r->m_unScreenshotCount = 3;
        return true;
    }
    if (callback_expected == k_iCallback_RemoteStorageSubscribePublishedFileResult
        && callback_size >= (int)sizeof(RemoteStorageSubscribePublishedFileResult_t)) {
        RemoteStorageSubscribePublishedFileResult_t *r = (RemoteStorageSubscribePublishedFileResult_t *)callback;
        r->m_eResult         = 1; /* k_EResultOK */
        r->m_nPublishedFileId = 123456;
        return true;
    }
    if (callback_expected == k_iCallback_RemoteStorageUnsubscribePublishedFileResult
        && callback_size >= (int)sizeof(RemoteStorageUnsubscribePublishedFileResult_t)) {
        RemoteStorageUnsubscribePublishedFileResult_t *r = (RemoteStorageUnsubscribePublishedFileResult_t *)callback;
        r->m_eResult         = 1;
        r->m_nPublishedFileId = 123456;
        return true;
    }
    if (callback_expected == k_iCallback_RemoteStorageFileShareResult
        && callback_size >= (int)sizeof(RemoteStorageFileShareResult_t)) {
        RemoteStorageFileShareResult_t *r = (RemoteStorageFileShareResult_t *)callback;
        r->m_eResult = 1;
        r->m_hFile   = 77;
        snprintf(r->m_rgchFilename, k_cchFilenameMax, "%s", "company.json");
        return true;
    }
    if (callback_expected == k_iCallback_RemoteStorageDownloadUGCResult
        && callback_size >= (int)sizeof(RemoteStorageDownloadUGCResult_t)) {
        RemoteStorageDownloadUGCResult_t *r = (RemoteStorageDownloadUGCResult_t *)callback;
        r->m_eResult        = 1;
        r->m_hFile          = 77;
        r->m_nAppID         = 480;
        r->m_nSizeInBytes   = (int32)(sizeof(mock_ugc_payload) - 1);
        snprintf(r->m_pchFileName, k_cchFilenameMax, "%s", "company.json");
        r->m_ulSteamIDOwner = 76561197960265728ULL;
        return true;
    }
    if (callback_expected == k_iCallback_LobbyCreated
        && callback_size >= (int)sizeof(LobbyCreated_t)) {
        LobbyCreated_t *r = (LobbyCreated_t *)callback;
        r->m_eResult        = 1;
        r->m_ulSteamIDLobby = MOCK_LOBBY;
        return true;
    }
    if (callback_expected == k_iCallback_LobbyEnter
        && callback_size >= (int)sizeof(LobbyEnter_t)) {
        LobbyEnter_t *r = (LobbyEnter_t *)callback;
        r->m_ulSteamIDLobby         = MOCK_LOBBY;
        r->m_rgfChatPermissions     = 0;
        r->m_bLocked                = 0;
        r->m_EChatRoomEnterResponse = 1; /* k_EChatRoomEnterResponseSuccess */
        return true;
    }
    if (callback_expected == k_iCallback_LeaderboardUGCSet
        && callback_size >= (int)sizeof(LeaderboardUGCSet_t)) {
        LeaderboardUGCSet_t *r = (LeaderboardUGCSet_t *)callback;
        r->m_eResult           = 1;
        r->m_hSteamLeaderboard = 42;
        return true;
    }
    return false;
}

/* ISteamMatchmaking — lobby 109775240910000001, owned by the local user
 * 76561197960265728, with friend 76561197960265729 as second member.
 * Handles: 20 = create lobby, 21 = join lobby. */
#define MOCK_ME 76561197960265728ULL
#define MOCK_KV_SLOTS 16
typedef struct { uint64_t user; char key[256]; char value[8192]; } mock_kv;
static mock_kv mock_lobby_data[MOCK_KV_SLOTS];
static mock_kv mock_member_data[MOCK_KV_SLOTS];

static const char *mock_kv_get(mock_kv *table, uint64_t user, const char *key) {
    for (int i = 0; i < MOCK_KV_SLOTS; i++) {
        if (table[i].key[0] && table[i].user == user && strcmp(table[i].key, key) == 0) {
            return table[i].value;
        }
    }
    return "";
}
static bool mock_kv_set(mock_kv *table, uint64_t user, const char *key, const char *value) {
    int free_slot = -1;
    for (int i = 0; i < MOCK_KV_SLOTS; i++) {
        if (table[i].key[0] && table[i].user == user && strcmp(table[i].key, key) == 0) {
            snprintf(table[i].value, sizeof(table[i].value), "%s", value);
            return true;
        }
        if (!table[i].key[0] && free_slot < 0) { free_slot = i; }
    }
    if (free_slot < 0) { return false; }
    table[free_slot].user = user;
    snprintf(table[free_slot].key, sizeof(table[free_slot].key), "%s", key);
    snprintf(table[free_slot].value, sizeof(table[free_slot].value), "%s", value);
    return true;
}

SteamAPICall_t SteamAPI_ISteamMatchmaking_CreateLobby(ISteamMatchmaking *self, int lobby_type, int max_members) { return 20; }
SteamAPICall_t SteamAPI_ISteamMatchmaking_JoinLobby(ISteamMatchmaking *self, uint64_steamid lobby) { return 21; }
void           SteamAPI_ISteamMatchmaking_LeaveLobby(ISteamMatchmaking *self, uint64_steamid lobby) { }
bool           SteamAPI_ISteamMatchmaking_InviteUserToLobby(ISteamMatchmaking *self, uint64_steamid lobby, uint64_steamid invitee) { return true; }
int            SteamAPI_ISteamMatchmaking_GetNumLobbyMembers(ISteamMatchmaking *self, uint64_steamid lobby) { return lobby == MOCK_LOBBY ? 2 : 0; }
uint64_steamid SteamAPI_ISteamMatchmaking_GetLobbyMemberByIndex(ISteamMatchmaking *self, uint64_steamid lobby, int member) {
    if (lobby != MOCK_LOBBY) { return 0; }
    return member == 0 ? MOCK_ME : member == 1 ? MOCK_FRIEND_IN_GAME : 0;
}
const char    *SteamAPI_ISteamMatchmaking_GetLobbyData(ISteamMatchmaking *self, uint64_steamid lobby, const char *key) {
    return mock_kv_get(mock_lobby_data, lobby, key);
}
bool           SteamAPI_ISteamMatchmaking_SetLobbyData(ISteamMatchmaking *self, uint64_steamid lobby, const char *key, const char *value) {
    return mock_kv_set(mock_lobby_data, lobby, key, value);
}
const char    *SteamAPI_ISteamMatchmaking_GetLobbyMemberData(ISteamMatchmaking *self, uint64_steamid lobby, uint64_steamid user, const char *key) {
    return mock_kv_get(mock_member_data, user, key);
}
void           SteamAPI_ISteamMatchmaking_SetLobbyMemberData(ISteamMatchmaking *self, uint64_steamid lobby, const char *key, const char *value) {
    mock_kv_set(mock_member_data, MOCK_ME, key, value);
}
bool           SteamAPI_ISteamMatchmaking_SendLobbyChatMsg(ISteamMatchmaking *self, uint64_steamid lobby, const void *body, int size) { return size > 0; }
int            SteamAPI_ISteamMatchmaking_GetLobbyChatEntry(ISteamMatchmaking *self, uint64_steamid lobby, int chat_id, uint64_steamid *user, void *data, int size, int *entry_type) {
    int slot = chat_id % MOCK_CHAT_SLOTS;
    int n = mock_chat[slot].len < size ? mock_chat[slot].len : size;
    if (user)       { *user = mock_chat[slot].user; }
    if (entry_type) { *entry_type = 1; }
    if (data && n > 0) { memcpy(data, mock_chat[slot].data, (size_t)n); }
    return n;
}
bool           SteamAPI_ISteamMatchmaking_SetLobbyJoinable(ISteamMatchmaking *self, uint64_steamid lobby, bool joinable) { return lobby == MOCK_LOBBY; }
bool           SteamAPI_ISteamMatchmaking_SetLobbyType(ISteamMatchmaking *self, uint64_steamid lobby, int lobby_type) { return lobby == MOCK_LOBBY; }
uint64_steamid SteamAPI_ISteamMatchmaking_GetLobbyOwner(ISteamMatchmaking *self, uint64_steamid lobby) { return lobby == MOCK_LOBBY ? MOCK_ME : 0; }
bool           SteamAPI_ISteamMatchmaking_SetLobbyOwner(ISteamMatchmaking *self, uint64_steamid lobby, uint64_steamid new_owner) { return lobby == MOCK_LOBBY; }
void           SteamAPI_ISteamFriends_ActivateGameOverlayInviteDialog(ISteamFriends *self, uint64_steamid lobby) { }

/* ISteamTimeline — accessor + no-op annotations. Async "does...exist" calls
 * return deterministic fake handles (10 = event, 11 = game phase). */

void SteamAPI_ISteamTimeline_SetTimelineGameMode(ISteamTimeline *self, ETimelineGameMode mode) { }
void SteamAPI_ISteamTimeline_SetTimelineTooltip(ISteamTimeline *self, const char *description, float time_delta) { }
void SteamAPI_ISteamTimeline_ClearTimelineTooltip(ISteamTimeline *self, float time_delta) { }
TimelineEventHandle_t SteamAPI_ISteamTimeline_AddInstantaneousTimelineEvent(ISteamTimeline *self, const char *title, const char *description, const char *icon, uint32 icon_priority, float start_offset_seconds, ETimelineEventClipPriority possible_clip) { return 100; }
TimelineEventHandle_t SteamAPI_ISteamTimeline_AddRangeTimelineEvent(ISteamTimeline *self, const char *title, const char *description, const char *icon, uint32 icon_priority, float start_offset_seconds, float duration, ETimelineEventClipPriority possible_clip) { return 101; }
TimelineEventHandle_t SteamAPI_ISteamTimeline_StartRangeTimelineEvent(ISteamTimeline *self, const char *title, const char *description, const char *icon, uint32 icon_priority, float start_offset_seconds, ETimelineEventClipPriority possible_clip) { return 102; }
void SteamAPI_ISteamTimeline_UpdateRangeTimelineEvent(ISteamTimeline *self, TimelineEventHandle_t event, const char *title, const char *description, const char *icon, uint32 icon_priority, ETimelineEventClipPriority possible_clip) { }
void SteamAPI_ISteamTimeline_EndRangeTimelineEvent(ISteamTimeline *self, TimelineEventHandle_t event, float end_offset_seconds) { }
void SteamAPI_ISteamTimeline_RemoveTimelineEvent(ISteamTimeline *self, TimelineEventHandle_t event) { }
SteamAPICall_t SteamAPI_ISteamTimeline_DoesEventRecordingExist(ISteamTimeline *self, TimelineEventHandle_t event) { return 10; }
void SteamAPI_ISteamTimeline_StartGamePhase(ISteamTimeline *self) { }
void SteamAPI_ISteamTimeline_EndGamePhase(ISteamTimeline *self) { }
void SteamAPI_ISteamTimeline_SetGamePhaseID(ISteamTimeline *self, const char *phase_id) { }
SteamAPICall_t SteamAPI_ISteamTimeline_DoesGamePhaseRecordingExist(ISteamTimeline *self, const char *phase_id) { return 11; }
void SteamAPI_ISteamTimeline_AddGamePhaseTag(ISteamTimeline *self, const char *tag_name, const char *tag_icon, const char *tag_group, uint32 priority) { }
void SteamAPI_ISteamTimeline_SetGamePhaseAttribute(ISteamTimeline *self, const char *attribute_group, const char *attribute_value, uint32 priority) { }
void SteamAPI_ISteamTimeline_OpenOverlayToGamePhase(ISteamTimeline *self, const char *phase_id) { }
void SteamAPI_ISteamTimeline_OpenOverlayToTimelineEvent(ISteamTimeline *self, TimelineEventHandle_t event) { }

/* ISteamUGC — deterministic Workshop stubs. Subscribe/Unsubscribe return fake
 * handles (20/21). Two subscribed items; item 123456 is installed. */
SteamAPICall_t SteamAPI_ISteamUGC_SubscribeItem(ISteamUGC *self, PublishedFileId_t file_id) { return 20; }
SteamAPICall_t SteamAPI_ISteamUGC_UnsubscribeItem(ISteamUGC *self, PublishedFileId_t file_id) { return 21; }
uint32 SteamAPI_ISteamUGC_GetNumSubscribedItems(ISteamUGC *self, bool include_locally_disabled) { return 2; }
uint32 SteamAPI_ISteamUGC_GetSubscribedItems(ISteamUGC *self, PublishedFileId_t *file_ids, uint32 max_entries, bool include_locally_disabled) {
    uint32 n = 0;
    if (file_ids && max_entries > 0) { file_ids[n++] = 123456; }
    if (file_ids && max_entries > 1) { file_ids[n++] = 234567; }
    return n;
}
uint32 SteamAPI_ISteamUGC_GetItemState(ISteamUGC *self, PublishedFileId_t file_id) { return 1 | 4; } /* subscribed | installed */
bool SteamAPI_ISteamUGC_GetItemInstallInfo(ISteamUGC *self, PublishedFileId_t file_id, uint64_t *size_on_disk, char *folder, uint32 folder_size, uint32 *timestamp) {
    if (size_on_disk) { *size_on_disk = 4096; }
    if (timestamp)    { *timestamp = 1609459200; }
    if (folder && folder_size > 0) {
        const char *p = "/mock/workshop/123456";
        uint32 i = 0;
        for (; p[i] && i < folder_size - 1; i++) { folder[i] = p[i]; }
        folder[i] = '\0';
    }
    return true;
}
bool SteamAPI_ISteamUGC_GetItemDownloadInfo(ISteamUGC *self, PublishedFileId_t file_id, uint64_t *bytes_downloaded, uint64_t *bytes_total) {
    if (bytes_downloaded) { *bytes_downloaded = 4096; }
    if (bytes_total)      { *bytes_total = 4096; }
    return true;
}
bool SteamAPI_ISteamUGC_DownloadItem(ISteamUGC *self, PublishedFileId_t file_id, bool high_priority) { return true; }
SteamAPICall_t SteamAPI_ISteamUGC_CreateItem(ISteamUGC *self, AppId_t consumer_app_id, int file_type) { return 22; }
UGCUpdateHandle_t SteamAPI_ISteamUGC_StartItemUpdate(ISteamUGC *self, AppId_t consumer_app_id, PublishedFileId_t file_id) { return 100; }
bool SteamAPI_ISteamUGC_SetItemTitle(ISteamUGC *self, UGCUpdateHandle_t handle, const char *title) { return true; }
bool SteamAPI_ISteamUGC_SetItemDescription(ISteamUGC *self, UGCUpdateHandle_t handle, const char *description) { return true; }
bool SteamAPI_ISteamUGC_SetItemVisibility(ISteamUGC *self, UGCUpdateHandle_t handle, int visibility) { return true; }
bool SteamAPI_ISteamUGC_SetItemContent(ISteamUGC *self, UGCUpdateHandle_t handle, const char *content_folder) { return true; }
bool SteamAPI_ISteamUGC_SetItemPreview(ISteamUGC *self, UGCUpdateHandle_t handle, const char *preview_file) { return true; }
bool SteamAPI_ISteamUGC_SetItemTags(ISteamUGC *self, UGCUpdateHandle_t handle, const SteamParamStringArray_t *tags, bool allow_admin_tags) { return true; }
SteamAPICall_t SteamAPI_ISteamUGC_SubmitItemUpdate(ISteamUGC *self, UGCUpdateHandle_t handle, const char *change_note) { return 23; }
int SteamAPI_ISteamUGC_GetItemUpdateProgress(ISteamUGC *self, UGCUpdateHandle_t handle, uint64_t *bytes_processed, uint64_t *bytes_total) {
    if (bytes_processed) *bytes_processed = 0;
    if (bytes_total) *bytes_total = 0;
    return 0; /* k_EItemUpdateStatusInvalid */
}
SteamAPICall_t SteamAPI_ISteamUGC_DeleteItem(ISteamUGC *self, PublishedFileId_t file_id) { return 24; }

/* ISteamNetworkingSockets / Utils — deterministic stubs. Fake handles; no
 * messages delivered (round-trip P2P needs a real relay + peer). */
HSteamListenSocket  SteamAPI_ISteamNetworkingSockets_CreateListenSocketP2P(ISteamNetworkingSockets *self, int local_virtual_port, int n_options, const void *options) { return 30; }
HSteamNetConnection SteamAPI_ISteamNetworkingSockets_ConnectP2P(ISteamNetworkingSockets *self, const void *identity_remote, int remote_virtual_port, int n_options, const void *options) { return 31; }
int  SteamAPI_ISteamNetworkingSockets_AcceptConnection(ISteamNetworkingSockets *self, HSteamNetConnection conn) { return 1; } /* k_EResultOK */
bool SteamAPI_ISteamNetworkingSockets_CloseConnection(ISteamNetworkingSockets *self, HSteamNetConnection peer, int reason, const char *debug, bool enable_linger) { return true; }
int  SteamAPI_ISteamNetworkingSockets_SendMessageToConnection(ISteamNetworkingSockets *self, HSteamNetConnection conn, const void *data, uint32 cb_data, int send_flags, int64_t *out_message_number) { if (out_message_number) { *out_message_number = 1; } return 1; }
int  SteamAPI_ISteamNetworkingSockets_ReceiveMessagesOnConnection(ISteamNetworkingSockets *self, HSteamNetConnection conn, void **out_messages, int max_messages) { return 0; }
void SteamAPI_ISteamNetworkingUtils_InitRelayNetworkAccess(ISteamNetworkingUtils *self) { }
/* SteamNetworkingIdentity: m_eType (int32) at 0, m_cbSize at 4, m_steamID64 at 8. */
void SteamAPI_SteamNetworkingIdentity_SetSteamID64(void *identity, uint64_steamid steam_id) {
    if (identity) {
        int32 type = 16; /* k_ESteamNetworkingIdentityType_SteamID */
        memcpy(identity, &type, sizeof(type));
        memcpy((unsigned char *)identity + 8, &steam_id, sizeof(steam_id));
    }
}
uint64_steamid SteamAPI_SteamNetworkingIdentity_GetSteamID64(void *identity) {
    uint64_steamid id = 0;
    if (identity) { memcpy(&id, (unsigned char *)identity + 8, sizeof(id)); }
    return id;
}

/* Phase 4d. Poll group 40 holds connection 31, which has one message waiting:
   "hello" from friend ...729, reliable, message number 7. The message is laid
   out like SteamNetworkingMessage_t (same offsets under both packings). */
static unsigned char mock_net_message[256];
static char mock_net_payload[] = "hello";
bool SteamAPI_ISteamNetworkingSockets_CloseListenSocket(ISteamNetworkingSockets *self, HSteamListenSocket socket) { return socket == 30; }
HSteamNetPollGroup SteamAPI_ISteamNetworkingSockets_CreatePollGroup(ISteamNetworkingSockets *self) { return 40; }
bool SteamAPI_ISteamNetworkingSockets_DestroyPollGroup(ISteamNetworkingSockets *self, HSteamNetPollGroup group) { return group == 40; }
bool SteamAPI_ISteamNetworkingSockets_SetConnectionPollGroup(ISteamNetworkingSockets *self, HSteamNetConnection conn, HSteamNetPollGroup group) { return true; }
int  SteamAPI_ISteamNetworkingSockets_ReceiveMessagesOnPollGroup(ISteamNetworkingSockets *self, HSteamNetPollGroup group, void **out_messages, int max_messages) {
    if (group != 40 || max_messages < 1 || !out_messages) { return 0; }
    void    *data   = mock_net_payload;
    int      size   = 5;
    uint32   conn   = 31;
    int64_t  number = 7;
    int      flags  = 8; /* k_nSteamNetworkingSend_Reliable */
    memset(mock_net_message, 0, sizeof(mock_net_message));
    memcpy(mock_net_message + 0,   &data,   sizeof(data));
    memcpy(mock_net_message + 8,   &size,   sizeof(size));
    memcpy(mock_net_message + 12,  &conn,   sizeof(conn));
    SteamAPI_SteamNetworkingIdentity_SetSteamID64(mock_net_message + 16, MOCK_FRIEND_IN_GAME);
    memcpy(mock_net_message + 168, &number, sizeof(number));
    memcpy(mock_net_message + 196, &flags,  sizeof(flags));
    out_messages[0] = mock_net_message;
    return 1;
}
int  SteamAPI_ISteamNetworkingSockets_GetConnectionRealTimeStatus(ISteamNetworkingSockets *self, HSteamNetConnection conn, void *status, int lanes, void *lane_status) {
    if (conn != 31 || !status) { return 3; /* k_EResultNoConnection */ }
    /* SteamNetConnectionRealTimeStatus_t offsets, measured against SDK 1.64. */
    unsigned char *s = status;
    int32 state = 3, ping = 42, send_rate = 256000, pending_unrel = 0, pending_rel = 64, unacked = 0;
    float q_local = 0.95f, q_remote = 0.9f, out_pps = 10.0f, out_bps = 1200.0f, in_pps = 8.0f, in_bps = 800.0f;
    int64_t queue = 1500;
    memset(s, 0, 120);
    memcpy(s + 0,  &state, 4);     memcpy(s + 4,  &ping, 4);
    memcpy(s + 8,  &q_local, 4);   memcpy(s + 12, &q_remote, 4);
    memcpy(s + 16, &out_pps, 4);   memcpy(s + 20, &out_bps, 4);
    memcpy(s + 24, &in_pps, 4);    memcpy(s + 28, &in_bps, 4);
    memcpy(s + 32, &send_rate, 4); memcpy(s + 36, &pending_unrel, 4);
    memcpy(s + 40, &pending_rel, 4); memcpy(s + 44, &unacked, 4);
    memcpy(s + 48, &queue, 8);
    return 1; /* k_EResultOK */
}

/* SteamNetConnectionStatusChangedCallback_t as Steam lays it out on this
   platform (measured against SDK 1.64): m_info starts at 8 under pack(8)
   (Windows) and at 4 under pack(4) (Linux/macOS). Deliberately NOT taken from
   src/steam_api_c.h — the test must catch the extension reading wrong offsets. */
#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#define MOCK_NETCB_INFO 4
#define MOCK_NETCB_OLD  700
#define MOCK_NETCB_SIZE 704
#else
#define MOCK_NETCB_INFO 8
#define MOCK_NETCB_OLD  704
#define MOCK_NETCB_SIZE 712
#endif
void SteamMock_FireNetConnectionStatus(uint32 conn, uint64_t peer, uint32 listen_socket, int32 state, int32 old_state) {
    unsigned char cb[MOCK_NETCB_SIZE];
    memset(cb, 0, sizeof(cb));
    memcpy(cb + 0, &conn, sizeof(conn));
    SteamAPI_SteamNetworkingIdentity_SetSteamID64(cb + MOCK_NETCB_INFO, peer);   /* m_info.m_identityRemote */
    memcpy(cb + MOCK_NETCB_INFO + 144, &listen_socket, sizeof(listen_socket));  /* m_info.m_hListenSocket */
    memcpy(cb + MOCK_NETCB_INFO + 176, &state, sizeof(state));                  /* m_info.m_eState */
    memcpy(cb + MOCK_NETCB_OLD, &old_state, sizeof(old_state));                 /* m_eOldState */
    mock_dispatch(k_iCallback_SteamNetConnectionStatusChanged, cb);
}
void SteamAPI_SteamNetworkingMessage_t_Release(void *message) { }
