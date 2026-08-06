PKG_NAME = chwd-db
PKGVER := $(shell awk -F= '/^pkgver=/{print $$2}' PKGBUILD)
PKGREL := $(shell awk -F= '/^pkgrel=/{print $$2}' PKGBUILD)

.PHONY: all clean build sign

all: build

build:
	makepkg -f

sign: build
	gpg --detach-sign --armor $(PKG_NAME)-$(PKGVER)-$(PKGREL)-any.pkg.tar.zst

clean:
	rm -rf pkg/ src/ *.pkg.tar.zst *.pkg.tar.zst.sig
