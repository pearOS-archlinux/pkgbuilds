# Makefile for pearos-zshconfig

PACKAGE_NAME = pearos-zshconfig
ROOT_HOME_DIR = /root
SKEL_DIR = /etc/skel

.PHONY: all package install uninstall clean

all: package

package:
	@echo "Creating Arch Linux package..."
	makepkg
	@echo "Package created!"

install:
	@echo "Installing $(PACKAGE_NAME)..."
	install -Dm644 .zshrc $(DESTDIR)$(ROOT_HOME_DIR)/.zshrc
	install -Dm644 .zshrc $(DESTDIR)$(SKEL_DIR)/.zshrc
	@if [ -z "$(DESTDIR)" ]; then install -Dm644 .zshrc "$(HOME)/.zshrc"; fi
	@echo "Installation complete!"

uninstall:
	@echo "Uninstalling $(PACKAGE_NAME)..."
	rm -f $(DESTDIR)$(ROOT_HOME_DIR)/.zshrc
	rm -f $(DESTDIR)$(SKEL_DIR)/.zshrc
	@if [ -z "$(DESTDIR)" ]; then rm -f "$(HOME)/.zshrc"; fi
	@echo "Uninstallation complete!"

clean:
	@echo "Cleaning build files and packages..."
	rm -f *.pkg.tar.zst *.pkg.tar.xz *.pkg.tar.zst.sig *.pkg.tar.xz.sig
	rm -rf ./pkg
	@echo "Clean complete!"
