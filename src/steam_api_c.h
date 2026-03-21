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
ISteamApps         *SteamAPI_SteamApps_v008(void);
ISteamUtils        *SteamAPI_SteamUtils_v010(void);

/* ── ISteamUser ────────────────────────────────────────────────────── */
uint64_steamid SteamAPI_ISteamUser_GetSteamID(ISteamUser *self);

/* ── ISteamFriends ─────────────────────────────────────────────────── */
const char *SteamAPI_ISteamFriends_GetPersonaName(ISteamFriends *self);
bool        SteamAPI_ISteamFriends_SetRichPresence(ISteamFriends *self, const char *key, const char *value);
void        SteamAPI_ISteamFriends_ActivateGameOverlay(ISteamFriends *self, const char *dialog);

/* ── ISteamUserStats ───────────────────────────────────────────────── */
bool SteamAPI_ISteamUserStats_SetAchievement(ISteamUserStats *self, const char *name);
bool SteamAPI_ISteamUserStats_ClearAchievement(ISteamUserStats *self, const char *name);
bool SteamAPI_ISteamUserStats_StoreStats(ISteamUserStats *self);
bool SteamAPI_ISteamUserStats_GetStatInt32(ISteamUserStats *self, const char *name, int32 *data);
bool SteamAPI_ISteamUserStats_SetStatInt32(ISteamUserStats *self, const char *name, int32 data);
bool SteamAPI_ISteamUserStats_GetStatFloat(ISteamUserStats *self, const char *name, float *data);
bool SteamAPI_ISteamUserStats_SetStatFloat(ISteamUserStats *self, const char *name, float data);

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

#endif /* STEAM_API_C_H */
