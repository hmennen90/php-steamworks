PHP_ARG_WITH([steamworks],
  [for Steamworks SDK support],
  [AS_HELP_STRING([--with-steamworks=DIR],
    [Include Steamworks SDK support. DIR = path to Steamworks SDK])])

PHP_ARG_ENABLE([steamworks-mock],
  [whether to use mock Steam API for testing],
  [AS_HELP_STRING([--enable-steamworks-mock],
    [Use mock Steam API instead of real SDK (for CI testing)])],
  [no], [no])

if test "$PHP_STEAMWORKS" != "no"; then
  STEAM_SDK_DIR="$PHP_STEAMWORKS"
  if test -z "$STEAM_SDK_DIR" || test "$STEAM_SDK_DIR" = "yes"; then
    STEAM_SDK_DIR="$PWD/sdk"
  fi

  STEAMWORKS_SOURCES="src/php_steamworks.c \
    src/modules/steam_init.c \
    src/modules/steam_user.c \
    src/modules/steam_friends.c \
    src/modules/steam_stats.c \
    src/modules/steam_remote.c \
    src/modules/steam_apps.c \
    src/modules/steam_utils.c"

  if test "$PHP_STEAMWORKS_MOCK" = "yes"; then
    dnl CI mock mode — use mock SDK headers and compiled mock implementation
    MOCK_DIR="$PWD/ci/mock_sdk"
    if ! test -f "$MOCK_DIR/public/steam/steam_api.h"; then
      AC_MSG_ERROR([Mock SDK not found in ci/mock_sdk/])
    fi
    PHP_ADD_INCLUDE($MOCK_DIR/public)
    STEAMWORKS_SOURCES="$STEAMWORKS_SOURCES ci/mock_sdk/steam_api_mock.c"
    AC_DEFINE(STEAMWORKS_MOCK, 1, [Using mock Steam API])
  else
    dnl Real SDK mode
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
  fi

  PHP_NEW_EXTENSION(steamworks, $STEAMWORKS_SOURCES, $ext_shared)
fi
