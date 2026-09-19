# php-steamworks — CLAUDE.md

Instruktionen für Claude Code. Dieses Dokument beschreibt Architektur,
Konventionen und Arbeitsweise für die `php-steamworks` PHP-Extension.

---

## Projektübersicht

`php-steamworks` ist eine native PHP-Extension (C), die die Steamworks SDK
von Valve als PHP-API verfügbar macht. Sie ist der offizielle Steam-Layer
für die PHP Game Engine und kann unabhängig davon in beliebigen PHP CLI-
Projekten verwendet werden.

**Sprachen:** C (Extension), PHP (Tests, Beispiele, Stubs)
**Zielplattformen:** macOS (arm64, x86_64), Linux (x86_64), Windows (x86_64)
**PHP-Mindestversion:** 8.1
**Steamworks SDK:** 1.65 (Runtime-Libs liegen im Repo, Header stellt der Nutzer bereit)

---

## Verzeichnisstruktur

```
php-steamworks/
├── src/
│   ├── php_steamworks.c        ← Haupt-Extension-Datei, alle PHP_FUNCTION-Definitionen
│   ├── php_steamworks.h        ← Header: Makros, Funktions-Deklarationen, Structs
│   ├── steam_api_c.h           ← C-taugliche Deklarationen der flachen Steam-API
│   ├── steam_iface.h           ← Interface-Accessoren (steamworks_utils() etc.)
│   ├── steam_iface.c           ← Laufzeit-Auflösung der Interface-Versionen
│   ├── modules/
│   │   ├── steam_init.c        ← SteamAPI_Init, SteamAPI_Shutdown, RunCallbacks
│   │   ├── steam_user.c        ← ISteamUser: SteamID, Auth Tickets
│   │   ├── steam_friends.c     ← ISteamFriends: Persona Name, Avatar, Rich Presence
│   │   ├── steam_stats.c       ← ISteamUserStats: Achievements, Stats, Leaderboards
│   │   ├── steam_remote.c      ← ISteamRemoteStorage: Cloud Saves
│   │   ├── steam_apps.c        ← ISteamApps: DLC, Beta, Language, AppID
│   │   ├── steam_utils.c       ← ISteamUtils: Overlay, AppID, Country, Language
│   │   ├── steam_async.c       ← Async CallResults (Handle+Poll): steam_get_call_result
│   │   ├── steam_timeline.c    ← ISteamTimeline: Game Recording, Events, Game Phases
│   │   ├── steam_ugc.c         ← ISteamUGC: Workshop (Consume-Pfad: subscribe/state/download)
│   │   ├── steam_callback.c    ← Allgemeine Callbacks (CCallbackBase-Fabrikation): Web-API-Ticket, Net-Events
│   │   ├── steam_net.c         ← ISteamNetworkingSockets: P2P (connect/send/receive/status)
│   │   ├── steam_matchmaking.c ← ISteamMatchmaking: Lobbys (erstellen/beitreten/Daten/Chat)
│   │   └── steam_mock_hooks.c  ← nur Mock-Build: steam_mock_fire_callback() löst Callbacks für Tests aus
├── stubs/
│   └── steamworks.php          ← PHP-Stubs für IDE-Autocompletion (kein Runtime-Code)
├── tests/
│   ├── bootstrap.php
│   ├── SteamInitTest.php
│   ├── SteamStatsTest.php
│   └── SteamRemoteTest.php
├── examples/
│   ├── basic_init.php          ← Minimales Init + Shutdown Beispiel
│   ├── achievements.php        ← Achievement unlock Workflow
│   └── cloud_save.php          ← Cloud Save read/write
├── sdk/                        ← .gitignore — Nutzer legt Steamworks SDK hier ab
│   └── .gitkeep
├── config.m4                   ← Unix Build-Konfiguration (phpize)
├── config.w32                  ← Windows Build-Konfiguration
├── php_steamworks.stub.php     ← Stub-Datei für php-src Stub-Generator
├── INSTALL.md                  ← Setup-Anleitung (SDK-Download, Build-Schritte)
├── CHANGELOG.md
├── LICENSE                     ← MIT
└── README.md
```

---

## Architekturprinzipien

