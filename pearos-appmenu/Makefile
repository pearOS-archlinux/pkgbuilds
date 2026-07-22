PKGVER  = 6.7.80
PKGREL  = 1
ARCH    = x86_64
PKG     = plasma-appmenu-$(PKGVER)-$(PKGREL)-$(ARCH).pkg.tar.zst

SOURCES = $(shell find . \
    -path './build'          -prune -o \
    -path './pkg'            -prune -o \
    -path './src'            -prune -o \
    -path './.git'           -prune -o \
    \( -name "*.cpp"         \
    -o -name "*.h"           \
    -o -name "*.qml"         \
    -o -name "*.xml"         \
    -o -name "*.json"        \
    -o -name "CMakeLists.txt"\
    -o -name "PKGBUILD"      \
    \) -print)

.PHONY: all clean

all: $(PKG)

$(PKG): $(SOURCES)
	makepkg -f --noconfirm --noprogressbar

clean:
	rm -rf build pkg src plasma-appmenu-*.pkg.tar.zst
