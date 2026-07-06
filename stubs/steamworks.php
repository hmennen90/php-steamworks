<?php
// stubs/steamworks.php
// AUTO-GENERATED — nicht manuell bearbeiten

/* ── Leaderboard-Konstanten (ELeaderboard* aus der SDK) ── */
const STEAM_LEADERBOARD_SORT_ASCENDING = 1;
const STEAM_LEADERBOARD_SORT_DESCENDING = 2;
const STEAM_LEADERBOARD_DISPLAY_NUMERIC = 1;
const STEAM_LEADERBOARD_DISPLAY_TIME_SECONDS = 2;
const STEAM_LEADERBOARD_DISPLAY_TIME_MILLISECONDS = 3;
const STEAM_LEADERBOARD_UPLOAD_KEEP_BEST = 1;
const STEAM_LEADERBOARD_UPLOAD_FORCE_UPDATE = 2;
const STEAM_LEADERBOARD_DATA_GLOBAL = 0;
const STEAM_LEADERBOARD_DATA_GLOBAL_AROUND_USER = 1;
const STEAM_LEADERBOARD_DATA_FRIENDS = 2;

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

/**
 * Prüft, ob der Nutzer aktuell bei Steam eingeloggt und online ist.
 *
 * @return bool true wenn eingeloggt
 */
function steam_user_is_logged_on(): bool {}

/**
 * Gibt das Steam-Level des Nutzers zurück.
 *
 * @return int|false Steam-Level, false bei Fehler
 */
function steam_user_get_player_steam_level(): int|false {}

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

/**
 * Öffnet eine URL im Steam Overlay Browser.
 *
 * @param string $url Die zu öffnende URL
 * @param bool $modal true für modales Fenster (blockiert Spiel), false für normales Overlay (Standard)
 * @return void
 */
function steam_friends_activate_overlay_to_web_page(string $url, bool $modal = false): void {}

/**
 * Gibt den eigenen Persona-State zurück (STEAM_PERSONA_STATE_*).
 *
 * @return int|false EPersonaState, false wenn Steam nicht initialisiert
 */
function steam_friends_get_persona_state(): int|false {}

/**
 * Anzahl der Freunde, die den angegebenen Flags entsprechen.
 *
 * @param int $flags STEAM_FRIEND_FLAG_* (Default: IMMEDIATE = normale Freundesliste)
 * @return int|false Anzahl, false bei Fehler
 */
function steam_friends_get_friend_count(int $flags = STEAM_FRIEND_FLAG_IMMEDIATE): int|false {}

/**
 * SteamID des Freundes an Position $index (0-basiert).
 *
 * @param int $index 0..(get_friend_count()-1)
 * @param int $flags STEAM_FRIEND_FLAG_* (muss zu get_friend_count() passen)
 * @return int|false SteamID (64-bit), false bei Fehler
 */
function steam_friends_get_friend_by_index(int $index, int $flags = STEAM_FRIEND_FLAG_IMMEDIATE): int|false {}

/**
 * Beziehung zu einem Nutzer (STEAM_FRIEND_RELATIONSHIP_*).
 *
 * @param int $steam_id SteamID
 * @return int|false EFriendRelationship, false bei Fehler
 */
function steam_friends_get_friend_relationship(int $steam_id): int|false {}

/**
 * Persona-State eines Freundes (STEAM_PERSONA_STATE_*).
 *
 * @param int $steam_id SteamID
 * @return int|false EPersonaState, false bei Fehler
 */
function steam_friends_get_friend_persona_state(int $steam_id): int|false {}

/**
 * Anzeigename eines Freundes/Nutzers.
 * Nur verlässlich für gecachte Nutzer — sonst vorher request_user_information() aufrufen.
 *
 * @param int $steam_id SteamID
 * @return string|false Name, false bei Fehler
 */
function steam_friends_get_friend_persona_name(int $steam_id): string|false {}