### 1. Flache C-API, keine C++-Objekte in PHP

Die Steamworks SDK stellt neben der C++ OOP-API eine flache C-API bereit
(`steam_api.h`). **Immer die flache C-API verwenden.** Nie direkt
C++-Klassen (`ISteamFriends*` etc.) in Zend-API-Code einbetten.

```c
// RICHTIG — flache C-API (Interface über steam_iface.h, siehe Prinzip 2)
SteamAPI_ISteamFriends_GetPersonaName(steamworks_friends());

// FALSCH — C++ direkt
SteamFriends()->GetPersonaName();
```

### 2. Interfaces zur Laufzeit auflösen, nie über versionierte Accessoren

Die SDK bietet pro Interface einen versionierten Accessor
(`SteamAPI_SteamUtils_v011()`). Das ist ein **Link-Zeit-Symbol** — jede SDK, die
ein Interface hochzählt, zerbricht damit den Build. SDK 1.65 hat genau so
`SteamAPI_SteamUtils_v010` entfernt.

Deshalb: **niemals `SteamAPI_Steam*_vNNN()` aufrufen.** Stattdessen die
Accessoren aus `steam_iface.h` verwenden, die die Version zur Laufzeit als
String auflösen.

```c
// RICHTIG
ISteamUtils *utils = steamworks_utils();

// FALSCH — bricht beim nächsten SDK-Versionssprung
ISteamUtils *utils = SteamAPI_SteamUtils_v011();
```

Die Versionstabellen stehen in `src/steam_iface.c`, neueste Version zuerst.

**Versionsstrings sind nicht ableitbar.** `ISteamApps` heißt
`"STEAMAPPS_INTERFACE_VERSION009"`, nicht `"SteamApps009"`. Immer aus dem
`*_INTERFACE_VERSION`-Define des SDK-Headers kopieren, nie raten — ein falscher
String löst das Interface still nicht auf, und die Funktion gibt denselben
`false`-Wert zurück wie bei „Steam läuft nicht".

**Legacy-Fallbacks nur bei identischem vtable-Layout.** Einträge nach dem ersten
sind veraltete Versionen, die weiter funktionieren und einmalig pro Prozess
`E_DEPRECATED` auslösen. Das ist aber nur zulässig, wenn das alte Interface
dieselbe vtable-Reihenfolge hat: die flachen `SteamAPI_ISteamX_*`-Funktionen in
`libsteam_api` sind gegen die **aktuelle** Version kompiliert und rufen feste
Slots auf. Methoden am Ende anhängen ist unkritisch, Entfernen oder Umsortieren
nicht. Vor dem Eintragen prüfen:

```bash
diff <(grep -oE 'virtual [^(]+\(' alt/isteamfoo.h) \
     <(grep -oE 'virtual [^(]+\(' neu/isteamfoo.h)
```

`ISteamUtils` hat deshalb **keinen** Fallback: SDK 1.65 hat zwei Methoden aus der
Mitte entfernt. `ISteamNetworkingSockets012` → `013` ist layout-identisch und
daher sicher.

### 3. Jede Steamworks-Schnittstelle = ein Modul

Jedes `steam_*.c`-Modul kapselt genau eine Steamworks-Schnittstelle.
`php_steamworks.c` registriert alle Funktionen aus allen Modulen.

### 4. Fehlerbehandlung über PHP Warnings + bool Returns

Steam-Funktionen die fehlschlagen, geben `false` zurück und setzen ein
PHP-Warning mit `php_error_docref`. Keine Exceptions aus der Extension heraus.

```c
if (!SteamAPI_Init()) {
    php_error_docref(NULL, E_WARNING, "SteamAPI_Init failed. Is Steam running?");
    RETURN_FALSE;
}
RETURN_TRUE;
```

### 5. String-Ownership explizit behandeln

Steam gibt `const char*` zurück, die intern verwaltet werden. Immer mit
`RETURN_STRING()` kopieren, nie direkt übergeben.

```c
// RICHTIG
const char *name = SteamAPI_ISteamFriends_GetPersonaName(friends);
RETURN_STRING(name);  // Zend Engine kopiert den String

// FALSCH
RETURN_STRINGL(name, strlen(name), 0);  // ownership-Problem
```

