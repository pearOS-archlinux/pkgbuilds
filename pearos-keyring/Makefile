# Configurații
KEY_ID = 4C1A9F3C131ACA95
PKG_NAME = pearos-keyring
VERSION = 26.3

.PHONY: all clean build sign update-sums

all: update-sums build sign

pearos.gpg:
	gpg --export $(KEY_ID) > pearos.gpg

update-sums: pearos.gpg
	updpkgsums

build: update-sums
	makepkg -fsc

sign: build
	@echo "Semnez pachetul construit..."
	gpg --detach-sign --armor $(PKG_NAME)-$(VERSION)-1-any.pkg.tar.zst

clean:
	rm -rf pkg/ src/ *.pkg.tar.zst *.pkg.tar.zst.sig pearos.gpg