/**
 * Fordert Persona-/Avatar-Infos für einen (noch nicht gecachten) Nutzer an.
 *
 * @param int $steam_id SteamID
 * @param bool $name_only true = nur Name, kein Avatar
 * @return bool true wenn Daten angefordert werden (noch nicht verfügbar),
 *              false wenn bereits lokal vorhanden
 */
function steam_friends_request_user_information(int $steam_id, bool $name_only = false): bool {}

/**
 * Avatar eines Nutzers als rohe RGBA8888-Bytes.
 * Liefert null, wenn kein Avatar gesetzt ist oder er noch nicht geladen wurde
 * (dann request_user_information() aufrufen und später erneut versuchen).
 *
 * @param int $steam_id SteamID
 * @param int $size STEAM_AVATAR_SMALL (32px) | STEAM_AVATAR_MEDIUM (64px) | STEAM_AVATAR_LARGE (128px)
 * @return array{width:int, height:int, rgba:string}|null rgba = width*height*4 Bytes
 */
function steam_friends_get_friend_avatar(int $steam_id, int $size = STEAM_AVATAR_MEDIUM): ?array {}

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

/**
 * Zeigt dem Spieler eine Fortschritts-Benachrichtigung für ein noch nicht
 * freigeschaltetes Achievement ("47 von 100").
 *
 * Voraussetzung: Das Achievement muss im Steamworks Partner-Backend mit einer
 * Progress-Stat konfiguriert sein, sonst wird nichts angezeigt. NICHT mit
 * $cur_progress >= $max_progress aufrufen — dort gehört steam_stats_set_achievement() hin.
 *
 * @param string $achievement_id Achievement-ID aus dem Steamworks Partner-Backend
 * @param int $cur_progress Aktueller Fortschritt
 * @param int $max_progress Zielwert
 * @return bool true bei Erfolg
 */
function steam_stats_indicate_achievement_progress(string $achievement_id, int $cur_progress, int $max_progress): bool {}

/**
 * Prüft, ob ein Achievement freigeschaltet ist.
 *
 * @param string $achievement_id Achievement-ID
 * @return bool|null true=freigeschaltet, false=nicht freigeschaltet,
 *                   null bei unbekannter ID / Stats nicht geladen
 */
function steam_stats_get_achievement(string $achievement_id): ?bool {}

/**
 * Gibt den Unlock-Zeitpunkt eines Achievements als Unix-Timestamp zurück.
 *
 * @param string $achievement_id Achievement-ID
 * @return int|null Unix-Timestamp, oder null wenn nicht freigeschaltet/unbekannt
 */
function steam_stats_get_achievement_unlock_time(string $achievement_id): ?int {}

/**
 * Gibt die Gesamtzahl der im Backend konfigurierten Achievements zurück.
 * In Kombination mit steam_stats_get_achievement_name() zum Enumerieren.
 *
 * @return int|false Anzahl, false bei Fehler
 */
function steam_stats_get_num_achievements(): int|false {}

/**
 * Gibt die interne API-ID eines Achievements über seinen Index zurück.
 *
 * @param int $index 0-basierter Index (< steam_stats_get_num_achievements())
 * @return string|false Achievement-API-ID, false bei ungültigem Index
 */
function steam_stats_get_achievement_name(int $index): string|false {}

/**
 * Liest ein Anzeige-Attribut eines Achievements aus dem Backend.
 *
 * @param string $achievement_id Achievement-ID
 * @param string $key "name" (Anzeigename), "desc" (Beschreibung) oder "hidden" ("0"/"1")
 * @return string|false Attributwert (leerer String bei unbekanntem Key), false bei Fehler
 */
function steam_stats_get_achievement_display_attribute(string $achievement_id, string $key): string|false {}

/**
 * Setzt alle Stats (und optional Achievements) des Nutzers zurück.
 * Vorsicht: irreversibel für den Nutzer. Primär zum Testen gedacht.
 *
 * @param bool $achievements_too true, um auch Achievements zurückzusetzen
 * @return bool true bei Erfolg
 */
function steam_stats_reset_all_stats(bool $achievements_too = false): bool {}