### 6. Callbacks sind Pull, nicht Push

Steam Callbacks werden über `SteamAPI_RunCallbacks()` verarbeitet, das
**jeden Frame** aus dem PHP Game Loop aufgerufen werden muss. Kein
Threading, kein Background-Polling. PHP ist single-threaded, Steam Callbacks
folgen diesem Modell.

---

## Zend-API Patterns — Referenz

### Basis-Funktions-Template

```c
PHP_FUNCTION(steam_beispiel_funktion) {
    // 1. Parameter parsen
    zend_long   param_long;
    zend_string *param_str;
    zend_bool   param_bool;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(param_long)
        Z_PARAM_STR(param_str)
    ZEND_PARSE_PARAMETERS_END();

    // 2. Steam-Call
    bool result = SteamAPI_DoSomething((int)param_long, ZSTR_VAL(param_str));

    // 3. Return
    RETURN_BOOL(result);
}
```

### Array zurückgeben

```c
PHP_FUNCTION(steam_get_installed_depots) {
    array_init(return_value);

    DepotId_t depots[64];
    uint32 count = SteamAPI_ISteamApps_GetInstalledDepots(
        SteamAPI_SteamApps_v009(), 0, depots, 64
    );

    for (uint32 i = 0; i < count; i++) {
        add_next_index_long(return_value, (zend_long)depots[i]);
    }
    // Kein RETURN_* nötig — return_value wurde direkt befüllt
}
```

### Optional-Parameter mit Default

```c
PHP_FUNCTION(steam_set_rich_presence) {
    zend_string *key, *value = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STR(key)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(value)
    ZEND_PARSE_PARAMETERS_END();

    const char *val = value ? ZSTR_VAL(value) : "";
    RETURN_BOOL(SteamAPI_ISteamFriends_SetRichPresence(
        SteamAPI_SteamFriends_v017(), ZSTR_VAL(key), val
    ));
}
```

---

## Funktions-Namenskonvention

```
steam_{schnittstelle}_{aktion}

steam_init()                    ← SteamAPI_Init
steam_shutdown()                ← SteamAPI_Shutdown
steam_run_callbacks()           ← SteamAPI_RunCallbacks (jeden Frame!)

steam_user_get_steam_id()       ← ISteamUser
steam_user_get_auth_ticket()

steam_friends_get_name()        ← ISteamFriends
steam_friends_set_rich_presence()
steam_friends_activate_overlay()
steam_friends_activate_overlay_to_web_page()

steam_stats_set_achievement()   ← ISteamUserStats
steam_stats_clear_achievement()
steam_stats_store()
steam_stats_get_int()
steam_stats_set_int()
steam_stats_get_float()
steam_stats_set_float()
steam_stats_find_leaderboard()
steam_stats_upload_score()

steam_remote_file_write()       ← ISteamRemoteStorage
steam_remote_file_read()
steam_remote_file_exists()
steam_remote_file_delete()
steam_remote_file_list()

steam_apps_is_subscribed()      ← ISteamApps
steam_apps_is_dlc_installed()
steam_apps_get_app_id()
steam_apps_get_language()

steam_utils_get_app_id()        ← ISteamUtils
steam_utils_is_overlay_enabled()
steam_utils_get_country_code()
```

---

## Implementierungsreihenfolge (Priorität)

**Stand v0.11.0: Phase 1–3 im Kern vollständig (102 Funktionen).**
Zusätzlich umgesetzt: asynchrone CallResult-Infrastruktur (`steam_get_call_result()`,
`src/modules/steam_async.c`), Achievement-Lesepfad, erweiterte Apps/Utils/User-Getter,
Leaderboard-Score-Details (`int32[]`), ISteamTimeline (Game Recording),
ISteamFriends-Erweiterung (Freundesliste/Persona/Avatare), ISteamUser-Auth-Tickets
(Session + Web-API), ISteamUGC (Workshop-Consume-Pfad), ein allgemeines
Callback-Subsystem (`steam_callback.c`, CCallbackBase-Fabrikation) und
ISteamNetworkingSockets-P2P-Core (`steam_net.c`).
Alles gegen die echte Steamworks SDK 1.64 verifiziert **und live gegen echtes Steam getestet**
(Signaturen, Struct-Layouts, Callback-IDs, vtable-Dispatch). Offene Follow-ups:
UGC-Browsing (`SteamUGCDetails_t`), Netzwerk-Statistiken/Lanes.

