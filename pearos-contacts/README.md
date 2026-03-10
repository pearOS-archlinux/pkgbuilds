# pearOS Contacts - Qt6/C++ Version

A modern, native contacts application for pearOS built with Qt6 and C++17, reusing the AppStore classmorphism UI.

## Features

- **My Groups**: Sidebar with contact groups (e.g. Personal, Work, Family) in AppStore style
- **Per-group contacts**: Main area shows contacts for the selected group
- **Contact fields**: Name, phone, email, and notes
- **Quick editing**: Add, edit, and delete contacts; rename groups
- **Persistent storage**: Data is saved under `/usr/share/extras/pearos-contacts/`

## Technology Stack

- **Language**: C++17
- **GUI Framework**: Qt6 (Widgets)
- **Build System**: CMake

## Prerequisites

### Build Dependencies

```bash
sudo pacman -S base-devel cmake qt6-base qt6-svg
```

## Building

1. Clone the repository:
```bash
git clone https://github.com/arch-linux-gui/pearos-contacts.git
cd pearos-contacts
```

2. Run the build script:
```bash
# This will create a build directory.
./build.sh
```

The binary will be in the build directory.

## Running

### From build directory

```bash
./build/pearos-contacts
```

### From system installation (if installed)

```bash
pearos-contacts
```

## License

This project is distributed under the MIT License. See LICENSE.

## Credits

- **Author**: DemonKiller
- **Original Project**: pearOS AppStore (Qt6/C++)
- **This App**: pearOS Contacts (Qt6/C++) built on top of the same UI
- **Community**: Arch Linux and Qt communities

## Contact

For issues, questions, or contributions, please visit:
- GitHub: https://github.com/arch-linux-gui/pearos-contacts
- Website: https://arkalinuxgui.org
- Discord: https://discord.com/invite/NgAFEw9Tkf
