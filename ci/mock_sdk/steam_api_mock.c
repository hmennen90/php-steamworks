/*
 * Mock implementation of the Steamworks flat API for CI testing.
 * All functions return safe default/failure values.
 */

#include "public/steam/steam_api_flat.h"

static int mock_instance = 0;

/* Core */
ESteamAPIInitResult SteamAPI_InitFlat(SteamErrMsg *pOutErrMsg) {
    if (pOutErrMsg) { (*pOutErrMsg)[0] = '\0'; }
    return k_ESteamAPIInitResult_NoSteamClient;
}
void SteamAPI_Shutdown(void) { }
void SteamAPI_RunCallbacks(void) { }
void SteamAPI_RegisterCallback(void *callback, int iCallback) { }
void SteamAPI_UnregisterCallback(void *callback) { }

/* Accessors — return a non-NULL pointer so functions can proceed */
ISteamUser*          SteamAPI_SteamUser_v023(void)          { return (ISteamUser*)&mock_instance; }
ISteamFriends*       SteamAPI_SteamFriends_v018(void)       { return (ISteamFriends*)&mock_instance; }
ISteamUserStats*     SteamAPI_SteamUserStats_v013(void)     { return (ISteamUserStats*)&mock_instance; }
ISteamRemoteStorage* SteamAPI_SteamRemoteStorage_v016(void) { return (ISteamRemoteStorage*)&mock_instance; }
ISteamApps*          SteamAPI_SteamApps_v009(void)          { return (ISteamApps*)&mock_instance; }
ISteamUtils*         SteamAPI_SteamUtils_v010(void)         { return (ISteamUtils*)&mock_instance; }
ISteamUGC*           SteamAPI_SteamUGC_v021(void)           { return (ISteamUGC*)&mock_instance; }
ISteamNetworkingSockets* SteamAPI_SteamNetworkingSockets_SteamAPI_v012(void) { return (ISteamNetworkingSockets*)&mock_instance; }
ISteamNetworkingUtils*   SteamAPI_SteamNetworkingUtils_SteamAPI_v004(void)   { return (ISteamNetworkingUtils*)&mock_instance; }

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
        entry->m_hUGC        = 0;
    }
    if (details) {
        for (int32 i = 0; i < n; i++) {
            details[i] = (index + 1) * 100 + i;
        }
    }
    return true;
}
int SteamAPI_ISteamUserStats_GetLeaderboardEntryCount(ISteamUserStats *self, SteamLeaderboard_t leaderboard) { return 5; }

/* ISteamRemoteStorage */
bool        SteamAPI_ISteamRemoteStorage_FileWrite(ISteamRemoteStorage *self, const char *file, const void *data, int32 size) { return false; }
int32       SteamAPI_ISteamRemoteStorage_GetFileSize(ISteamRemoteStorage *self, const char *file) { return 0; }
int32       SteamAPI_ISteamRemoteStorage_FileRead(ISteamRemoteStorage *self, const char *file, void *data, int32 size) { return 0; }
bool        SteamAPI_ISteamRemoteStorage_FileExists(ISteamRemoteStorage *self, const char *file) { return false; }
bool        SteamAPI_ISteamRemoteStorage_FileDelete(ISteamRemoteStorage *self, const char *file) { return false; }
int32       SteamAPI_ISteamRemoteStorage_GetFileCount(ISteamRemoteStorage *self) { return 0; }
const char* SteamAPI_ISteamRemoteStorage_GetFileNameAndSize(ISteamRemoteStorage *self, int32 index, int32 *size) { *size = 0; return ""; }

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

/* ISteamUtils */
AppId_t     SteamAPI_ISteamUtils_GetAppID(ISteamUtils *self) { return 480; }
bool        SteamAPI_ISteamUtils_IsOverlayEnabled(ISteamUtils *self) { return false; }
const char* SteamAPI_ISteamUtils_GetIPCountry(ISteamUtils *self) { return "US"; }
bool        SteamAPI_ISteamUtils_IsSteamRunningOnSteamDeck(ISteamUtils *self) { return false; }
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
    return false;
}

/* ISteamTimeline — accessor + no-op annotations. Async "does...exist" calls
 * return deterministic fake handles (10 = event, 11 = game phase). */
ISteamTimeline* SteamAPI_SteamTimeline_v004(void) { return (ISteamTimeline*)&mock_instance; }

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
void SteamAPI_SteamNetworkingIdentity_SetSteamID64(void *identity, uint64_steamid steam_id) { }
uint64_steamid SteamAPI_SteamNetworkingIdentity_GetSteamID64(void *identity) { return 0; }
void SteamAPI_SteamNetworkingMessage_t_Release(void *message) { }
