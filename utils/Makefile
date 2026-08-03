.PHONY: all build clean

all: build

build:
	cmake -S hyprvisor -B hyprvisor/build -DCMAKE_BUILD_TYPE=Release
	cmake --build hyprvisor/build
	cmake -S wifiutil -B wifiutil/build -DCMAKE_BUILD_TYPE=Release
	cmake --build wifiutil/build

clean:
	rm -rf hyprvisor/build wifiutil/build
	rm -rf hyprvisor/gui/__pycache__
	rm -rf pkg src
	rm -f *.pkg.tar.* *.log
