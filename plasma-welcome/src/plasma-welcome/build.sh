#!/bin/bash
# Build pearOS welcome (C++/Qt6). Produces dist/pearos-welcome for packaging/install.
set -e

echo "Building pearOS welcome application..."

mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
make -j$(nproc)
cd ..

mkdir -p dist
cp build/pearos-welcome dist/pearos-welcome
cp styles.qss dist/
cp -r assets dist/
cp list_windows.js dist/

echo ""
echo "Build complete! Executable: dist/pearos-welcome"
echo "Install: make install   or   makepkg && sudo pacman -U *.pkg.tar.zst"
