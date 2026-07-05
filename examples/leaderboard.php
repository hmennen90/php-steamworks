<?php
/**
 * Leaderboard Workflow — Score hochladen und Top-Einträge lesen.
 *
 * Voraussetzung: steam_appid.txt + Steam läuft.
 * Das Leaderboard muss im Steamworks Partner-Backend konfiguriert sein
 * (oder wird via find_or_create angelegt).
 *
 * Leaderboard-Calls sind ASYNCHRON: jede Funktion gibt ein Handle zurück,
 * dessen Ergebnis über steam_get_call_result() gepollt wird — jeden Frame,
 * bis nicht mehr null zurückkommt.
 */

if (!steam_init()) {
    echo "Steam konnte nicht initialisiert werden.\n";
    exit(1);
}

/**
 * Pollt ein Call-Handle bis zum Ergebnis. In einem echten Spiel würde man
 * NICHT blockierend warten, sondern das Handle über mehrere Frames pollen.
 */
function await(int $handle, int $max_frames = 100): array|false|null
{
    for ($i = 0; $i < $max_frames; $i++) {
        $result = steam_get_call_result($handle);
        if ($result !== null) {
            return $result; // array bei Erfolg, false bei Fehler
        }
        steam_run_callbacks();
        usleep(50000);
    }
    return null; // Timeout
}

// 1. Leaderboard suchen (oder anlegen)
$handle = steam_stats_find_or_create_leaderboard(
    'HighScores',
    STEAM_LEADERBOARD_SORT_DESCENDING,
    STEAM_LEADERBOARD_DISPLAY_NUMERIC
);
$found = await($handle);

if (!is_array($found) || !$found['found']) {
    echo "Leaderboard nicht gefunden.\n";
    steam_shutdown();
    exit(1);
}
$leaderboard = $found['leaderboard'];
echo "Leaderboard-Handle: $leaderboard\n";

// 2. Score hochladen
$upload = await(steam_stats_upload_score(
    $leaderboard, 1500, STEAM_LEADERBOARD_UPLOAD_KEEP_BEST
));
if (is_array($upload) && $upload['success']) {
    echo "Score hochgeladen. Neuer Rang: {$upload['rank_new']} (vorher {$upload['rank_prev']})\n";
}

// 3. Top 10 herunterladen
$download = await(steam_stats_download_leaderboard_entries(
    $leaderboard, STEAM_LEADERBOARD_DATA_GLOBAL, 1, 10
));
if (is_array($download)) {
    echo "Top {$download['count']}:\n";
    for ($i = 0; $i < $download['count']; $i++) {
        $entry = steam_stats_get_downloaded_entry($download['entries'], $i);
        if ($entry !== null) {
            echo "  #{$entry['global_rank']}  SteamID {$entry['steam_id']}  Score {$entry['score']}\n";
        }
    }
}

steam_shutdown();
