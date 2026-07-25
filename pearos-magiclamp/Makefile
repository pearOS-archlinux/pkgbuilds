PKGNAME  := pearos-magiclamp
BUILDDIR := $(CURDIR)/.pkgbuild

.PHONY: all clean

all:
	BUILDDIR="$(BUILDDIR)" makepkg -si --nosign

clean:
	rm -rf "$(BUILDDIR)" build pkg src.makepkg
	rm -f $(PKGNAME)-*.pkg.tar.* $(PKGNAME)-*.log
