/*
 * steam_iface.h — accessors for the Steamworks interfaces used by this extension.
 *
 * Each accessor resolves its interface at runtime and returns NULL when Steam
 * is not initialized or provides no supported version. Call sites treat NULL
 * the way they always have: E_WARNING plus a false return.
 */

#ifndef STEAM_IFACE_H
#define STEAM_IFACE_H

#include "steam_api_c.h"

ISteamUser              *steamworks_user(void);
ISteamFriends           *steamworks_friends(void);
ISteamUserStats         *steamworks_stats(void);
ISteamRemoteStorage     *steamworks_remote_storage(void);
ISteamApps              *steamworks_apps(void);
ISteamUtils             *steamworks_utils(void);
ISteamTimeline          *steamworks_timeline(void);
ISteamUGC               *steamworks_ugc(void);
ISteamNetworkingSockets *steamworks_net(void);
ISteamNetworkingUtils   *steamworks_net_utils(void);
ISteamMatchmaking       *steamworks_matchmaking(void);

#endif /* STEAM_IFACE_H */
