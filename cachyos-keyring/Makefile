PKG_NAME = cachyos-keyring
PKGVER := $(shell awk -F= '/^pkgver=/{print $$2}' PKGBUILD)
PKGREL := $(shell awk -F= '/^pkgrel=/{print $$2}' PKGBUILD)

.PHONY: all clean build sign update-sums

all: build

update-sums:
	updpkgsums

build:
	makepkg -fsc

clean:
	rm -rf pkg/ src/ *.pkg.tar.zst *.pkg.tar.zst.sig
