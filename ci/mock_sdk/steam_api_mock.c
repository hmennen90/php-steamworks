/*
 * Mock implementation of the Steamworks flat API for CI testing.
 * All functions return safe default/failure values.
 */

#include "public/steam/steam_api_flat.h"

static int mock_instance = 0;

/* Core */
bool SteamAPI_Init(void)     { return false; }
void SteamAPI_Shutdown(void) { }
void SteamAPI_RunCallbacks(void) { }

/* Accessors — return a non-NULL pointer so functions can proceed */
ISteamUser*          SteamAPI_SteamUser_v023(void)          { return (ISteamUser*)&mock_instance; }
ISteamFriends*       SteamAPI_SteamFriends_v017(void)       { return (ISteamFriends*)&mock_instance; }
ISteamUserStats*     SteamAPI_SteamUserStats_v012(void)     { return (ISteamUserStats*)&mock_instance; }
ISteamRemoteStorage* SteamAPI_SteamRemoteStorage_v016(void) { return (ISteamRemoteStorage*)&mock_instance; }
ISteamApps*          SteamAPI_SteamApps_v008(void)          { return (ISteamApps*)&mock_instance; }
ISteamUtils*         SteamAPI_SteamUtils_v010(void)         { return (ISteamUtils*)&mock_instance; }

/* ISteamUser */
uint64_steamid SteamAPI_ISteamUser_GetSteamID(ISteamUser *self) { return 0; }

/* ISteamFriends */
const char* SteamAPI_ISteamFriends_GetPersonaName(ISteamFriends *self) { return "MockPlayer"; }
bool        SteamAPI_ISteamFriends_SetRichPresence(ISteamFriends *self, const char *key, const char *value) { return false; }
void        SteamAPI_ISteamFriends_ActivateGameOverlay(ISteamFriends *self, const char *dialog) { }

/* ISteamUserStats */
bool SteamAPI_ISteamUserStats_SetAchievement(ISteamUserStats *self, const char *name)   { return false; }
bool SteamAPI_ISteamUserStats_ClearAchievement(ISteamUserStats *self, const char *name) { return false; }
bool SteamAPI_ISteamUserStats_StoreStats(ISteamUserStats *self)                          { return false; }
bool SteamAPI_ISteamUserStats_GetStatInt32(ISteamUserStats *self, const char *name, int32 *data)  { *data = 0; return false; }
bool SteamAPI_ISteamUserStats_SetStatInt32(ISteamUserStats *self, const char *name, int32 data)   { return false; }
bool SteamAPI_ISteamUserStats_GetStatFloat(ISteamUserStats *self, const char *name, float *data)  { *data = 0.0f; return false; }
bool SteamAPI_ISteamUserStats_SetStatFloat(ISteamUserStats *self, const char *name, float data)   { return false; }

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
