#ifndef STEAM_API_FLAT_H
#define STEAM_API_FLAT_H

/*
 * Mock flat API for CI testing.
 * Provides function declarations matching the real Steamworks SDK.
 * Implementations are in ci/mock_sdk/steam_api_mock.c
 */

#include "steam_api.h"

/* Steam error message buffer */
typedef char SteamErrMsg[1024];

/* Leaderboard enums */
typedef enum {
    k_ELeaderboardSortMethodNone       = 0,
    k_ELeaderboardSortMethodAscending  = 1,
    k_ELeaderboardSortMethodDescending = 2,
} ELeaderboardSortMethod;

typedef enum {
    k_ELeaderboardDisplayTypeNone             = 0,
    k_ELeaderboardDisplayTypeNumeric          = 1,
    k_ELeaderboardDisplayTypeTimeSeconds      = 2,
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

/* ISteamTimeline enums (must match src/steam_api_c.h) */
typedef enum {
    k_ETimelineGameMode_Invalid       = 0,
    k_ETimelineGameMode_Playing       = 1,
    k_ETimelineGameMode_Staging       = 2,
    k_ETimelineGameMode_Menus         = 3,
    k_ETimelineGameMode_LoadingScreen = 4,
    k_ETimelineGameMode_Max           = 5,
} ETimelineGameMode;

typedef enum {
    k_ETimelineEventClipPriority_Invalid  = 0,
    k_ETimelineEventClipPriority_None     = 1,
    k_ETimelineEventClipPriority_Standard = 2,
    k_ETimelineEventClipPriority_Featured = 3,
} ETimelineEventClipPriority;

#define k_cGamePhaseIDStrMaxLen 64

/* Callback IDs (must match src/steam_api_c.h and the real SDK) */
enum {
    k_iCallback_LeaderboardFindResult       = 1100 + 4,
    k_iCallback_LeaderboardScoresDownloaded = 1100 + 5,
    k_iCallback_LeaderboardScoreUploaded    = 1100 + 6,
    k_iCallback_SteamTimelineGamePhaseRecordingExists = 6000 + 1,
    k_iCallback_SteamTimelineEventRecordingExists      = 6000 + 2,
    k_iCallback_RemoteStorageSubscribePublishedFileResult   = 1300 + 13,
    k_iCallback_RemoteStorageUnsubscribePublishedFileResult = 1300 + 15,
    k_iCallback_GetTicketForWebApiResponse                  = 100 + 68,
};

/* CallResult structs — layout must match src/steam_api_c.h and the real SDK.
 * Same conditional packing as the SDK: pack(4) on Linux/macOS/FreeBSD, else pack(8). */
#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#pragma pack(push, 4)
#else
#pragma pack(push, 8)
#endif
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

/* ISteamTimeline CallResult structs (must match src/steam_api_c.h) */
typedef struct {
    char     m_rgchPhaseID[k_cGamePhaseIDStrMaxLen];
    uint64_t m_ulRecordingMS;
    uint64_t m_ulLongestClipMS;
    uint32   m_unClipCount;
    uint32   m_unScreenshotCount;
} SteamTimelineGamePhaseRecordingExists_t;

typedef struct {
    TimelineEventHandle_t m_ulEventID;
    uint8_t               m_bRecordingExists;
} SteamTimelineEventRecordingExists_t;

typedef struct {
    int32             m_eResult;
    PublishedFileId_t m_nPublishedFileId;
} RemoteStorageSubscribePublishedFileResult_t;

typedef struct {
    int32             m_eResult;
    PublishedFileId_t m_nPublishedFileId;
} RemoteStorageUnsubscribePublishedFileResult_t;

typedef struct {
    HAuthTicket m_hAuthTicket;
    int32       m_eResult;
    int32       m_cubTicket;
    uint8_t     m_rgubTicket[2560];
} GetTicketForWebApiResponse_t;
#pragma pack(pop)

typedef enum {
    k_ESteamAPIInitResult_OK = 0,
    k_ESteamAPIInitResult_FailedGeneric = 1,
    k_ESteamAPIInitResult_NoSteamClient = 2,
    k_ESteamAPIInitResult_VersionMismatch = 3,
} ESteamAPIInitResult;

/* Core */
ESteamAPIInitResult SteamAPI_InitFlat(SteamErrMsg *pOutErrMsg);
void SteamAPI_Shutdown(void);
void SteamAPI_RunCallbacks(void);

/* Accessors */
ISteamUser*          SteamAPI_SteamUser_v023(void);
ISteamFriends*       SteamAPI_SteamFriends_v018(void);
ISteamUserStats*     SteamAPI_SteamUserStats_v013(void);
ISteamRemoteStorage* SteamAPI_SteamRemoteStorage_v016(void);
ISteamApps*          SteamAPI_SteamApps_v009(void);
ISteamUtils*         SteamAPI_SteamUtils_v010(void);
ISteamTimeline*      SteamAPI_SteamTimeline_v004(void);
ISteamUGC*           SteamAPI_SteamUGC_v021(void);

/* ISteamUser */
uint64_steamid SteamAPI_ISteamUser_GetSteamID(ISteamUser *self);
bool           SteamAPI_ISteamUser_BLoggedOn(ISteamUser *self);
int            SteamAPI_ISteamUser_GetPlayerSteamLevel(ISteamUser *self);
HAuthTicket    SteamAPI_ISteamUser_GetAuthSessionTicket(ISteamUser *self, void *ticket, int max_ticket, uint32 *ticket_size, const void *identity);
HAuthTicket    SteamAPI_ISteamUser_GetAuthTicketForWebApi(ISteamUser *self, const char *identity);
int            SteamAPI_ISteamUser_BeginAuthSession(ISteamUser *self, const void *auth_ticket, int ticket_size, uint64_steamid steam_id);
void           SteamAPI_ISteamUser_EndAuthSession(ISteamUser *self, uint64_steamid steam_id);
void           SteamAPI_ISteamUser_CancelAuthTicket(ISteamUser *self, HAuthTicket handle);
void           SteamAPI_RegisterCallback(void *callback, int iCallback);
void           SteamAPI_UnregisterCallback(void *callback);

/* ISteamFriends */
const char* SteamAPI_ISteamFriends_GetPersonaName(ISteamFriends *self);
bool        SteamAPI_ISteamFriends_SetRichPresence(ISteamFriends *self, const char *key, const char *value);
void        SteamAPI_ISteamFriends_ActivateGameOverlay(ISteamFriends *self, const char *dialog);
void        SteamAPI_ISteamFriends_ActivateGameOverlayToWebPage(ISteamFriends *self, const char *url, int eMode);
int         SteamAPI_ISteamFriends_GetPersonaState(ISteamFriends *self);
int         SteamAPI_ISteamFriends_GetFriendCount(ISteamFriends *self, int friend_flags);
uint64_steamid SteamAPI_ISteamFriends_GetFriendByIndex(ISteamFriends *self, int index, int friend_flags);
int         SteamAPI_ISteamFriends_GetFriendRelationship(ISteamFriends *self, uint64_steamid steam_id);
int         SteamAPI_ISteamFriends_GetFriendPersonaState(ISteamFriends *self, uint64_steamid steam_id);
const char* SteamAPI_ISteamFriends_GetFriendPersonaName(ISteamFriends *self, uint64_steamid steam_id);
int         SteamAPI_ISteamFriends_GetSmallFriendAvatar(ISteamFriends *self, uint64_steamid steam_id);
int         SteamAPI_ISteamFriends_GetMediumFriendAvatar(ISteamFriends *self, uint64_steamid steam_id);
int         SteamAPI_ISteamFriends_GetLargeFriendAvatar(ISteamFriends *self, uint64_steamid steam_id);
bool        SteamAPI_ISteamFriends_RequestUserInformation(ISteamFriends *self, uint64_steamid steam_id, bool require_name_only);

/* ISteamUserStats */
bool SteamAPI_ISteamUserStats_SetAchievement(ISteamUserStats *self, const char *name);
bool SteamAPI_ISteamUserStats_ClearAchievement(ISteamUserStats *self, const char *name);
bool SteamAPI_ISteamUserStats_StoreStats(ISteamUserStats *self);
bool SteamAPI_ISteamUserStats_GetStatInt32(ISteamUserStats *self, const char *name, int32 *data);
bool SteamAPI_ISteamUserStats_SetStatInt32(ISteamUserStats *self, const char *name, int32 data);
bool SteamAPI_ISteamUserStats_GetStatFloat(ISteamUserStats *self, const char *name, float *data);
bool SteamAPI_ISteamUserStats_SetStatFloat(ISteamUserStats *self, const char *name, float data);
bool SteamAPI_ISteamUserStats_IndicateAchievementProgress(ISteamUserStats *self, const char *name, uint32 cur_progress, uint32 max_progress);
bool        SteamAPI_ISteamUserStats_GetAchievement(ISteamUserStats *self, const char *name, bool *achieved);
bool        SteamAPI_ISteamUserStats_GetAchievementAndUnlockTime(ISteamUserStats *self, const char *name, bool *achieved, uint32 *unlock_time);
uint32      SteamAPI_ISteamUserStats_GetNumAchievements(ISteamUserStats *self);
const char* SteamAPI_ISteamUserStats_GetAchievementName(ISteamUserStats *self, uint32 achievement);
const char* SteamAPI_ISteamUserStats_GetAchievementDisplayAttribute(ISteamUserStats *self, const char *name, const char *key);
bool        SteamAPI_ISteamUserStats_ResetAllStats(ISteamUserStats *self, bool achievements_too);
SteamAPICall_t SteamAPI_ISteamUserStats_FindLeaderboard(ISteamUserStats *self, const char *name);
SteamAPICall_t SteamAPI_ISteamUserStats_FindOrCreateLeaderboard(ISteamUserStats *self, const char *name, ELeaderboardSortMethod sort, ELeaderboardDisplayType display);
SteamAPICall_t SteamAPI_ISteamUserStats_UploadLeaderboardScore(ISteamUserStats *self, SteamLeaderboard_t leaderboard, ELeaderboardUploadScoreMethod method, int32 score, const int32 *details, int details_count);
SteamAPICall_t SteamAPI_ISteamUserStats_DownloadLeaderboardEntries(ISteamUserStats *self, SteamLeaderboard_t leaderboard, ELeaderboardDataRequest request, int range_start, int range_end);
bool           SteamAPI_ISteamUserStats_GetDownloadedLeaderboardEntry(ISteamUserStats *self, SteamLeaderboardEntries_t entries, int index, LeaderboardEntry_t *entry, int32 *details, int details_max);
int            SteamAPI_ISteamUserStats_GetLeaderboardEntryCount(ISteamUserStats *self, SteamLeaderboard_t leaderboard);

/* ISteamRemoteStorage */
bool        SteamAPI_ISteamRemoteStorage_FileWrite(ISteamRemoteStorage *self, const char *file, const void *data, int32 size);
int32       SteamAPI_ISteamRemoteStorage_GetFileSize(ISteamRemoteStorage *self, const char *file);
int32       SteamAPI_ISteamRemoteStorage_FileRead(ISteamRemoteStorage *self, const char *file, void *data, int32 size);
bool        SteamAPI_ISteamRemoteStorage_FileExists(ISteamRemoteStorage *self, const char *file);
bool        SteamAPI_ISteamRemoteStorage_FileDelete(ISteamRemoteStorage *self, const char *file);
int32       SteamAPI_ISteamRemoteStorage_GetFileCount(ISteamRemoteStorage *self);
const char* SteamAPI_ISteamRemoteStorage_GetFileNameAndSize(ISteamRemoteStorage *self, int32 index, int32 *size);

/* ISteamApps */
bool        SteamAPI_ISteamApps_BIsSubscribed(ISteamApps *self);
bool        SteamAPI_ISteamApps_BIsDlcInstalled(ISteamApps *self, AppId_t dlc_id);
uint64_steamid SteamAPI_ISteamApps_GetAppOwner(ISteamApps *self);
const char* SteamAPI_ISteamApps_GetCurrentGameLanguage(ISteamApps *self);
bool        SteamAPI_ISteamApps_BIsSubscribedApp(ISteamApps *self, AppId_t appid);
bool        SteamAPI_ISteamApps_GetCurrentBetaName(ISteamApps *self, char *name, int name_buffer_size);
uint32      SteamAPI_ISteamApps_GetEarliestPurchaseUnixTime(ISteamApps *self, AppId_t appid);
uint32      SteamAPI_ISteamApps_GetInstalledDepots(ISteamApps *self, AppId_t appid, DepotId_t *depots, uint32 max_depots);
int         SteamAPI_ISteamApps_GetDLCCount(ISteamApps *self);
int         SteamAPI_ISteamApps_GetAppBuildId(ISteamApps *self);

/* ISteamUtils */
AppId_t     SteamAPI_ISteamUtils_GetAppID(ISteamUtils *self);
bool        SteamAPI_ISteamUtils_IsOverlayEnabled(ISteamUtils *self);
const char* SteamAPI_ISteamUtils_GetIPCountry(ISteamUtils *self);
bool        SteamAPI_ISteamUtils_IsSteamRunningOnSteamDeck(ISteamUtils *self);
const char* SteamAPI_ISteamUtils_GetSteamUILanguage(ISteamUtils *self);
uint32      SteamAPI_ISteamUtils_GetServerRealTime(ISteamUtils *self);
uint8_t     SteamAPI_ISteamUtils_GetCurrentBatteryPower(ISteamUtils *self);
uint32      SteamAPI_ISteamUtils_GetSecondsSinceAppActive(ISteamUtils *self);
bool        SteamAPI_ISteamUtils_IsAPICallCompleted(ISteamUtils *self, SteamAPICall_t call, bool *failed);
bool        SteamAPI_ISteamUtils_GetAPICallResult(ISteamUtils *self, SteamAPICall_t call, void *callback, int callback_size, int callback_expected, bool *failed);
bool        SteamAPI_ISteamUtils_GetImageSize(ISteamUtils *self, int image, uint32 *width, uint32 *height);
bool        SteamAPI_ISteamUtils_GetImageRGBA(ISteamUtils *self, int image, uint8_t *dest, int dest_size);

/* ISteamUGC */
SteamAPICall_t SteamAPI_ISteamUGC_SubscribeItem(ISteamUGC *self, PublishedFileId_t file_id);
SteamAPICall_t SteamAPI_ISteamUGC_UnsubscribeItem(ISteamUGC *self, PublishedFileId_t file_id);
uint32 SteamAPI_ISteamUGC_GetNumSubscribedItems(ISteamUGC *self, bool include_locally_disabled);
uint32 SteamAPI_ISteamUGC_GetSubscribedItems(ISteamUGC *self, PublishedFileId_t *file_ids, uint32 max_entries, bool include_locally_disabled);
uint32 SteamAPI_ISteamUGC_GetItemState(ISteamUGC *self, PublishedFileId_t file_id);
bool   SteamAPI_ISteamUGC_GetItemInstallInfo(ISteamUGC *self, PublishedFileId_t file_id, uint64_t *size_on_disk, char *folder, uint32 folder_size, uint32 *timestamp);
bool   SteamAPI_ISteamUGC_GetItemDownloadInfo(ISteamUGC *self, PublishedFileId_t file_id, uint64_t *bytes_downloaded, uint64_t *bytes_total);
bool   SteamAPI_ISteamUGC_DownloadItem(ISteamUGC *self, PublishedFileId_t file_id, bool high_priority);

/* ISteamTimeline */
void SteamAPI_ISteamTimeline_SetTimelineGameMode(ISteamTimeline *self, ETimelineGameMode mode);
void SteamAPI_ISteamTimeline_SetTimelineTooltip(ISteamTimeline *self, const char *description, float time_delta);
void SteamAPI_ISteamTimeline_ClearTimelineTooltip(ISteamTimeline *self, float time_delta);
TimelineEventHandle_t SteamAPI_ISteamTimeline_AddInstantaneousTimelineEvent(ISteamTimeline *self, const char *title, const char *description, const char *icon, uint32 icon_priority, float start_offset_seconds, ETimelineEventClipPriority possible_clip);
TimelineEventHandle_t SteamAPI_ISteamTimeline_AddRangeTimelineEvent(ISteamTimeline *self, const char *title, const char *description, const char *icon, uint32 icon_priority, float start_offset_seconds, float duration, ETimelineEventClipPriority possible_clip);
TimelineEventHandle_t SteamAPI_ISteamTimeline_StartRangeTimelineEvent(ISteamTimeline *self, const char *title, const char *description, const char *icon, uint32 icon_priority, float start_offset_seconds, ETimelineEventClipPriority possible_clip);
void SteamAPI_ISteamTimeline_UpdateRangeTimelineEvent(ISteamTimeline *self, TimelineEventHandle_t event, const char *title, const char *description, const char *icon, uint32 icon_priority, ETimelineEventClipPriority possible_clip);
void SteamAPI_ISteamTimeline_EndRangeTimelineEvent(ISteamTimeline *self, TimelineEventHandle_t event, float end_offset_seconds);
void SteamAPI_ISteamTimeline_RemoveTimelineEvent(ISteamTimeline *self, TimelineEventHandle_t event);
SteamAPICall_t SteamAPI_ISteamTimeline_DoesEventRecordingExist(ISteamTimeline *self, TimelineEventHandle_t event);
void SteamAPI_ISteamTimeline_StartGamePhase(ISteamTimeline *self);
void SteamAPI_ISteamTimeline_EndGamePhase(ISteamTimeline *self);
void SteamAPI_ISteamTimeline_SetGamePhaseID(ISteamTimeline *self, const char *phase_id);
SteamAPICall_t SteamAPI_ISteamTimeline_DoesGamePhaseRecordingExist(ISteamTimeline *self, const char *phase_id);
void SteamAPI_ISteamTimeline_AddGamePhaseTag(ISteamTimeline *self, const char *tag_name, const char *tag_icon, const char *tag_group, uint32 priority);
void SteamAPI_ISteamTimeline_SetGamePhaseAttribute(ISteamTimeline *self, const char *attribute_group, const char *attribute_value, uint32 priority);
void SteamAPI_ISteamTimeline_OpenOverlayToGamePhase(ISteamTimeline *self, const char *phase_id);
void SteamAPI_ISteamTimeline_OpenOverlayToTimelineEvent(ISteamTimeline *self, TimelineEventHandle_t event);

#endif /* STEAM_API_FLAT_H */