/* ── Leaderboards (asynchron via Handle + Poll) ── */

/**
 * Sucht ein Leaderboard anhand seines Namens (asynchron).
 * Gibt ein Call-Handle zurück; das Ergebnis wird über steam_get_call_result()
 * abgeholt (Typ "leaderboard_found").
 *
 * @param string $name Leaderboard-Name aus dem Steamworks Partner-Backend
 * @return int|false Call-Handle, false bei Fehler
 */
function steam_stats_find_leaderboard(string $name): int|false {}

/**
 * Sucht ein Leaderboard oder erstellt es, falls es nicht existiert (asynchron).
 *
 * @param string $name Leaderboard-Name
 * @param int $sort_method STEAM_LEADERBOARD_SORT_ASCENDING|STEAM_LEADERBOARD_SORT_DESCENDING
 * @param int $display_type STEAM_LEADERBOARD_DISPLAY_NUMERIC|_TIME_SECONDS|_TIME_MILLISECONDS
 * @return int|false Call-Handle, false bei Fehler
 */
function steam_stats_find_or_create_leaderboard(string $name, int $sort_method, int $display_type): int|false {}

/**
 * Lädt einen Score in ein Leaderboard hoch (asynchron).
 * Ergebnis über steam_get_call_result() (Typ "score_uploaded").
 *
 * @param int $leaderboard Leaderboard-Handle (aus dem find-Ergebnis)
 * @param int $score Der hochzuladende Score
 * @param int $method STEAM_LEADERBOARD_UPLOAD_KEEP_BEST (Default) oder _FORCE_UPDATE
 * @param list<int>|null $details Optionale spielspezifische int32-Detailwerte (z. B. Split-Zeiten),
 *                                die mit dem Eintrag gespeichert und beim Download wieder ausgelesen
 *                                werden. Maximal STEAM_LEADERBOARD_DETAILS_MAX (64); überzählige
 *                                werden verworfen.
 * @return int|false Call-Handle, false bei Fehler
 */
function steam_stats_upload_score(int $leaderboard, int $score, int $method = STEAM_LEADERBOARD_UPLOAD_KEEP_BEST, ?array $details = null): int|false {}

/**
 * Lädt Leaderboard-Einträge herunter (asynchron).
 * Ergebnis über steam_get_call_result() (Typ "scores_downloaded"); die einzelnen
 * Einträge dann über steam_stats_get_downloaded_entry().
 *
 * @param int $leaderboard Leaderboard-Handle
 * @param int $request STEAM_LEADERBOARD_DATA_GLOBAL|_GLOBAL_AROUND_USER|_FRIENDS
 * @param int $range_start 1-basierter Startindex
 * @param int $range_end 1-basierter Endindex
 * @return int|false Call-Handle, false bei Fehler
 */
function steam_stats_download_leaderboard_entries(int $leaderboard, int $request, int $range_start, int $range_end): int|false {}

/**
 * Liest einen einzelnen heruntergeladenen Leaderboard-Eintrag (synchron).
 * Erst nach erfolgreichem steam_stats_download_leaderboard_entries()-Poll gültig.
 *
 * @param int $entries Entries-Handle aus dem "scores_downloaded"-Ergebnis
 * @param int $index 0-basierter Index innerhalb des heruntergeladenen Bereichs
 * @return array{steam_id:int, global_rank:int, score:int, details:list<int>}|null
 *         `details` enthält die beim Upload mitgegebenen int32-Werte (leeres Array, wenn keine).
 */
function steam_stats_get_downloaded_entry(int $entries, int $index): ?array {}

/**
 * Gibt die Gesamtzahl der Einträge eines Leaderboards zurück (synchron).
 *
 * @param int $leaderboard Leaderboard-Handle
 * @return int Anzahl der Einträge
 */
function steam_stats_get_leaderboard_entry_count(int $leaderboard): int {}

/* ── steam_async.c ── */

