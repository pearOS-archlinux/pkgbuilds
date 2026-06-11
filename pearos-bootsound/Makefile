PKGNAME = pearos-bootsound
PKGVER = 26.6
PKGEXT = .pkg.tar.zst

.PHONY: all clean

all:
	makepkg -s --noconfirm

clean:
	rm -f *.pkg.tar.zst
	rm -rf pkg/
	rm -rf src/
	rm -rf pearos-boot-sound/
