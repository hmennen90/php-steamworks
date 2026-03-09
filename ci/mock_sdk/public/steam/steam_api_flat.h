#ifndef STEAM_API_FLAT_H
#define STEAM_API_FLAT_H

/*
 * Mock flat API for CI testing.
 * Provides function declarations matching the real Steamworks SDK.
 * Implementations are in ci/mock_sdk/steam_api_mock.c
 */

#include "steam_api.h"

/* Core */
bool SteamAPI_Init(void);
void SteamAPI_Shutdown(void);
void SteamAPI_RunCallbacks(void);

/* Accessors */
ISteamUser*          SteamAPI_SteamUser_v023(void);
ISteamFriends*       SteamAPI_SteamFriends_v017(void);
ISteamUserStats*     SteamAPI_SteamUserStats_v012(void);
ISteamRemoteStorage* SteamAPI_SteamRemoteStorage_v016(void);
ISteamApps*          SteamAPI_SteamApps_v008(void);
ISteamUtils*         SteamAPI_SteamUtils_v010(void);

/* ISteamUser */
uint64_steamid SteamAPI_ISteamUser_GetSteamID(ISteamUser *self);

/* ISteamFriends */
const char* SteamAPI_ISteamFriends_GetPersonaName(ISteamFriends *self);
bool        SteamAPI_ISteamFriends_SetRichPresence(ISteamFriends *self, const char *key, const char *value);
void        SteamAPI_ISteamFriends_ActivateGameOverlay(ISteamFriends *self, const char *dialog);

/* ISteamUserStats */
bool SteamAPI_ISteamUserStats_SetAchievement(ISteamUserStats *self, const char *name);
bool SteamAPI_ISteamUserStats_ClearAchievement(ISteamUserStats *self, const char *name);
bool SteamAPI_ISteamUserStats_StoreStats(ISteamUserStats *self);
bool SteamAPI_ISteamUserStats_GetStatInt32(ISteamUserStats *self, const char *name, int32 *data);
bool SteamAPI_ISteamUserStats_SetStatInt32(ISteamUserStats *self, const char *name, int32 data);
bool SteamAPI_ISteamUserStats_GetStatFloat(ISteamUserStats *self, const char *name, float *data);
bool SteamAPI_ISteamUserStats_SetStatFloat(ISteamUserStats *self, const char *name, float data);

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

/* ISteamUtils */
AppId_t     SteamAPI_ISteamUtils_GetAppID(ISteamUtils *self);
bool        SteamAPI_ISteamUtils_IsOverlayEnabled(ISteamUtils *self);
const char* SteamAPI_ISteamUtils_GetIPCountry(ISteamUtils *self);

#endif /* STEAM_API_FLAT_H */
