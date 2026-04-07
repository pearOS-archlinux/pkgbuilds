#!/usr/bin/env bash
set -e

cd "$(dirname "$0")"

OUT_BIN="pearos-notch"
SRC="main.cpp"

mkdir -p build

echo "Compiling $SRC -> build/$OUT_BIN"

g++ -std=c++17 -O2 -Wall -Wextra -o "build/$OUT_BIN" "$SRC" \
  $(pkg-config --cflags --libs x11 cairo librsvg-2.0 gdk-pixbuf-2.0 gio-2.0 glib-2.0 libpulse) \
  || { echo "Build failed"; exit 1; }

echo "Build completed: ./build/$OUT_BIN"

cp PKGBUILD build/
echo "PKGBUILD copied in ./build/"

for f in main.cpp notch.svg prev.svg next.svg play.svg pause.svg settings.svg pearos-notch.desktop pearos-notch.png pearos-notch.install; do
  if [ -f "$f" ]; then
    cp "$f" build/
  fi
done
echo "Required sources for PKGBUILD have been copied in ./build/."

if [ -f build/Makefile ]; then
  echo "Makefile already exists in ./build/."
elif [ -f Makefile ]; then
  cp Makefile build/
  echo "Makefile copied in ./build/."
else
  cat > build/Makefile << 'EOF'
.PHONY: pkg clean

pkg:
	@echo "Creating Arch package with makepkg..."
	@makepkg -sf

clean:
	@echo "Deleting build/ directory..."
	@cd .. && rm -rf build
EOF
  echo "Makefile generated in ./build/."
fi

