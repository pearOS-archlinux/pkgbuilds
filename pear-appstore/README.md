# PearOS AppStore - Qt6/C++ Version

A modern, native package manager for Arch Linux built with Qt6 and C++17. This is a complete rewrite of the original Wails-based application.

## Features

- **Search Packages**: Search through official Arch repositories (core, extra) and AUR
- **View Installed Packages**: Browse and manage installed packages
- **Check for Updates**: View available updates for both official and AUR packages
- **Package Management**: Install, uninstall, and update packages
- **Modern UI**: Clean, dark-themed interface with responsive design
- **Smart Helper Detection**: Automatically detects and uses yay, paru, or falls back to pacman
- **Thread-Safe**: Uses modern C++ threading features for safe concurrent operations
- **Comprehensive Logging**: Built-in logger for debugging and monitoring

## Technology Stack

- **Language**: C++17
- **GUI Framework**: Qt6 (Widgets)
- **Package Management**: libalpm (Arch Linux Package Manager library)
- **AUR Integration**: AUR RPC API via Qt Network + Chaotic AUR Support
- **Build System**: CMake
- **Threading**: Qt Concurrent & STL threading

## Prerequisites

### Build Dependencies

```bash
sudo pacman -S base-devel cmake qt6-base qt6-svg alpm pkgconf
```

You can optionally also have either either yay or paru if you would like to work with packages from the AUR.

## Building

1. Clone the repository:
```bash
git clone https://github.com/arch-linux-gui/pearos-appstore.git
cd pearos-appstore
```

2. Run Build Script
```bash
# This will create a build directory.
./build.sh
```

Binary will be in the build directory.

## Running

### From Build Directory

```bash
./build/pearos-appstore
```

### From System Installation (if installed)

```bash
pearos-appstore
```


## License

This project is part of the Arch Linux GUI project.
It is distributed under the MIT License. Check LICENSE.

## Credits

- **Author**: DemonKiller
- **Original Project**: Wails-based PearOS AppStore
- **Rewrite**: Qt6/C++ implementation
- **Community**: Arch Linux and Qt communities

## Contact

For issues, questions, or contributions, please visit:
- GitHub: https://github.com/arch-linux-gui/pearos-appstore
- Website: https://arkalinuxgui.org
- Discord: https://discord.com/invite/NgAFEw9Tkf