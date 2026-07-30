BUILD_DIR := build

.PHONY: all build clean distclean

all: build

build:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=Release
	cmake --build $(BUILD_DIR) -j$(shell nproc)

clean:
	rm -rf $(BUILD_DIR) pkg src *.pkg.tar.zst *.pkg.tar.xz

distclean: clean
