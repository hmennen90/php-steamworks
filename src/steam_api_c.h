/*
 * steam_api_c.h — C-compatible declarations for the Steamworks flat API.
 *
 * The official steam_api_flat.h includes C++ headers, making it unusable
 * from plain C. This header re-declares only the flat API symbols that
 * php-steamworks actually uses, using opaque pointers for interface handles.
 *
 * All symbols here are exported by libsteam_api as extern "C" in the SDK.
 */

#ifndef STEAM_API_C_H
#define STEAM_API_C_H

#include <stdint.h>
#include <stdbool.h>

/* Opaque interface handles — C++ classes on the SDK side */
typedef void ISteamUser;
typedef void ISteamFriends;
typedef void ISteamUserStats;
typedef void ISteamRemoteStorage;
typedef void ISteamApps;
typedef void ISteamUtils;

/* SDK typedefs */
typedef uint32_t AppId_t;
typedef int32_t  int32;
typedef uint32_t uint32;
typedef uint64_t uint64_steamid;

/* Async / leaderboard handle types (all opaque uint64 on the SDK side) */
typedef uint64_t SteamAPICall_t;
typedef uint64_t SteamLeaderboard_t;
typedef uint64_t SteamLeaderboardEntries_t;
typedef uint64_t CSteamID_t;
typedef uint64_t UGCHandle_t;

/* Leaderboard enums (values match the SDK exactly) */
typedef enum {
    k_ELeaderboardSortMethodNone       = 0,
    k_ELeaderboardSortMethodAscending  = 1,
    k_ELeaderboardSortMethodDescending = 2,
} ELeaderboardSortMethod;

typedef enum {
    k_ELeaderboardDisplayTypeNone            = 0,
    k_ELeaderboardDisplayTypeNumeric         = 1,
    k_ELeaderboardDisplayTypeTimeSeconds     = 2,
    k_ELeaderboardDisplayTypeTimeMilliSeconds = 3,
} ELeaderboardDisplayType;

typedef enum {
    k_ELeaderboardUploadScoreMethodNone        = 0,
    k_ELeaderboardUploadScoreMethodKeepBest    = 1,
    k_ELeaderboardUploadScoreMethodForceUpdate = 2,
} ELeaderboardUploadScoreMethod;

typedef enum {
    k_ELeaderboardDataRequestGlobal           = 0,
    k_ELeaderboardDataRequestGlobalAroundUser = 1,
    k_ELeaderboardDataRequestFriends          = 2,
    k_ELeaderboardDataRequestUsers            = 3,
} ELeaderboardDataRequest;

/* Callback IDs — needed as iCallbackExpected in GetAPICallResult.
   k_iSteamUserStatsCallbacks base is 1100 in the SDK. */
enum {
    k_iCallback_LeaderboardFindResult       = 1100 + 4, /* 1104 */
    k_iCallback_LeaderboardScoresDownloaded = 1100 + 5, /* 1105 */
    k_iCallback_LeaderboardScoreUploaded    = 1100 + 6, /* 1106 */
};

/* CallResult structs — layout must match the SDK's #pragma pack(8) callbacks. */
#pragma pack(push, 8)
typedef struct {
    SteamLeaderboard_t m_hSteamLeaderboard;
    uint8_t            m_bLeaderboardFound;
} LeaderboardFindResult_t;

typedef struct {
    uint8_t            m_bSuccess;
    SteamLeaderboard_t m_hSteamLeaderboard;
    int32              m_nScore;
    uint8_t            m_bScoreChanged;
    int                m_nGlobalRankNew;
    int                m_nGlobalRankPrevious;
} LeaderboardScoreUploaded_t;

typedef struct {
    SteamLeaderboard_t        m_hSteamLeaderboard;
    SteamLeaderboardEntries_t m_hSteamLeaderboardEntries;
    int                       m_cEntryCount;
} LeaderboardScoresDownloaded_t;

