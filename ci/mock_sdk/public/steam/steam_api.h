#ifndef STEAM_API_H
#define STEAM_API_H

/*
 * Mock Steamworks SDK headers for CI testing.
 * These stubs allow the extension to compile without the real SDK.
 * All functions return failure/default values.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef uint32_t AppId_t;
typedef uint32_t DepotId_t;
typedef uint64_t uint64_steamid;
typedef int32_t  int32;
typedef uint32_t uint32;

/* Async / leaderboard handle types (opaque uint64 on the SDK side) */
typedef uint64_t SteamAPICall_t;
typedef uint64_t SteamLeaderboard_t;
typedef uint64_t SteamLeaderboardEntries_t;
typedef uint64_t CSteamID_t;
typedef uint64_t UGCHandle_t;
typedef uint64_t TimelineEventHandle_t;
typedef uint32_t HAuthTicket;

typedef void ISteamUser;
typedef void ISteamFriends;
typedef void ISteamUserStats;
typedef void ISteamRemoteStorage;
typedef void ISteamApps;
typedef void ISteamUtils;
typedef void ISteamUGC;
typedef void ISteamTimeline;

#endif /* STEAM_API_H */
