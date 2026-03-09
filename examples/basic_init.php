<?php
/**
 * Minimales Steam Init + Shutdown Beispiel.
 *
 * Voraussetzung: steam_appid.txt mit App-ID im Arbeitsverzeichnis.
 * Für Tests: 480 (Spacewar)
 */

if (!steam_init()) {
    echo "Steam konnte nicht initialisiert werden. Läuft Steam?\n";
    exit(1);
}

echo "Steam initialisiert!\n";
echo "Spielername: " . steam_friends_get_name() . "\n";
echo "App-ID: " . steam_utils_get_app_id() . "\n";

// Game Loop Simulation
for ($i = 0; $i < 60; $i++) {
    steam_run_callbacks();
    usleep(16667); // ~60 FPS
}

steam_shutdown();
echo "Steam heruntergefahren.\n";
