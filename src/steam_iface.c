/*
 * steam_iface.c — runtime resolution of Steamworks interface versions.
 *
 * Steam exposes each interface under a versioned name. The SDK's
 * SteamAPI_Steam*_vNNN() accessors bake that name into a link-time symbol, so
 * a build only works against the exact SDK release it was compiled with — SDK
 * 1.65 dropped SteamAPI_SteamUtils_v010 and broke linking outright. Resolving
 * the name at runtime instead lets one binary serve several SDK releases.
 *
 * Each interface carries a table of versions, newest first. Entries after the
 * first are legacy versions: they keep working for backwards compatibility but
 * are deprecated and will be dropped in a future release, and using one raises
 * E_DEPRECATED.
 *
 * A legacy entry is only permissible when the older interface has the SAME
 * vtable layout as the current one. The flat SteamAPI_ISteamX_* functions are
 * compiled into libsteam_api against the current interface, so they call fixed
 * vtable slots; handing them an object of a version whose layout differs calls
 * the wrong method. Adding methods at the end is safe, removing or reordering
 * any method is not. Verify before adding an entry, by comparing the virtual
 * method order of both SDK headers:
 *
 *   diff <(grep -oE 'virtual [^(]+\(' old/isteamfoo.h) \
 *        <(grep -oE 'virtual [^(]+\(' new/isteamfoo.h)
 *
 * This is why ISteamUtils has no fallback: SDK 1.65 removed GetCSERIPPort and
 * IsSteamRunningOnSteamDeck from the middle of the interface, so SteamUtils010
 * and SteamUtils011 are not layout-compatible. ISteamNetworkingSockets012 and
 * 013 have identical layouts, so that fallback is safe.
 *
 * The version strings are copied verbatim from the SDK's *_INTERFACE_VERSION
 * defines. They are NOT derivable from the interface or accessor name —
 * ISteamApps is "STEAMAPPS_INTERFACE_VERSION009", not "SteamApps009". A wrong
 * string fails silently at runtime (the interface simply never resolves), so
 * never guess one; copy it from the SDK header. SteamInterfaceTest guards the
 * table against drift.
 */

#include "php_steamworks.h"
#include "steam_iface.h"

/* Steamworks SDK 1.65 */
static const char *const user_versions[]      = { "SteamUser023",                        NULL };
static const char *const friends_versions[]   = { "SteamFriends018",                     NULL };
static const char *const stats_versions[]     = { "STEAMUSERSTATS_INTERFACE_VERSION013", NULL };
static const char *const remote_versions[]    = { "STEAMREMOTESTORAGE_INTERFACE_VERSION016", NULL };
static const char *const apps_versions[]      = { "STEAMAPPS_INTERFACE_VERSION009",      NULL };
static const char *const timeline_versions[]  = { "STEAMTIMELINE_INTERFACE_V004",        NULL };
static const char *const ugc_versions[]       = { "STEAMUGC_INTERFACE_VERSION021",       NULL };
static const char *const net_utils_versions[] = { "SteamNetworkingUtils004",             NULL };

/*
 * No SteamUtils010 fallback: SDK 1.65 removed two methods from the middle of
 * ISteamUtils, so the older interface is not vtable-compatible with the flat
 * functions in libsteam_api 1.65. An old client fails cleanly instead.
 */
static const char *const utils_versions[] = { "SteamUtils011", NULL };

static const char *const net_versions[] = {
    "SteamNetworkingSockets013",  /* SDK 1.65+ */
    "SteamNetworkingSockets012",  /* SDK <= 1.64 — deprecated, layout-identical */
    NULL
};

typedef struct {
    const char        *name;      /* interface name, for the deprecation notice */
    const char *const *versions;  /* newest first, NULL-terminated */
    bool               notified;  /* legacy notice already raised this process */
} steamworks_iface;

static steamworks_iface iface_user      = { "ISteamUser",              user_versions,      false };
static steamworks_iface iface_friends   = { "ISteamFriends",           friends_versions,   false };
static steamworks_iface iface_stats     = { "ISteamUserStats",         stats_versions,     false };
static steamworks_iface iface_remote    = { "ISteamRemoteStorage",     remote_versions,    false };
static steamworks_iface iface_apps      = { "ISteamApps",              apps_versions,      false };
static steamworks_iface iface_utils     = { "ISteamUtils",             utils_versions,     false };
static steamworks_iface iface_timeline  = { "ISteamTimeline",          timeline_versions,  false };
static steamworks_iface iface_ugc       = { "ISteamUGC",               ugc_versions,       false };
static steamworks_iface iface_net       = { "ISteamNetworkingSockets", net_versions,       false };
static steamworks_iface iface_net_utils = { "ISteamNetworkingUtils",   net_utils_versions, false };

/*
 * Returns the newest interface version Steam provides, or NULL if it provides
 * none of them — which is also the case before steam_init().
 *
 * `notified` is process-global on purpose: these accessors run on every Steam
 * call, so a per-call notice would fire every frame of the game loop. The
 * extension is single-threaded by design (see steam_run_callbacks), so the
 * unguarded write is safe.
 */
static void *steamworks_resolve(steamworks_iface *iface)
{
    HSteamUser user = SteamAPI_GetHSteamUser();

    for (int i = 0; iface->versions[i] != NULL; i++) {
        void *ptr = SteamInternal_FindOrCreateUserInterface(user, iface->versions[i]);
        if (ptr == NULL) {
            continue;
        }

        if (i > 0 && !iface->notified) {
            iface->notified = true;
            php_error_docref(NULL, E_DEPRECATED,
                "Steam provides only the legacy %s interface \"%s\". Support for it "
                "will be removed in a future release; update to Steamworks SDK 1.65 "
                "or newer and to a current Steam client",
                iface->name, iface->versions[i]);
        }

        return ptr;
    }

    /*
     * A live Steam session that offers none of our versions means the client
     * predates this build. Say so — otherwise call sites report the generic
     * "Steam not initialized", which sends people looking in the wrong place.
     */
    if (user != 0 && !iface->notified) {
        iface->notified = true;
        php_error_docref(NULL, E_WARNING,
            "Steam provides no supported %s version (need \"%s\"). "
            "The Steam client is older than this build of the extension",
            iface->name, iface->versions[0]);
    }

    return NULL;
}

ISteamUser              *steamworks_user(void)           { return steamworks_resolve(&iface_user); }
ISteamFriends           *steamworks_friends(void)        { return steamworks_resolve(&iface_friends); }
ISteamUserStats         *steamworks_stats(void)          { return steamworks_resolve(&iface_stats); }
ISteamRemoteStorage     *steamworks_remote_storage(void) { return steamworks_resolve(&iface_remote); }
ISteamApps              *steamworks_apps(void)           { return steamworks_resolve(&iface_apps); }
ISteamUtils             *steamworks_utils(void)          { return steamworks_resolve(&iface_utils); }
ISteamTimeline          *steamworks_timeline(void)       { return steamworks_resolve(&iface_timeline); }
ISteamUGC               *steamworks_ugc(void)            { return steamworks_resolve(&iface_ugc); }
ISteamNetworkingSockets *steamworks_net(void)            { return steamworks_resolve(&iface_net); }
ISteamNetworkingUtils   *steamworks_net_utils(void)      { return steamworks_resolve(&iface_net_utils); }
