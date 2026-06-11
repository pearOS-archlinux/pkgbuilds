#!/bin/bash
set -e

LOCKSCREEN=/usr/share/plasma/shells/org.kde.plasma.desktop/contents/lockscreen
THEME=/usr/share/pearos/lockscreen

cp "$THEME/LockScreenUi.qml" "$LOCKSCREEN/LockScreenUi.qml"

mkdir -p "$LOCKSCREEN/fonts"
cp "$THEME/fonts/"*.otf "$LOCKSCREEN/fonts/"

mkdir -p "$LOCKSCREEN/images"
cp "$THEME/images/"*.svg "$LOCKSCREEN/images/"

fc-cache -f /usr/share/fonts/pearos/