### Phase 1 — Launch-kritisch ✅ erledigt
Diese Funktionen werden für jeden Steam-Release benötigt:

1. `steam_init()` — SteamAPI_Init
2. `steam_shutdown()` — SteamAPI_Shutdown
3. `steam_run_callbacks()` — jeden Frame
4. `steam_apps_get_app_id()` — App-Identifikation
5. `steam_friends_get_name()` — Spielername im UI
6. `steam_stats_set_achievement()` + `steam_stats_store()` — Achievements
7. `steam_remote_file_write()` + `steam_remote_file_read()` — Cloud Saves

### Phase 2 — Post-Launch ✅ erledigt
8. `steam_stats_get/set_int/float()` — Statistiken
9. `steam_stats_find_leaderboard()` + `steam_stats_upload_score()` — Leaderboards (async)
10. `steam_friends_set_rich_presence()` — Rich Presence
11. `steam_friends_activate_overlay()` — Steam Overlay
12. `steam_friends_activate_overlay_to_web_page()` — URL im Steam Overlay öffnen
13. `steam_apps_is_dlc_installed()` — DLC-Prüfung

### Phase 3 — Kern erledigt (alles live gegen echtes Steam verifiziert)
- **ISteamTimeline** (Game Recording) ✅ (`steam_timeline.c`, 18 Fkt., V004)
- **ISteamFriends erweitern** ✅ (`steam_friends.c`) — Freundesliste, Persona-State, Avatare (RGBA)
- **Auth-Tickets** ✅ (`steam_user.c` + `steam_callback.c`): Session-Tickets
  (`get_auth_session_ticket`, `begin/end_auth_session`, `cancel_auth_ticket`) **und**
  Web-API-Ticket (`get_auth_ticket_for_web_api` + `get_web_api_ticket_result`, callback-basiert)
- **ISteamUGC** (Workshop) ✅ Consume-Pfad (`steam_ugc.c`, V021). Offen: Browsing (`SteamUGCDetails_t`)
- **Callback-Subsystem** ✅ (`steam_callback.c`) — fabriziert CCallbackBase-kompatible Objekte und
  registriert sie via `SteamAPI_RegisterCallback`; koexistiert mit `RunCallbacks`, lässt den
  verifizierten CallResult-Pfad unangetastet. vtable-Dispatch live bestätigt
- **ISteamNetworkingSockets** ✅ P2P-Core (`steam_net.c`, v012): connect/listen/accept/send/
  receive/close + Connection-Status-Events (id 1221). Message-/Callback-Structs über
  SDK-1.64-verifizierte Byte-Offsets. Offen: Statistiken/Lanes, voller Accept-Flow

### Phase 4 — Multiplayer ohne eigenen Server ✅ umgesetzt in v0.16.0 (Live-Test offen)

