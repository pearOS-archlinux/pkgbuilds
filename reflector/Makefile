.PHONY: all clean

all:
	makepkg -s

clean:
	find . -mindepth 1 -maxdepth 1 ! -name Makefile ! -name PKGBUILD -exec rm -rf {} +
