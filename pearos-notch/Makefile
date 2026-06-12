NPROC   := $(shell nproc)
MKPDIR  := $(CURDIR)/.makepkg

.PHONY: all build package clean

all: package

build:
	cmake -B build -S . -G Ninja \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX=/usr \
		-DWITH_LAYER_SHELL=ON
	cmake --build build -j$(NPROC)

package: build
	BUILDDIR=$(MKPDIR) PKGDEST=$(CURDIR) makepkg -f

clean:
	rm -rf build/ .makepkg/ *.pkg.tar.zst *.pkg.tar.xz