Alle Teilphasen testgetrieben gebaut, jeder neue Struct gegen die SDK-Header unter
pack(8) **und** pack(4) vermessen (siehe „Struct-Layout prüfen"). Dabei gefunden und
behoben: die Offsets der Verbindungs-Events (`SteamNetConnectionStatusChangedCallback_t`)
waren nur für Windows richtig. Offen: Live-Test mit zwei Steam-Konten.

Ziel: Code Tycoon bekommt drei Mehrspieler-Modi, die nur Steams eigene
Infrastruktur nutzen — Leaderboards, Remote Storage, Lobbys, Relay-Netz. Kein
Server des Spiele-Studios. Die Callback-IDs unten sind gegen die SDK-Header
geprüft (1.64; die betroffenen Interfaces sind in 1.65 unverändert).

Querschnitt für alle Teilphasen:
- Workflow „Neue Funktionen" (unten) pro Funktion, inkl. Mock + CI-Liste.
- **Struct-Packing prüfen:** Callback-Structs sind auf Windows pack(8), auf
  Linux/macOS pack(4). Jeden neuen Struct in den `#pragma pack`-Block von
  `steam_api_c.h`, Offsets per C++-Probe gegen die echten Header bestätigen
  (Beispiel `LobbyCreated_t`: `m_ulSteamIDLobby` liegt bei 8 bzw. 4).
- Callbacks landen wie `net_events` in einer Warteschlange in
  `steam_callback.c`; PHP holt sie mit einer `*_get_*`-Funktion ab. Daten, die
  nur im Callback gültig sind (Lobby-Chat), dort kopieren.
- Neue Interfaces bekommen einen Eintrag in `steam_iface.c` (Versions-String aus
  dem SDK-Header kopieren) plus `mock_offered[]` und `SteamInterfaceTest`.

**4a — Anhänge an Leaderboards** (Modus „Wochen-Herausforderung", „geteilte Welt")
- `steam_remote_file_share($file)` → CallResult 1307 `RemoteStorageFileShareResult_t` (UGC-Handle)
- `steam_stats_attach_leaderboard_ugc($lb, $ugc)` → CallResult 1111 `LeaderboardUGCSet_t`
- `steam_remote_ugc_download($ugc, $priority = 0)` → CallResult 1317 `RemoteStorageDownloadUGCResult_t`
- `steam_remote_ugc_read($ugc, $size, $offset = 0)` — synchron, `UGCRead`
- `steam_remote_get_ugc_details($ugc)` — synchron, `GetUGCDetails`
- `steam_stats_get_downloaded_entry()` liefert zusätzlich `ugc` (`LeaderboardEntry_t::m_hUGC`)

**4b — Einladen und Beitreten über Rich Presence** (laufende Partie)
- `steam_friends_invite_user_to_game($friend, $connect)` — `InviteUserToGame`
- `steam_friends_activate_invite_dialog_connect_string($connect)` — `ActivateGameOverlayInviteDialogConnectString`
- `steam_friends_get_friend_rich_presence($friend, $key)` — `GetFriendRichPresence`
- `steam_friends_get_friend_game_played($friend)` — `GetFriendGamePlayed` (`FriendGameInfo_t`)
- `steam_friends_get_join_requests()` — Warteschlange aus Callback 337 `GameRichPresenceJoinRequested_t`
- `steam_apps_get_launch_command_line()` — `GetLaunchCommandLine` (Start per Einladung)

**4c — Lobbys** (neues Modul `steam_matchmaking.c`, `"SteamMatchMaking009"`)
- CallResults: `steam_matchmaking_create_lobby($type, $max)` → 513 `LobbyCreated_t`,
  `steam_matchmaking_join_lobby($lobby)` → 504 `LobbyEnter_t`
- Synchron: `leave_lobby`, `invite_user_to_lobby`, `get_num_lobby_members`,
  `get_lobby_member_by_index`, `get_lobby_owner`, `set_lobby_owner`,
  `get/set_lobby_data`, `get/set_lobby_member_data`, `set_lobby_joinable`,
  `set_lobby_type`, `send_lobby_chat_msg`; dazu `steam_friends_activate_overlay_invite_dialog($lobby)`
- `steam_matchmaking_get_events()` — Callbacks 333 `GameLobbyJoinRequested_t`,
  505 `LobbyDataUpdate_t`, 506 `LobbyChatUpdate_t`, 507 `LobbyChatMsg_t`
  (Chat-Text per `GetLobbyChatEntry` schon im Callback gelesen)

**4d — Netzwerk-Feinschliff** (optional)
- Poll-Groups: `create_poll_group`, `set_connection_poll_group`, `receive_messages_on_poll_group`
- `steam_net_get_connection_status($conn)` — `GetConnectionRealTimeStatus` (Ping, Qualität)
- Offener Punkt aus Phase 3 schließen: voller Accept-Flow

Abschluss jeder Teilphase: Live-Test mit zwei Steam-Konten (erst App 480, dann
die Spiel-App). Auslieferung: Tag → Runtime-Build in static-php-cli (zieht immer
den neuesten Tag) → Spiel.

### Zurückgestellt (dokumentiert)
- Score-`details`-Arrays (`int32[]`) bei Leaderboard-Upload/Download ✅ erledigt in v0.10.0
- Real-SDK-Verifikation: Leaderboard-Callback-IDs (1104–1106) ✅ gegen SDK 1.64 (v0.9.0);
  ISteamTimeline ✅ gegen SDK 1.64 (v0.10.0) — Accessor `SteamTimeline_v004`, alle Flat-Signaturen,
  Struct-Layouts und Callback-IDs (6000-Basis → 6001/6002) bestätigt. Die echte SDK liegt lokal
  unter `~/Downloads/steamworks_sdk_164/sdk`; ABI-Abweichungen (die der Mock nicht fängt) lassen
  sich per C++-Probe gegen die realen Header verifizieren.

---

## config.m4 — Build-Konfiguration

```m4
PHP_ARG_WITH([steamworks],
  [for Steamworks SDK support],
  [AS_HELP_STRING([--with-steamworks=DIR],
    [Include Steamworks SDK support. DIR = path to Steamworks SDK])])

if test "$PHP_STEAMWORKS" != "no"; then
  STEAM_SDK_DIR="$PHP_STEAMWORKS"
  if test -z "$STEAM_SDK_DIR"; then
    STEAM_SDK_DIR="$PWD/sdk"
  fi

  if ! test -f "$STEAM_SDK_DIR/public/steam/steam_api.h"; then
    AC_MSG_ERROR([Steamworks SDK not found. Download from partner.steamgames.com])
  fi

  PHP_ADD_INCLUDE($STEAM_SDK_DIR/public)

  case $host_os in
    darwin*)
      PHP_ADD_LIBRARY_WITH_PATH(steam_api, $STEAM_SDK_DIR/redistributable_bin/osx, STEAMWORKS_SHARED_LIBADD)
      ;;
    linux*)
      PHP_ADD_LIBRARY_WITH_PATH(steam_api, $STEAM_SDK_DIR/redistributable_bin/linux64, STEAMWORKS_SHARED_LIBADD)
      ;;
  esac

  PHP_SUBST(STEAMWORKS_SHARED_LIBADD)
  PHP_NEW_EXTENSION(steamworks,
    src/php_steamworks.c \
    src/steam_iface.c \
    src/modules/steam_init.c \
    src/modules/steam_user.c \
    src/modules/steam_friends.c \
    src/modules/steam_stats.c \
    src/modules/steam_async.c \
    src/modules/steam_remote.c \
    src/modules/steam_apps.c \
    src/modules/steam_utils.c \
    src/modules/steam_timeline.c \
    src/modules/steam_ugc.c \
    src/modules/steam_callback.c \
    src/modules/steam_net.c \
    src/modules/steam_matchmaking.c \
    src/modules/steam_mock_hooks.c,
    $ext_shared)
fi
```

---

## PHP Stubs (IDE-Support)

Für jeden implementierten PHP_FUNCTION-Eintrag muss ein entsprechender
Stub in `stubs/steamworks.php` existieren. Stubs haben keine Implementierung —
sie dienen nur der IDE-Autocompletion und dem Stub-Generator.

```php
<?php
// stubs/steamworks.php
// AUTO-GENERATED — nicht manuell bearbeiten

/**
 * Initialisiert die Steam API.
 * Muss vor allen anderen steam_* Funktionen aufgerufen werden.
 * Steam muss laufen, steam_appid.txt muss im Arbeitsverzeichnis liegen.
 *
 * @return bool true bei Erfolg, false wenn Steam nicht läuft
 */
function steam_init(): bool {}

/**
 * Verarbeitet ausstehende Steam Callbacks.
 * MUSS jeden Frame aufgerufen werden.
 *
 * @return void
 */
function steam_run_callbacks(): void {}

/**
 * Schaltet ein Achievement frei und persistiert es sofort.
 *
 * @param string $achievement_id Achievement-ID aus dem Steamworks Partner-Backend
 * @return bool true bei Erfolg
 */
function steam_stats_set_achievement(string $achievement_id): bool {}
```

---

## Tests

Tests laufen gegen eine gemockte Steam-Umgebung. Da Steam nicht im CI
verfügbar ist, testen wir primär:

1. **Extension lädt** — `extension_loaded('steamworks')` = true
2. **Funktionen existieren** — `function_exists('steam_init')` für alle registrierten Funktionen
3. **Parameter-Validierung** — falsche Typen lösen TypeError aus
4. **Graceful Failure** — `steam_init()` gibt `false` zurück wenn Steam nicht läuft
   (kein Fatal Error, kein Crash)

```php
// tests/SteamInitTest.php
class SteamInitTest extends PHPUnit\Framework\TestCase {
    public function testExtensionLoaded(): void {
        $this->assertTrue(extension_loaded('steamworks'));
    }

    public function testInitReturnsFalseWithoutSteam(): void {
        // Im CI läuft kein Steam — false ist der korrekte Return
        $result = steam_init();
        $this->assertIsBool($result);
    }

    public function testRunCallbacksExistsAsFunction(): void {
        $this->assertTrue(function_exists('steam_run_callbacks'));
    }
}
```

---

## Callbacks testen (Mock-Hook)

Der Mock liefert von sich aus keine Callbacks. Im Mock-Build (`--enable-steamworks-mock`)
gibt es deshalb `steam_mock_fire_callback(string $name, array $args)`
(`src/modules/steam_mock_hooks.c`, komplett in `#ifdef STEAMWORKS_MOCK`, in Release-Builds
nicht vorhanden). Er registriert die Callbacks und lässt den Mock den Struct im
SDK-Layout bauen und über die `CCallbackBase`-vtable ausliefern — der Test läuft also
durch den echten Code in `steam_callback.c`.

- Neuer Callback: `SteamMock_Fire*()` in `ci/mock_sdk/steam_api_mock.c` + Prototyp im
  Mock-Header + Zweig in `steam_mock_hooks.c`.
- Tests überspringen, wenn `function_exists('steam_mock_fire_callback')` false ist.
- Layouts, die vom Packing abhängen, baut der Mock mit **eigenen** gemessenen
  Offsets, nie mit denen aus `src/steam_api_c.h` — sonst kann der Test einen falschen
  Offset nicht finden.

## Struct-Layout prüfen (beide Packings, unter Windows)

Die SDK wählt das Callback-Packing allein über `__linux__`/`__APPLE__`
(`steamclientpublic.h`). Ein Messprogramm mit MSVC gegen die echten Header, einmal normal
und einmal mit `/D__linux__` gebaut, liefert damit die Layouts für Windows (pack 8) und
Linux/macOS (pack 4). Dasselbe Programm gegen `src/steam_api_c.h` gebaut muss identische
`sizeof`/`offsetof`-Werte ausgeben. Vor jedem neuen Callback- oder CallResult-Struct.

## SDK-Upgrade

1. Interface-Versionsstrings beider SDKs vergleichen:
   ```bash
   grep -rhoE '#define STEAM[A-Z_]*_INTERFACE_VERSION[ \t]+"[A-Za-z0-9_]+"' <sdk>/public/steam/*.h | sort -u
   ```
2. Für jede geänderte Version die Tabelle in `src/steam_iface.c` anpassen.
   Alte Version nur dann als Fallback stehen lassen, wenn die vtable-Reihenfolge
   identisch ist (siehe Architekturprinzip 2) — sonst ersatzlos streichen.
3. `mock_offered[]` / `mock_legacy[]` in `ci/mock_sdk/steam_api_mock.c` nachziehen
   und `EXPECTED_VERSIONS` / `LEGACY_VERSIONS` in `tests/SteamInterfaceTest.php`.
4. Genutzte Symbole gegen die neue SDK prüfen — entfernte Symbole finden:
   ```bash
   grep -oE "SteamAPI_[A-Za-z0-9_]+" src/steam_api_c.h | sort -u | while read s; do
     grep -q "\b$s\b" <sdk>/public/steam/steam_api_flat.h || echo "fehlt: $s"
   done
   ```
5. `sdk/redistributable_bin/**` durch die neuen Runtime-Libs ersetzen.
6. Verifizieren: `STEAMWORKS_SDK_DIR=<sdk> phpunit --filter testVersionStringsMatchARealSdk`
   und gegen die echte SDK bauen.
7. README („SDK compatibility"), INSTALL.md, CHANGELOG und `steamworks-docs-keeper` laufen lassen.

## Wichtige Einschränkungen (nie vergessen)

- **Clean-Targets zerstören getrackte Dateien:**
  - `make clean` löscht rekursiv **alle** `*.so` im Baum — also auch die
    ausgelieferten Runtime-Libraries unter
    `sdk/redistributable_bin/{linux32,linux64,linuxarm64,androidarm64}/`.
  - `phpize --clean` löscht zusätzlich das `tests/`-Verzeichnis.

  Stattdessen Objekte gezielt entfernen:
  `find src ci -name '*.lo' -delete && rm -rf src/.libs src/modules/.libs ci/mock_sdk/.libs modules`
  Nach jedem Clean `git status` prüfen.
- **`config.m4` geändert? → `phpize` erneut ausführen.** `./configure` allein
  regeneriert das Makefile nicht, die neue Quelldatei wird still nicht gebaut.
  Auf macOS fällt das nicht beim Linken auf (`-undefined suppress`), sondern
  erst als SIGSEGV zur Laufzeit.

- **`sdk/`-Ordner gehört nicht ins Repo.** `.gitignore` enthält `sdk/` komplett.
  Die Steamworks SDK darf nicht redistribuiert werden (Valve-Lizenz).
- **`libsteam_api` gehört nicht ins Binary.** Wird als `.so/.dylib/.dll` neben
  der Extension ausgeliefert. Steam installiert sie selbst auf Nutzerrechnern.
- **Kein Threading.** `steam_run_callbacks()` ist nicht thread-safe. Immer
  aus dem Haupt-Game-Loop aufrufen.
- **steam_appid.txt.** Für lokales Testen muss eine Datei `steam_appid.txt`
  mit der App-ID im Arbeitsverzeichnis liegen. Für Tests: `480` (Spacewar,
  Valves Test-App).
- **Keine Exceptions.** Die Extension wirft keine PHP-Exceptions. Fehler
  werden als `E_WARNING` + `false`-Return kommuniziert.

---

## Verbindung zur PHP Game Engine

In der PHP Game Engine wird `php-steamworks` über den `SteamManager` eingebunden:

```
engine/src/Platform/SteamManager.php   ← PHP-Wrapper um die Extension
engine/src/Platform/NullSteamManager.php ← Stub wenn Steam nicht verfügbar
```

Die Engine verwendet das Interface-Pattern:

```php
interface PlatformInterface {
    public function init(): bool;
    public function tick(): void;
    public function unlockAchievement(string $id): bool;
    public function getPlayerName(): string;
    public function saveToCloud(string $filename, string $data): bool;
    public function loadFromCloud(string $filename): ?string;
}
```

`SteamManager` implementiert `PlatformInterface` mit echten Steam-Calls.
`NullSteamManager` implementiert sie als No-Ops für Builds ohne Steam.

---

## Workflow für neue Funktionen

1. Steam-Funktionsname in `sdk/public/steam/steam_api_flat.h` nachschlagen
2. Deklaration in `src/steam_api_c.h` hinzufügen (flache C-API)
3. PHP_FUNCTION in passendem `src/modules/steam_*.c` implementieren
4. Deklaration in `src/php_steamworks.h` hinzufügen
5. Registrierung in `src/php_steamworks.c` in der `steamworks_functions[]`-Tabelle (arginfo + PHP_FE)
6. Stub in `stubs/steamworks.php` ergänzen
7. Test in `tests/` ergänzen
8. **Mock SDK aktualisieren** — CI baut gegen den Mock SDK, nicht die echte SDK:
   - `ci/mock_sdk/public/steam/steam_api_flat.h` — Funktionsdeklaration
   - `ci/mock_sdk/steam_api_mock.c` — No-Op-Implementierung
9. **CI-Funktionsliste aktualisieren** — Neue Funktion in die `$functions`-Arrays
   in `.github/workflows/ci.yml` eintragen (Unix- und Windows-Job)
10. Eintrag in `CHANGELOG.md`
11. **`steamworks-docs-keeper`-Agent laufen lassen** — prüft Stubs, README-
    Funktionsliste, beide CI-Arrays, Mock SDK, SDK-Version und Build-Config auf
    Drift. Doku-Drift ist hier still: kein Build und kein Test schlägt fehl,
    wenn ein Stub oder ein README-Eintrag fehlt.