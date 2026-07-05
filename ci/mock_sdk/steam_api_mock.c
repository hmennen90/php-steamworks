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

/* Accessors — return a non-NULL pointer so functions can proceed */
ISteamUser*          SteamAPI_SteamUser_v023(void)          { return (ISteamUser*)&mock_instance; }
ISteamFriends*       SteamAPI_SteamFriends_v018(void)       { return (ISteamFriends*)&mock_instance; }
ISteamUserStats*     SteamAPI_SteamUserStats_v013(void)     { return (ISteamUserStats*)&mock_instance; }
ISteamRemoteStorage* SteamAPI_SteamRemoteStorage_v016(void) { return (ISteamRemoteStorage*)&mock_instance; }
ISteamApps*          SteamAPI_SteamApps_v009(void)          { return (ISteamApps*)&mock_instance; }
ISteamUtils*         SteamAPI_SteamUtils_v010(void)         { return (ISteamUtils*)&mock_instance; }

/* ISteamUser */
uint64_steamid SteamAPI_ISteamUser_GetSteamID(ISteamUser *self) { return 0; }

/* ISteamFriends */
const char* SteamAPI_ISteamFriends_GetPersonaName(ISteamFriends *self) { return "MockPlayer"; }
bool        SteamAPI_ISteamFriends_SetRichPresence(ISteamFriends *self, const char *key, const char *value) { return false; }
void        SteamAPI_ISteamFriends_ActivateGameOverlay(ISteamFriends *self, const char *dialog) { }
void        SteamAPI_ISteamFriends_ActivateGameOverlayToWebPage(ISteamFriends *self, const char *url, int eMode) { }

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
bool SteamAPI_ISteamUserStats_RequestCurrentStats(ISteamUserStats *self) { return true; }
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
    if (entry) {
        entry->m_steamIDUser = 76561197960265728ULL;
        entry->m_nGlobalRank = index + 1;
        entry->m_nScore      = 1000 - index;
        entry->m_cDetails    = 0;
        entry->m_hUGC        = 0;
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

/* ISteamUtils */
AppId_t     SteamAPI_ISteamUtils_GetAppID(ISteamUtils *self) { return 480; }
bool        SteamAPI_ISteamUtils_IsOverlayEnabled(ISteamUtils *self) { return false; }
const char* SteamAPI_ISteamUtils_GetIPCountry(ISteamUtils *self) { return "US"; }
bool        SteamAPI_ISteamUtils_IsSteamRunningOnSteamDeck(ISteamUtils *self) { return false; }

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
    return false;
}
