<?php
/**
 * Achievement Unlock Workflow.
 *
 * Voraussetzung: steam_appid.txt + Steam läuft.
 * Achievement-IDs müssen im Steamworks Partner-Backend konfiguriert sein.
 */

if (!steam_init()) {
    echo "Steam konnte nicht initialisiert werden.\n";
    exit(1);
}

// Achievement freischalten
$achievement_id = 'ACH_WIN_ONE_GAME';

if (steam_stats_set_achievement($achievement_id)) {
    echo "Achievement '$achievement_id' freigeschaltet!\n";
} else {
    echo "Achievement konnte nicht gesetzt werden.\n";
}

// Auf Steam-Server persistieren
if (steam_stats_store()) {
    echo "Stats gespeichert.\n";
} else {
    echo "Stats konnten nicht gespeichert werden.\n";
}

// Callbacks verarbeiten damit Steam das Achievement anzeigt
for ($i = 0; $i < 10; $i++) {
    steam_run_callbacks();
    usleep(100000);
}

steam_shutdown();
