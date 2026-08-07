#!/bin/sh
# Build, install and enable the effect, then reload KWin's config.
set -e

cd "$(dirname "$0")"

cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build -j"$(nproc)"
sudo cmake --install build

kwriteconfig6 --file kwinrc --group Plugins --key kwin4_effect_pearostinterEnabled true
qdbus org.kde.KWin /KWin org.kde.KWin.reconfigure

echo "pearos-tinter installed and enabled."
