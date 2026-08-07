PKGNAME := pearos-window-borders

.PHONY: all package install uninstall clean

all: package

## build the package with makepkg, pulling in missing dependencies
package:
	makepkg -sf

## build and install it
install: package
	sudo pacman -U --noconfirm $(PKGNAME)-*.pkg.tar.zst

uninstall:
	sudo pacman -Rns --noconfirm $(PKGNAME)

## drop everything that must not reach the repository
clean:
	rm -rf src pkg build
	rm -f *.pkg.tar.zst *.pkg.tar.zst.sig
	rm -f *.log
