#include "../php_steamworks.h"
#include "../steam_api_c.h"

/*
 * Async CallResult infrastructure (Handle + Poll model).
 *
 * Steam's asynchronous calls (leaderboards, UGC, auth tickets, …) return a
 * SteamAPICall_t handle. The result is delivered later and must be fetched via
 * ISteamUtils::GetAPICallResult, which needs the expected callback id and the
 * result struct size — both of which depend on *which* call was made.
 *
 * We keep an internal registry mapping each pending SteamAPICall_t to the kind
 * of result it will produce. PHP userland calls the async function (gets an int
 * handle back), then polls steam_get_call_result($handle) each frame until it
 * stops returning null.
 *
 * PHP is single-threaded and Steam callbacks follow the same model (see
 * CLAUDE.md), so a plain file-static HashTable keyed by the handle is safe.
 */

static HashTable steamworks_pending_calls;
static bool steamworks_registry_initialized = false;

void steamworks_async_minit(void)
{
    /* Persistent (pemalloc) — the registry outlives individual requests in a
       long-running CLI game loop, and stores only integer kinds (no zvals). */
    zend_hash_init(&steamworks_pending_calls, 8, NULL, NULL, 1);
    steamworks_registry_initialized = true;
}

void steamworks_async_mshutdown(void)
{
    if (steamworks_registry_initialized) {
        zend_hash_destroy(&steamworks_pending_calls);
        steamworks_registry_initialized = false;
    }
}

void steamworks_register_call(uint64_t handle, enum steamworks_call_kind kind)
{
    if (!steamworks_registry_initialized || handle == 0) {
        return;
    }
    /* Store the kind as a pointer value (kind is >= 1, so it is never NULL). */
    zend_hash_index_update_ptr(&steamworks_pending_calls,
        (zend_ulong)handle, (void *)(uintptr_t)kind);
}

PHP_FUNCTION(steam_get_call_result)
{
    zend_long handle_long;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(handle_long)
    ZEND_PARSE_PARAMETERS_END();

    SteamAPICall_t handle = (SteamAPICall_t)handle_long;

    void *stored = steamworks_registry_initialized
        ? zend_hash_index_find_ptr(&steamworks_pending_calls, (zend_ulong)handle)
        : NULL;
    if (!stored) {
        php_error_docref(NULL, E_WARNING, "Unknown or already-consumed Steam call handle");
        RETURN_NULL();
    }
    enum steamworks_call_kind kind = (enum steamworks_call_kind)(uintptr_t)stored;

    ISteamUtils *utils = SteamAPI_SteamUtils_v010();
    if (!utils) {
        php_error_docref(NULL, E_WARNING, "Steam not initialized");
        RETURN_NULL();
    }

    bool failed = false;
    if (!SteamAPI_ISteamUtils_IsAPICallCompleted(utils, handle, &failed)) {
        /* Still pending — caller should poll again next frame. */
        RETURN_NULL();
    }

    /* Completed (success or failure): consume the handle either way. */
    zend_hash_index_del(&steamworks_pending_calls, (zend_ulong)handle);

    if (failed) {
        php_error_docref(NULL, E_WARNING, "Steam async call failed");
        RETURN_FALSE;
    }

    bool io_failed = false;

    switch (kind) {
        case STEAMWORKS_CALL_LEADERBOARD_FIND: {
            LeaderboardFindResult_t result;
            memset(&result, 0, sizeof(result));
            if (!SteamAPI_ISteamUtils_GetAPICallResult(utils, handle, &result,
                    (int)sizeof(result), k_iCallback_LeaderboardFindResult, &io_failed)
                || io_failed) {
                php_error_docref(NULL, E_WARNING, "Failed to read leaderboard find result");
                RETURN_FALSE;
            }
            array_init(return_value);
            add_assoc_string(return_value, "type", "leaderboard_found");
            add_assoc_bool(return_value, "found", result.m_bLeaderboardFound != 0);
            add_assoc_long(return_value, "leaderboard", (zend_long)result.m_hSteamLeaderboard);
            return;
        }
        case STEAMWORKS_CALL_SCORE_UPLOADED: {
            LeaderboardScoreUploaded_t result;
            memset(&result, 0, sizeof(result));
            if (!SteamAPI_ISteamUtils_GetAPICallResult(utils, handle, &result,
                    (int)sizeof(result), k_iCallback_LeaderboardScoreUploaded, &io_failed)
                || io_failed) {
                php_error_docref(NULL, E_WARNING, "Failed to read score upload result");
                RETURN_FALSE;
            }
            array_init(return_value);
            add_assoc_string(return_value, "type", "score_uploaded");
            add_assoc_bool(return_value, "success", result.m_bSuccess != 0);
            add_assoc_long(return_value, "score", (zend_long)result.m_nScore);
            add_assoc_bool(return_value, "changed", result.m_bScoreChanged != 0);
            add_assoc_long(return_value, "rank_new", (zend_long)result.m_nGlobalRankNew);
            add_assoc_long(return_value, "rank_prev", (zend_long)result.m_nGlobalRankPrevious);
            return;
        }
        case STEAMWORKS_CALL_SCORES_DOWNLOADED: {
            LeaderboardScoresDownloaded_t result;
            memset(&result, 0, sizeof(result));
            if (!SteamAPI_ISteamUtils_GetAPICallResult(utils, handle, &result,
                    (int)sizeof(result), k_iCallback_LeaderboardScoresDownloaded, &io_failed)
                || io_failed) {
                php_error_docref(NULL, E_WARNING, "Failed to read leaderboard download result");
                RETURN_FALSE;
            }
            array_init(return_value);
            add_assoc_string(return_value, "type", "scores_downloaded");
            add_assoc_long(return_value, "leaderboard", (zend_long)result.m_hSteamLeaderboard);
            add_assoc_long(return_value, "entries", (zend_long)result.m_hSteamLeaderboardEntries);
            add_assoc_long(return_value, "count", (zend_long)result.m_cEntryCount);
            return;
        }
        default:
            php_error_docref(NULL, E_WARNING, "Unknown Steam call kind");
            RETURN_FALSE;
    }
}
