VERSION  := 26.6.8
PKGNAME  := pearos-dock
PKGREL   := 1
DISTDIR  := dist
SRCDIR   := $(PKGNAME)-$(VERSION)
TARBALL  := $(DISTDIR)/$(SRCDIR).tar.gz
PKGFILE  := $(DISTDIR)/$(PKGNAME)-$(VERSION)-$(PKGREL)-x86_64.pkg.tar.zst

.PHONY: all clean

all: $(PKGFILE)

# ── Final package ────────────────────────────────────────────────────────────
$(PKGFILE): $(DISTDIR)/PKGBUILD $(TARBALL)
	cd $(DISTDIR) && makepkg -f
	@echo ""
	@echo "Package ready: $(PKGFILE)"
	@echo "Install with:  sudo pacman -U $(PKGFILE)"

# ── PKGBUILD (with real sha256) ───────────────────────────────────────────────
$(DISTDIR)/PKGBUILD: PKGBUILD $(TARBALL) | $(DISTDIR)
	cp PKGBUILD $(DISTDIR)/PKGBUILD
	$(eval SHA256 := $(shell sha256sum $(TARBALL) | cut -d' ' -f1))
	sed -i "s/SHA256SUM_PLACEHOLDER/$(SHA256)/" $(DISTDIR)/PKGBUILD

# ── Source tarball ────────────────────────────────────────────────────────────
$(TARBALL): | $(DISTDIR)
	tar --transform "s|^\.|$(SRCDIR)|" \
	    --exclude="./.git" \
	    --exclude="./dist" \
	    --exclude="./build" \
	    -czf $(TARBALL) .

$(DISTDIR):
	mkdir -p $(DISTDIR)

# ── Clean ─────────────────────────────────────────────────────────────────────
clean:
	rm -rf build dist