/**
 * Holt das Ergebnis eines asynchronen Steam-Calls ab (Poll).
 * Jeden Frame aufrufen, bis nicht mehr null zurückkommt:
 *   - null  → noch nicht fertig (erneut pollen) oder unbekanntes/verbrauchtes Handle
 *   - false → Call ist fehlgeschlagen
 *   - array → Ergebnis mit 'type'-Feld ("leaderboard_found", "score_uploaded",
 *             "scores_downloaded", "timeline_event_recording_exists",
 *             "timeline_game_phase_recording_exists"); das Handle ist danach verbraucht.
 *
 * @param int $handle Call-Handle einer asynchronen steam_*-Funktion
 * @return array|false|null
 */
function steam_get_call_result(int $handle): array|false|null {}

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

/**
 * Prüft, ob der Nutzer eine bestimmte App/DLC besitzt.
 *
 * @param int $app_id App- oder DLC-ID
 * @return bool true wenn abonniert/im Besitz
 */
function steam_apps_is_subscribed_app(int $app_id): bool {}

/**
 * Gibt den Namen der aktiven Beta-Branch zurück (falls das Spiel auf einer läuft).
 *
 * @return string|false Beta-Branch-Name, false wenn keine Beta aktiv
 */
function steam_apps_get_current_beta_name(): string|false {}

/**
 * Gibt den frühesten Kaufzeitpunkt einer App als Unix-Timestamp zurück.
 *
 * @param int $app_id App-ID
 * @return int|false Unix-Timestamp, false bei Fehler
 */
function steam_apps_get_earliest_purchase_time(int $app_id): int|false {}

/**
 * Gibt die IDs der installierten Depots einer App zurück.
 *
 * @param int $app_id App-ID
 * @return int[]|false Liste der Depot-IDs, false bei Fehler
 */
function steam_apps_get_installed_depots(int $app_id): array|false {}

/**
 * Gibt die Anzahl der DLCs zurück, die für die aktuelle App registriert sind.
 *
 * @return int|false Anzahl der DLCs, false bei Fehler
 */
function steam_apps_get_dlc_count(): int|false {}

/**
 * Gibt die Build-ID der aktuell installierten App zurück.
 *
 * @return int|false Build-ID, false bei Fehler
 */
function steam_apps_get_app_build_id(): int|false {}

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

/**
 * Prüft, ob das Spiel gerade auf einem Steam Deck läuft.
 * Nützlich um handheld-freundliche Defaults zu setzen (Fullscreen, größerer UI-Scale).
 *
 * @return bool true wenn auf Steam Deck
 */
function steam_utils_is_steam_deck(): bool {}

/**
 * Gibt die Sprache der Steam-Client-UI zurück (kann von der Spielsprache abweichen).
 *
 * @return string|false Sprachcode (z.B. "english"), false bei Fehler
 */
function steam_utils_get_steam_ui_language(): string|false {}

/**
 * Gibt die aktuelle Steam-Server-Zeit als Unix-Timestamp zurück.
 *
 * @return int|false Unix-Timestamp, false bei Fehler
 */
function steam_utils_get_server_real_time(): int|false {}

/**
 * Gibt den Akkustand zurück: 0–100 = Prozent, 255 = am Netzteil.
 * Nützlich, um auf Handhelds (Steam Deck) den Stromverbrauch anzupassen.
 *
 * @return int|false Akkustand, false bei Fehler
 */
function steam_utils_get_current_battery_power(): int|false {}

/**
 * Gibt die Sekunden seit der letzten Nutzer-Aktivität in der App zurück.
 *
 * @return int|false Sekunden, false bei Fehler
 */
function steam_utils_get_seconds_since_app_active(): int|false {}

/* ── steam_timeline.c (ISteamTimeline / Game Recording) ──
 *
 * Annotiert die Steam-Game-Recording-Timeline. Die meisten Funktionen sind
 * "fire and forget" (true bei Erfolg, false wenn Steam nicht initialisiert).
 * Die beiden does_*_recording_exist-Funktionen sind asynchron und geben ein
 * Call-Handle zurück (via steam_get_call_result() pollen).
 */

