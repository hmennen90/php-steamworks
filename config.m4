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
