.PHONY: all install clean deploy

# Build the ArchLinux package
all:
	makepkg -sf --noconfirm

# Build and install the package
install: all
	sudo pacman -U pearos-lockscreen*.pkg.tar.zst --noconfirm

# Deploy only theme assets without rebuilding (fast iteration during development)
deploy:
	sudo cp lockscreen/LockScreenUi.qml \
		/usr/share/plasma/shells/org.kde.plasma.desktop/contents/lockscreen/LockScreenUi.qml
	sudo mkdir -p \
		/usr/share/plasma/shells/org.kde.plasma.desktop/contents/lockscreen/fonts \
		/usr/share/fonts/pearos
	sudo cp lockscreen/fonts/*.otf \
		/usr/share/plasma/shells/org.kde.plasma.desktop/contents/lockscreen/fonts/
	sudo cp lockscreen/fonts/*.otf \
		/usr/share/fonts/pearos/
	sudo fc-cache -f
	sudo cp pearOS-dark/images/*.svg \
		/usr/share/plasma/shells/org.kde.plasma.desktop/contents/lockscreen/images/
	@echo "Theme deployed — press Super+L to test"

# Remove build artifacts
clean:
	rm -rf pkg/ src/ *.pkg.tar.zst
