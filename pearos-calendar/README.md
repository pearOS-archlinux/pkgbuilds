# pearOS Notes - Qt6/C++ Version

A modern, native notes application for pearOS built with Qt6 and C++17, reusing the AppStore classmorphism UI.

## Features

- **My Lists**: Sidebar cu liste de notițe („My Lists”) în stil AppStore
- **Per-list Notes**: Fereastră principală cu notele listei selectate
- **Progress Overview**: Text de tipul „X notes, Y marked done” pentru lista curentă
- **Fast Editing**: Bifezi/debifezi rapid notițe și adaugi altele noi
- **Persistent Storage**: Datele sunt salvate în `/usr/share/extras/pearos-notes/`

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
git clone https://github.com/arch-linux-gui/pearos-notes.git
cd pearos-notes
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
./build/pearos-notes
```

### From System Installation (if installed)

```bash
pearos-notes
```


## License

This project is distributed under the MIT License. Check LICENSE.

## Credits

- **Author**: DemonKiller
- **Original Project**: pearOS AppStore (Qt6/C++)
- **This App**: pearOS Notes (Qt6/C++) built on top of the same UI
- **Community**: Arch Linux and Qt communities

## Contact

For issues, questions, or contributions, please visit:
- GitHub: https://github.com/arch-linux-gui/pearos-notes
- Website: https://arkalinuxgui.org
- Discord: https://discord.com/invite/NgAFEw9Tkf