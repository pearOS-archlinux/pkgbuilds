.PHONY: all clean dev-build dev-install

all:
	makepkg -sf

clean:
	rm -rf build/ pkg/ src/ *.zst *.sig

dev-build:
	mkdir -p build
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
	cmake --build build --parallel

dev-install:
	cmake --install build
