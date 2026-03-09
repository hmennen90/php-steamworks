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
**Steamworks SDK:** 1.58+ (muss vom Nutzer selbst bereitgestellt werden)

---

## Verzeichnisstruktur

```
php-steamworks/
├── src/
│   ├── php_steamworks.c        ← Haupt-Extension-Datei, alle PHP_FUNCTION-Definitionen
│   ├── php_steamworks.h        ← Header: Makros, Funktions-Deklarationen, Structs
│   ├── modules/
│   │   ├── steam_init.c        ← SteamAPI_Init, SteamAPI_Shutdown, RunCallbacks
│   │   ├── steam_user.c        ← ISteamUser: SteamID, Auth Tickets
│   │   ├── steam_friends.c     ← ISteamFriends: Persona Name, Avatar, Rich Presence
│   │   ├── steam_stats.c       ← ISteamUserStats: Achievements, Stats, Leaderboards
│   │   ├── steam_remote.c      ← ISteamRemoteStorage: Cloud Saves
│   │   ├── steam_apps.c        ← ISteamApps: DLC, Beta, Language, AppID
│   │   ├── steam_utils.c       ← ISteamUtils: Overlay, AppID, Country, Language
│   │   └── steam_ugc.c         ← ISteamUGC: Workshop (Phase 2)
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
// RICHTIG — flache C-API
SteamAPI_ISteamFriends_GetPersonaName(SteamAPI_SteamFriends_v017());

// FALSCH — C++ direkt
SteamFriends()->GetPersonaName();
```

### 2. Jede Steamworks-Schnittstelle = ein Modul

Jedes `steam_*.c`-Modul kapselt genau eine Steamworks-Schnittstelle.
`php_steamworks.c` registriert alle Funktionen aus allen Modulen.

### 3. Fehlerbehandlung über PHP Warnings + bool Returns

Steam-Funktionen die fehlschlagen, geben `false` zurück und setzen ein
PHP-Warning mit `php_error_docref`. Keine Exceptions aus der Extension heraus.

```c
if (!SteamAPI_Init()) {
    php_error_docref(NULL, E_WARNING, "SteamAPI_Init failed. Is Steam running?");
    RETURN_FALSE;
}
RETURN_TRUE;
```

### 4. String-Ownership explizit behandeln

Steam gibt `const char*` zurück, die intern verwaltet werden. Immer mit
`RETURN_STRING()` kopieren, nie direkt übergeben.

```c
// RICHTIG
const char *name = SteamAPI_ISteamFriends_GetPersonaName(friends);
RETURN_STRING(name);  // Zend Engine kopiert den String

// FALSCH
RETURN_STRINGL(name, strlen(name), 0);  // ownership-Problem
```

### 5. Callbacks sind Pull, nicht Push

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
        SteamAPI_SteamApps_v008(), 0, depots, 64
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

### Phase 1 — Launch-kritisch
Diese Funktionen werden für jeden Steam-Release benötigt:

1. `steam_init()` — SteamAPI_Init
2. `steam_shutdown()` — SteamAPI_Shutdown
3. `steam_run_callbacks()` — jeden Frame
4. `steam_apps_get_app_id()` — App-Identifikation
5. `steam_friends_get_name()` — Spielername im UI
6. `steam_stats_set_achievement()` + `steam_stats_store()` — Achievements
7. `steam_remote_file_write()` + `steam_remote_file_read()` — Cloud Saves

### Phase 2 — Post-Launch
8. `steam_stats_get/set_int/float()` — Statistiken
9. `steam_stats_find_leaderboard()` + `steam_stats_upload_score()` — Leaderboards
10. `steam_friends_set_rich_presence()` — Rich Presence
11. `steam_friends_activate_overlay()` — Steam Overlay
12. `steam_apps_is_dlc_installed()` — DLC-Prüfung

### Phase 3 — Optional
13. ISteamUGC (Workshop) — nur wenn Mod-Support über Steam Workshop
14. ISteamNetworkingMessages — nur für Multiplayer

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
    src/modules/steam_init.c \
    src/modules/steam_user.c \
    src/modules/steam_friends.c \
    src/modules/steam_stats.c \
    src/modules/steam_remote.c \
    src/modules/steam_apps.c \
    src/modules/steam_utils.c,
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

## Wichtige Einschränkungen (nie vergessen)

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
2. PHP_FUNCTION in passendem `src/modules/steam_*.c` implementieren
3. Deklaration in `src/php_steamworks.h` hinzufügen
4. Registrierung in `src/php_steamworks.c` in der `steamworks_functions[]`-Tabelle
5. Stub in `stubs/steamworks.php` ergänzen
6. Test in `tests/` ergänzen
7. Eintrag in `CHANGELOG.md`