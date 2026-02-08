# Makefile for pearOS Welcome Application (C++/Qt6)

PACKAGE_NAME = pearos-welcome
EXECUTABLE = dist/pearos-welcome
DESKTOP_FILE = assets/welcome.desktop
ICON_FILE = assets/welcome.png
SHARE_DIR = /usr/share/pearos-welcome

# Installation directories
BIN_DIR = /usr/bin
APPS_DIR = /usr/share/applications
ICONS_DIR = /usr/share/pixmaps

.PHONY: all build package install uninstall clean

all: package

build:
	@echo "Building $(PACKAGE_NAME)..."
	./build.sh

package: build
	@echo "Creating Arch Linux package..."
	@if [ ! -f "$(EXECUTABLE)" ]; then \
		echo "Executable not found. Building first..."; \
		$(MAKE) build; \
	fi
	makepkg
	@echo "Package created!"

install: build
	@echo "Installing $(PACKAGE_NAME)..."
	install -Dm755 $(EXECUTABLE) $(DESTDIR)$(BIN_DIR)/$(PACKAGE_NAME)
	install -Dm644 $(DESKTOP_FILE) $(DESTDIR)$(APPS_DIR)/welcome.desktop
	install -Dm644 $(ICON_FILE) $(DESTDIR)$(ICONS_DIR)/welcome.png
	install -Dm644 styles.qss $(DESTDIR)$(SHARE_DIR)/styles.qss
	install -Dm644 assets/welcome.png $(DESTDIR)$(SHARE_DIR)/assets/welcome.png
	install -Dm644 assets/1welcome.png $(DESTDIR)$(SHARE_DIR)/assets/1welcome.png
	install -Dm644 assets/discord.svg $(DESTDIR)$(SHARE_DIR)/assets/discord.svg
	install -Dm644 assets/github.svg $(DESTDIR)$(SHARE_DIR)/assets/github.svg
	install -Dm644 assets/patreon.png $(DESTDIR)$(SHARE_DIR)/assets/patreon.png
	install -Dm644 assets/paypal.svg $(DESTDIR)$(SHARE_DIR)/assets/paypal.svg
	install -Dm644 assets/youtube.svg $(DESTDIR)$(SHARE_DIR)/assets/youtube.svg
	install -Dm644 list_windows.js $(DESTDIR)$(SHARE_DIR)/list_windows.js
	@echo "Installation complete!"

uninstall:
	@echo "Uninstalling $(PACKAGE_NAME)..."
	rm -f $(DESTDIR)$(BIN_DIR)/$(PACKAGE_NAME)
	rm -f $(DESTDIR)$(APPS_DIR)/welcome.desktop
	rm -f $(DESTDIR)$(ICONS_DIR)/welcome.png
	rm -rf $(DESTDIR)$(SHARE_DIR)
	@echo "Uninstallation complete!"

clean:
	@echo "Cleaning build files and packages..."
	rm -rf build/ dist/ src/
	rm -f *.pkg.tar.zst *.pkg.tar.xz *.pkg.tar.zst.sig *.pkg.tar.xz.sig
	rm -rf ./pkg
	@echo "Clean complete!"