/**
 * Setzt den aktuellen Spielmodus für die Timeline.
 *
 * @param int $mode STEAM_TIMELINE_GAME_MODE_PLAYING|_STAGING|_MENUS|_LOADING_SCREEN|_INVALID
 * @return bool true bei Erfolg, false wenn Steam nicht initialisiert
 */
function steam_timeline_set_game_mode(int $mode): bool {}

/**
 * Setzt die Tooltip-/State-Beschreibung für den aktuellen Timeline-Abschnitt.
 *
 * @param string $description Beschreibung (z.B. "Level 3 – Boss")
 * @param float $time_delta Zeitversatz in Sekunden relativ zu jetzt (0 = jetzt)
 * @return bool true bei Erfolg
 */
function steam_timeline_set_tooltip(string $description, float $time_delta = 0.0): bool {}

/**
 * Entfernt die Tooltip-/State-Beschreibung.
 *
 * @param float $time_delta Zeitversatz in Sekunden relativ zu jetzt
 * @return bool true bei Erfolg
 */
function steam_timeline_clear_tooltip(float $time_delta = 0.0): bool {}

/**
 * Fügt ein punktuelles (instantanes) Timeline-Event hinzu.
 *
 * @param string $title Kurztitel
 * @param string $description Beschreibung
 * @param string $icon Icon-Name (Steam-Timeline-Icon oder hochgeladenes Icon)
 * @param int $icon_priority Priorität für die Icon-Darstellung (höher = wichtiger)
 * @param float $start_offset_seconds Zeitversatz in Sekunden relativ zu jetzt
 * @param int $possible_clip STEAM_TIMELINE_CLIP_PRIORITY_NONE|_STANDARD|_FEATURED
 * @return int Event-Handle (0 bei Fehler / Steam nicht initialisiert)
 */
function steam_timeline_add_instantaneous_event(string $title, string $description, string $icon, int $icon_priority = 0, float $start_offset_seconds = 0.0, int $possible_clip = STEAM_TIMELINE_CLIP_PRIORITY_NONE): int {}

/**
 * Fügt ein Timeline-Event mit Dauer (Range) hinzu.
 *
 * @param string $title Kurztitel
 * @param string $description Beschreibung
 * @param string $icon Icon-Name
 * @param int $icon_priority Icon-Priorität
 * @param float $start_offset_seconds Startzeit-Versatz in Sekunden relativ zu jetzt
 * @param float $duration_seconds Dauer des Events in Sekunden
 * @param int $possible_clip STEAM_TIMELINE_CLIP_PRIORITY_*
 * @return int Event-Handle (0 bei Fehler)
 */
function steam_timeline_add_range_event(string $title, string $description, string $icon, int $icon_priority = 0, float $start_offset_seconds = 0.0, float $duration_seconds = 0.0, int $possible_clip = STEAM_TIMELINE_CLIP_PRIORITY_NONE): int {}

/**
 * Startet ein offenes Range-Event, das später mit steam_timeline_end_range_event()
 * abgeschlossen wird. Nützlich für Events unbekannter Dauer.
 *
 * @param string $title Kurztitel
 * @param string $description Beschreibung
 * @param string $icon Icon-Name
 * @param int $icon_priority Icon-Priorität
 * @param float $start_offset_seconds Startzeit-Versatz in Sekunden relativ zu jetzt
 * @param int $possible_clip STEAM_TIMELINE_CLIP_PRIORITY_*
 * @return int Event-Handle für update/end (0 bei Fehler)
 */
function steam_timeline_start_range_event(string $title, string $description, string $icon, int $icon_priority = 0, float $start_offset_seconds = 0.0, int $possible_clip = STEAM_TIMELINE_CLIP_PRIORITY_NONE): int {}

