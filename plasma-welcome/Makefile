# Makefile for pearOS Welcome Application

PACKAGE_NAME = pearos-welcome
EXECUTABLE = dist/pearos-welcome
DESKTOP_FILE = assets/welcome.desktop
ICON_FILE = assets/welcome.png

# Installation directories
BIN_DIR = /usr/bin
APPS_DIR = /usr/share/applications
ICONS_DIR = /usr/share/pixmaps

.PHONY: all build package install uninstall clean

all: build package

build:
	@echo "Building $(PACKAGE_NAME)..."
	./build.sh

package:
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
	@echo "Installation complete!"

uninstall:
	@echo "Uninstalling $(PACKAGE_NAME)..."
	rm -f $(DESTDIR)$(BIN_DIR)/$(PACKAGE_NAME)
	rm -f $(DESTDIR)$(APPS_DIR)/welcome.desktop
	rm -f $(DESTDIR)$(ICONS_DIR)/welcome.png
	@echo "Uninstallation complete!"

clean:
	@echo "Cleaning build files and packages..."
	rm -rf build/ dist/ *.spec
	rm -f *.pkg.tar.zst *.pkg.tar.xz *.pkg.tar.zst.sig *.pkg.tar.xz.sig
	rm -rf ./pkg
	find . -type d -name "__pycache__" -exec rm -r {} + 2>/dev/null || true
	find . -type f -name "*.pyc" -delete 2>/dev/null || true
	find . -type f -name "*.pyo" -delete 2>/dev/null || true
	@echo "Clean complete!"
