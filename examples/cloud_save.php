<?php
/**
 * Cloud Save Read/Write Beispiel.
 *
 * Voraussetzung: steam_appid.txt + Steam läuft.
 * Cloud Storage muss im Steamworks Partner-Backend aktiviert sein.
 */

if (!steam_init()) {
    echo "Steam konnte nicht initialisiert werden.\n";
    exit(1);
}

$save_file = 'savegame.json';

// Speicherstand schreiben
$save_data = json_encode([
    'level' => 5,
    'score' => 12500,
    'timestamp' => time(),
]);

if (steam_remote_file_write($save_file, $save_data)) {
    echo "Speicherstand in Cloud geschrieben.\n";
} else {
    echo "Cloud Save fehlgeschlagen.\n";
}

// Speicherstand lesen
if (steam_remote_file_exists($save_file)) {
    $data = steam_remote_file_read($save_file);
    if ($data !== false) {
        $save = json_decode($data, true);
        echo "Geladen: Level {$save['level']}, Score {$save['score']}\n";
    }
} else {
    echo "Kein Speicherstand in der Cloud gefunden.\n";
}

// Alle Cloud-Dateien auflisten
$files = steam_remote_file_list();
if ($files !== false) {
    echo "Cloud-Dateien:\n";
    foreach ($files as $file) {
        echo "  - $file\n";
    }
}

steam_shutdown();
