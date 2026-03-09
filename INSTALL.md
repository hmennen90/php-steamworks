# Installation

## 1. Steamworks SDK herunterladen

1. Auf [partner.steamgames.com](https://partner.steamgames.com/) einloggen
2. SDK unter **Documentation > Steamworks SDK** herunterladen
3. In den `sdk/`-Ordner im Projektverzeichnis entpacken

Die Struktur sollte so aussehen:
```
sdk/
├── public/
│   └── steam/
│       ├── steam_api.h
│       ├── steam_api_flat.h
│       └── ...
├── redistributable_bin/
│   ├── osx/
│   │   └── libsteam_api.dylib
│   ├── linux64/
│   │   └── libsteam_api.so
│   └── win64/
│       ├── steam_api64.dll
│       └── steam_api64.lib
└── ...
```

## 2. Build (Unix: macOS / Linux)

```bash
cd php-steamworks
phpize
./configure --with-steamworks=./sdk
make
sudo make install
```

Die Extension wird als `steamworks.so` in das PHP Extensions-Verzeichnis installiert.

## 3. Build (Windows)

Voraussetzung: PHP SDK + Visual Studio Build Tools.

```cmd
cd php-steamworks
phpize
configure --with-steamworks=sdk
nmake
```

## 4. PHP-Konfiguration

In `php.ini` hinzufügen:

```ini
extension=steamworks
```

Oder per CLI:

```bash
php -dextension=steamworks.so your_script.php
```

## 5. Testen

Erstelle eine `steam_appid.txt` im Arbeitsverzeichnis mit der App-ID (z.B. `480` für Spacewar):

```bash
echo "480" > steam_appid.txt
```

Teste:

```bash
php -dextension=steamworks.so -r "var_dump(steam_init());"
```

**Hinweis:** `steam_init()` gibt nur `true` zurück, wenn Steam im Hintergrund läuft.

## Runtime-Abhängigkeit

Die `libsteam_api` Shared Library muss zur Laufzeit auffindbar sein:

- **macOS:** `libsteam_api.dylib` neben dem PHP-Binary oder in `DYLD_LIBRARY_PATH`
- **Linux:** `libsteam_api.so` neben dem Binary oder in `LD_LIBRARY_PATH`
- **Windows:** `steam_api64.dll` neben dem Binary oder in `PATH`

Steam installiert diese Library automatisch bei Spielern, die das Spiel über Steam starten.
