.PHONY: build clean install

build:
	makepkg -s --noconfirm

install: build
	sudo pacman -U --noconfirm *.pkg.tar.zst

clean:
	rm -rf pkg/ src/ chwd/ *.pkg.tar.zst *.pkg.tar.zst.sig *.log