/**
 * Aktualisiert Titel/Beschreibung/Icon eines offenen Range-Events.
 *
 * @param int $event Event-Handle aus steam_timeline_start_range_event()
 * @param string $title Neuer Titel
 * @param string $description Neue Beschreibung
 * @param string $icon Neues Icon
 * @param int $icon_priority Icon-Priorität
 * @param int $possible_clip STEAM_TIMELINE_CLIP_PRIORITY_*
 * @return bool true bei Erfolg
 */
function steam_timeline_update_range_event(int $event, string $title, string $description, string $icon, int $icon_priority = 0, int $possible_clip = STEAM_TIMELINE_CLIP_PRIORITY_NONE): bool {}

/**
 * Schließt ein zuvor mit steam_timeline_start_range_event() gestartetes Event ab.
 *
 * @param int $event Event-Handle
 * @param float $end_offset_seconds Endzeit-Versatz in Sekunden relativ zu jetzt
 * @return bool true bei Erfolg
 */
function steam_timeline_end_range_event(int $event, float $end_offset_seconds = 0.0): bool {}

/**
 * Entfernt ein Timeline-Event wieder.
 *
 * @param int $event Event-Handle
 * @return bool true bei Erfolg
 */
function steam_timeline_remove_event(int $event): bool {}

/**
 * Prüft asynchron, ob für ein Event eine Aufnahme (Clip) existiert.
 * Ergebnis über steam_get_call_result() (Typ "timeline_event_recording_exists").
 *
 * @param int $event Event-Handle
 * @return int|false Call-Handle, false bei Fehler
 */
function steam_timeline_does_event_recording_exist(int $event): int|false {}

/**
 * Beginnt eine neue Game-Phase (gruppiert Gameplay für spätere Navigation).
 *
 * @return bool true bei Erfolg
 */
function steam_timeline_start_game_phase(): bool {}

/**
 * Beendet die aktuelle Game-Phase.
 *
 * @return bool true bei Erfolg
 */
function steam_timeline_end_game_phase(): bool {}

/**
 * Setzt eine ID für die aktuelle Game-Phase (zum späteren Wiederauffinden).
 *
 * @param string $phase_id Phase-ID (max STEAM_TIMELINE_PHASE_ID-Länge)
 * @return bool true bei Erfolg
 */
function steam_timeline_set_game_phase_id(string $phase_id): bool {}

/**
 * Prüft asynchron, ob für eine Game-Phase eine Aufnahme existiert.
 * Ergebnis über steam_get_call_result() (Typ "timeline_game_phase_recording_exists").
 *
 * @param string $phase_id Phase-ID
 * @return int|false Call-Handle, false bei Fehler
 */
function steam_timeline_does_game_phase_recording_exist(string $phase_id): int|false {}

/**
 * Hängt ein Tag an die aktuelle Game-Phase (z.B. "Boss: Drache").
 *
 * @param string $tag_name Tag-Name
 * @param string $tag_icon Icon-Name für das Tag
 * @param string $tag_group Gruppe, unter der das Tag einsortiert wird
 * @param int $priority Priorität für die Darstellung
 * @return bool true bei Erfolg
 */
function steam_timeline_add_game_phase_tag(string $tag_name, string $tag_icon, string $tag_group, int $priority = 0): bool {}

/**
 * Setzt ein Attribut (Key/Value) an der aktuellen Game-Phase.
 *
 * @param string $attribute_group Attribut-Gruppe (Key)
 * @param string $attribute_value Attribut-Wert
 * @param int $priority Priorität
 * @return bool true bei Erfolg
 */
function steam_timeline_set_game_phase_attribute(string $attribute_group, string $attribute_value, int $priority = 0): bool {}

/**
 * Öffnet das Steam-Overlay auf einer bestimmten Game-Phase.
 *
 * @param string $phase_id Phase-ID
 * @return bool true bei Erfolg
 */
function steam_timeline_open_overlay_to_game_phase(string $phase_id): bool {}

/**
 * Öffnet das Steam-Overlay auf einem bestimmten Timeline-Event.
 *
 * @param int $event Event-Handle
 * @return bool true bei Erfolg
 */
function steam_timeline_open_overlay_to_event(int $event): bool {}