typedef struct {
    CSteamID_t  m_steamIDUser;
    int32       m_nGlobalRank;
    int32       m_nScore;
    int32       m_cDetails;
    UGCHandle_t m_hUGC;
} LeaderboardEntry_t;
#pragma pack(pop)

/* Steam error message buffer (1024 bytes as per SDK) */
typedef char SteamErrMsg[1024];

/* Init result enum (matches ESteamAPIInitResult in SDK) */
typedef enum {
    k_ESteamAPIInitResult_OK = 0,
    k_ESteamAPIInitResult_FailedGeneric = 1,
    k_ESteamAPIInitResult_NoSteamClient = 2,
    k_ESteamAPIInitResult_VersionMismatch = 3,
} ESteamAPIInitResult;

/* ── Core ──────────────────────────────────────────────────────────── */
ESteamAPIInitResult SteamAPI_InitFlat(SteamErrMsg *pOutErrMsg);
void SteamAPI_Shutdown(void);
void SteamAPI_RunCallbacks(void);

/* ── Accessor functions (return interface pointers) ────────────────── */
ISteamUser         *SteamAPI_SteamUser_v023(void);
ISteamFriends      *SteamAPI_SteamFriends_v018(void);
ISteamUserStats    *SteamAPI_SteamUserStats_v013(void);
ISteamRemoteStorage*SteamAPI_SteamRemoteStorage_v016(void);
ISteamApps         *SteamAPI_SteamApps_v009(void);
ISteamUtils        *SteamAPI_SteamUtils_v010(void);

/* ── ISteamUser ────────────────────────────────────────────────────── */
uint64_steamid SteamAPI_ISteamUser_GetSteamID(ISteamUser *self);

/* ── ISteamFriends ─────────────────────────────────────────────────── */
const char *SteamAPI_ISteamFriends_GetPersonaName(ISteamFriends *self);
bool        SteamAPI_ISteamFriends_SetRichPresence(ISteamFriends *self, const char *key, const char *value);
void        SteamAPI_ISteamFriends_ActivateGameOverlay(ISteamFriends *self, const char *dialog);
void        SteamAPI_ISteamFriends_ActivateGameOverlayToWebPage(ISteamFriends *self, const char *url, int eMode);

/* ── ISteamUserStats ───────────────────────────────────────────────── */
bool SteamAPI_ISteamUserStats_SetAchievement(ISteamUserStats *self, const char *name);
bool SteamAPI_ISteamUserStats_ClearAchievement(ISteamUserStats *self, const char *name);
bool SteamAPI_ISteamUserStats_StoreStats(ISteamUserStats *self);
bool SteamAPI_ISteamUserStats_GetStatInt32(ISteamUserStats *self, const char *name, int32 *data);
bool SteamAPI_ISteamUserStats_SetStatInt32(ISteamUserStats *self, const char *name, int32 data);
bool SteamAPI_ISteamUserStats_GetStatFloat(ISteamUserStats *self, const char *name, float *data);
bool SteamAPI_ISteamUserStats_SetStatFloat(ISteamUserStats *self, const char *name, float data);
bool SteamAPI_ISteamUserStats_IndicateAchievementProgress(ISteamUserStats *self, const char *name, uint32 cur_progress, uint32 max_progress);

/* ── ISteamUserStats: Achievement read path ────────────────────────────── */
bool        SteamAPI_ISteamUserStats_RequestCurrentStats(ISteamUserStats *self);
bool        SteamAPI_ISteamUserStats_GetAchievement(ISteamUserStats *self, const char *name, bool *achieved);
bool        SteamAPI_ISteamUserStats_GetAchievementAndUnlockTime(ISteamUserStats *self, const char *name, bool *achieved, uint32 *unlock_time);
uint32      SteamAPI_ISteamUserStats_GetNumAchievements(ISteamUserStats *self);
const char *SteamAPI_ISteamUserStats_GetAchievementName(ISteamUserStats *self, uint32 achievement);
const char *SteamAPI_ISteamUserStats_GetAchievementDisplayAttribute(ISteamUserStats *self, const char *name, const char *key);
bool        SteamAPI_ISteamUserStats_ResetAllStats(ISteamUserStats *self, bool achievements_too);

