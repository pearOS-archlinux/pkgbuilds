# pearos-window-tinter
#
# make         -> build the package with makepkg -s
# make install -> build and install with makepkg -si
# make clean   -> remove every build leftover so the tree is git-clean
#
# IMPORTANT: makepkg wipes and recreates $srcdir, which would be ./src --
# the directory holding the real sources. BUILDDIR moves makepkg's src/
# and pkg/ staging dirs out of the repo so they can never clobber them.

PKGNAME  := pearos-window-tinter
BUILDDIR := $(CURDIR)/.makepkg

export BUILDDIR

.PHONY: all package install clean

all: package

package:
	makepkg -s

install:
	makepkg -si

clean:
	rm -rf build .makepkg
	rm -f *.pkg.tar.zst *.pkg.tar.xz *.pkg.tar.gz
	rm -f *.log *.log.* $(PKGNAME)-*.src.tar.gz
	find . -name '*~' -delete
	find . -name '*.orig' -delete
	find . -name '*.rej' -delete
