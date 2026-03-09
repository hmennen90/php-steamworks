<?php
// stubs/steamworks.php
// AUTO-GENERATED — nicht manuell bearbeiten

/* ── steam_init.c ── */

/**
 * Initialisiert die Steam API.
 * Muss vor allen anderen steam_* Funktionen aufgerufen werden.
 * Steam muss laufen, steam_appid.txt muss im Arbeitsverzeichnis liegen.
 *
 * @return bool true bei Erfolg, false wenn Steam nicht läuft
 */
function steam_init(): bool {}

/**
 * Fährt die Steam API herunter.
 * Sollte beim Beenden der Anwendung aufgerufen werden.
 *
 * @return void
 */
function steam_shutdown(): void {}

/**
 * Verarbeitet ausstehende Steam Callbacks.
 * MUSS jeden Frame aufgerufen werden.
 *
 * @return void
 */
function steam_run_callbacks(): void {}

/* ── steam_user.c ── */

/**
 * Gibt die SteamID des aktuellen Nutzers zurück.
 *
 * @return int|false SteamID als Integer, false bei Fehler
 */
function steam_user_get_steam_id(): int|false {}

/* ── steam_friends.c ── */

/**
 * Gibt den Anzeigenamen des aktuellen Nutzers zurück.
 *
 * @return string|false Spielername, false bei Fehler
 */
function steam_friends_get_name(): string|false {}

/**
 * Setzt einen Rich Presence Key/Value Eintrag.
 *
 * @param string $key Rich Presence Key
 * @param string|null $value Wert (null oder leer zum Löschen)
 * @return bool true bei Erfolg
 */
function steam_friends_set_rich_presence(string $key, ?string $value = null): bool {}

/**
 * Öffnet das Steam Overlay mit einem bestimmten Dialog.
 *
 * @param string $dialog z.B. "Friends", "Community", "Players", "Settings",
 *                       "OfficialGameGroup", "Stats", "Achievements"
 * @return void
 */
function steam_friends_activate_overlay(string $dialog): void {}

/* ── steam_stats.c ── */

/**
 * Schaltet ein Achievement frei.
 * Danach steam_stats_store() aufrufen zum Persistieren.
 *
 * @param string $achievement_id Achievement-ID aus dem Steamworks Partner-Backend
 * @return bool true bei Erfolg
 */
function steam_stats_set_achievement(string $achievement_id): bool {}

/**
 * Setzt ein Achievement zurück (nur für Entwicklung/Testing).
 *
 * @param string $achievement_id Achievement-ID
 * @return bool true bei Erfolg
 */
function steam_stats_clear_achievement(string $achievement_id): bool {}

/**
 * Persistiert alle Stat- und Achievement-Änderungen auf Steam-Server.
 *
 * @return bool true bei Erfolg
 */
function steam_stats_store(): bool {}

/**
 * Liest einen Integer-Stat.
 *
 * @param string $name Stat-Name aus dem Steamworks Partner-Backend
 * @return int|false Stat-Wert, false bei Fehler
 */
function steam_stats_get_int(string $name): int|false {}

/**
 * Setzt einen Integer-Stat.
 *
 * @param string $name Stat-Name
 * @param int $value Neuer Wert
 * @return bool true bei Erfolg
 */
function steam_stats_set_int(string $name, int $value): bool {}

/**
 * Liest einen Float-Stat.
 *
 * @param string $name Stat-Name
 * @return float|false Stat-Wert, false bei Fehler
 */
function steam_stats_get_float(string $name): float|false {}

/**
 * Setzt einen Float-Stat.
 *
 * @param string $name Stat-Name
 * @param float $value Neuer Wert
 * @return bool true bei Erfolg
 */
function steam_stats_set_float(string $name, float $value): bool {}

/* ── steam_remote.c ── */

/**
 * Schreibt Daten in eine Cloud-Datei.
 *
 * @param string $filename Dateiname im Cloud-Storage
 * @param string $data Zu schreibende Daten
 * @return bool true bei Erfolg
 */
function steam_remote_file_write(string $filename, string $data): bool {}

/**
 * Liest eine Cloud-Datei.
 *
 * @param string $filename Dateiname im Cloud-Storage
 * @return string|false Dateiinhalt, false bei Fehler
 */
function steam_remote_file_read(string $filename): string|false {}

/**
 * Prüft ob eine Cloud-Datei existiert.
 *
 * @param string $filename Dateiname im Cloud-Storage
 * @return bool true wenn die Datei existiert
 */
function steam_remote_file_exists(string $filename): bool {}

/**
 * Löscht eine Cloud-Datei.
 *
 * @param string $filename Dateiname im Cloud-Storage
 * @return bool true bei Erfolg
 */
function steam_remote_file_delete(string $filename): bool {}

/**
 * Listet alle Cloud-Dateien auf.
 *
 * @return array<string>|false Liste der Dateinamen, false bei Fehler
 */
function steam_remote_file_list(): array|false {}

/* ── steam_apps.c ── */

/**
 * Prüft ob der Nutzer das Spiel besitzt/abonniert hat.
 *
 * @return bool true wenn abonniert
 */
function steam_apps_is_subscribed(): bool {}

/**
 * Prüft ob ein DLC installiert ist.
 *
 * @param int $dlc_id DLC App-ID
 * @return bool true wenn installiert
 */
function steam_apps_is_dlc_installed(int $dlc_id): bool {}

/**
 * Gibt die App-ID des aktuellen Spiels zurück.
 *
 * @return int|false App-ID, false bei Fehler
 */
function steam_apps_get_app_id(): int|false {}

/**
 * Gibt die Sprache zurück, die der Nutzer in Steam eingestellt hat.
 *
 * @return string|false Sprachcode (z.B. "english", "german"), false bei Fehler
 */
function steam_apps_get_language(): string|false {}

/* ── steam_utils.c ── */

/**
 * Gibt die App-ID über ISteamUtils zurück.
 *
 * @return int|false App-ID, false bei Fehler
 */
function steam_utils_get_app_id(): int|false {}

/**
 * Prüft ob das Steam Overlay aktiv ist.
 *
 * @return bool true wenn Overlay verfügbar
 */
function steam_utils_is_overlay_enabled(): bool {}

/**
 * Gibt den ISO-Ländercode des Nutzers zurück (basierend auf IP).
 *
 * @return string|false Zwei-Buchstaben-Ländercode, false bei Fehler
 */
function steam_utils_get_country_code(): string|false {}