/* ── ISteamUserStats: Leaderboards (async via SteamAPICall_t) ──────────── */
SteamAPICall_t SteamAPI_ISteamUserStats_FindLeaderboard(ISteamUserStats *self, const char *name);
SteamAPICall_t SteamAPI_ISteamUserStats_FindOrCreateLeaderboard(ISteamUserStats *self, const char *name, ELeaderboardSortMethod sort, ELeaderboardDisplayType display);
SteamAPICall_t SteamAPI_ISteamUserStats_UploadLeaderboardScore(ISteamUserStats *self, SteamLeaderboard_t leaderboard, ELeaderboardUploadScoreMethod method, int32 score, const int32 *details, int details_count);
SteamAPICall_t SteamAPI_ISteamUserStats_DownloadLeaderboardEntries(ISteamUserStats *self, SteamLeaderboard_t leaderboard, ELeaderboardDataRequest request, int range_start, int range_end);
bool           SteamAPI_ISteamUserStats_GetDownloadedLeaderboardEntry(ISteamUserStats *self, SteamLeaderboardEntries_t entries, int index, LeaderboardEntry_t *entry, int32 *details, int details_max);
int            SteamAPI_ISteamUserStats_GetLeaderboardEntryCount(ISteamUserStats *self, SteamLeaderboard_t leaderboard);

/* ── ISteamRemoteStorage ───────────────────────────────────────────── */
bool        SteamAPI_ISteamRemoteStorage_FileWrite(ISteamRemoteStorage *self, const char *file, const void *data, int32 len);
int32       SteamAPI_ISteamRemoteStorage_FileRead(ISteamRemoteStorage *self, const char *file, void *data, int32 len);
int32       SteamAPI_ISteamRemoteStorage_GetFileSize(ISteamRemoteStorage *self, const char *file);
bool        SteamAPI_ISteamRemoteStorage_FileExists(ISteamRemoteStorage *self, const char *file);
bool        SteamAPI_ISteamRemoteStorage_FileDelete(ISteamRemoteStorage *self, const char *file);
int32       SteamAPI_ISteamRemoteStorage_GetFileCount(ISteamRemoteStorage *self);
const char *SteamAPI_ISteamRemoteStorage_GetFileNameAndSize(ISteamRemoteStorage *self, int32 file, int32 *size);

/* ── ISteamApps ────────────────────────────────────────────────────── */
bool        SteamAPI_ISteamApps_BIsSubscribed(ISteamApps *self);
bool        SteamAPI_ISteamApps_BIsDlcInstalled(ISteamApps *self, AppId_t dlcid);
uint64_steamid SteamAPI_ISteamApps_GetAppOwner(ISteamApps *self);
const char *SteamAPI_ISteamApps_GetCurrentGameLanguage(ISteamApps *self);

/* ── ISteamUtils ───────────────────────────────────────────────────── */
uint32      SteamAPI_ISteamUtils_GetAppID(ISteamUtils *self);
bool        SteamAPI_ISteamUtils_IsOverlayEnabled(ISteamUtils *self);
const char *SteamAPI_ISteamUtils_GetIPCountry(ISteamUtils *self);
bool        SteamAPI_ISteamUtils_IsSteamRunningOnSteamDeck(ISteamUtils *self);
bool        SteamAPI_ISteamUtils_IsAPICallCompleted(ISteamUtils *self, SteamAPICall_t call, bool *failed);
bool        SteamAPI_ISteamUtils_GetAPICallResult(ISteamUtils *self, SteamAPICall_t call, void *callback, int callback_size, int callback_expected, bool *failed);

#endif /* STEAM_API_C_H */
