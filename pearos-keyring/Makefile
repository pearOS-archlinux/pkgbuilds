PKG_NAME = pearos-keyring
PKGVER := $(shell awk -F= '/^pkgver=/{print $$2}' PKGBUILD)
PKGREL := $(shell awk -F= '/^pkgrel=/{print $$2}' PKGBUILD)

.PHONY: all clean build sign update-sums

all: update-sums build sign

update-sums:
	updpkgsums

build: update-sums
	makepkg -fsc

sign: build
	@echo "Semnez pachetul construit..."
	gpg --detach-sign --armor $(PKG_NAME)-$(PKGVER)-$(PKGREL)-any.pkg.tar.zst

clean:
	rm -rf pkg/ src/ *.pkg.tar.zst *.pkg.tar.zst.sig
