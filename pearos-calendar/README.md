# pearOS Calendar - Qt6/C++ Version

A modern, native calendar application for pearOS built with Qt6 and C++17, using the AppStore classmorphism UI (frameless, translucent, blurred window).

## Features

- **Month View**: Continuously scrollable month grid spanning 1980-2040, with custom-painted cells (today highlight, event markers)
- **Mini Calendar**: Compact sidebar calendar synced to the currently viewed month
- **Sidebar Events**: Collapsible sidebar listing upcoming events, with an animated open/close toggle
- **Event Management**: Double-click any day to add/edit/delete an event; events are persisted locally
- **PearID Integration**: Detects PearID login state (logged in / logged out / loading), shows the account name and avatar in the sidebar profile, with a cached last-known-good state so transient network errors don't flip a valid session to "logged out"
- **Classmorphism UI**: Frameless window, translucent background, KWin blur-behind (when available), custom traffic-light controls and edge resizing

## Technology Stack

- **Language**: C++17
- **GUI Framework**: Qt6 (Core, Gui, Widgets, Network, Concurrent)
- **Build System**: CMake
- **Optional**: KF6 WindowSystem (KWin blur-behind effect)

## Prerequisites

### Build Dependencies

```bash
sudo pacman -S base-devel cmake qt6-base
```

## Building

1. Clone the repository:
```bash
git clone https://github.com/arch-linux-gui/pearos-calendar.git
cd pearos-calendar
```

2. Run the build script:
```bash
# This will create a build directory.
./build.sh
```

Binary will be in the `build` directory.

## Running

### From Build Directory

```bash
./build/pear-calendar
```

### From System Installation (if installed)

```bash
pearos-calendar
```

## Project Structure

```
src/
  main.cpp             # Application entry point
  gui/mainwindow.{h,cpp} # Main window: header, sidebar, month/year views, event dialogs
  backend/pearidmanager.{h,cpp} # PearID login state, user info, avatar
  utils/                # Logger, shared types
pearID/                 # Bash scripts for PearID auth/session (shared with other pearOS apps)
assets/                 # Icons and resources
stylesheet.qss          # Application stylesheet
```

## License

This project is distributed under the MIT License. Check LICENSE.

## Credits

- **Original Project**: pearOS AppStore (Qt6/C++)
- **This App**: pearOS Calendar (Qt6/C++) built on top of the same UI
- **Community**: Arch Linux and Qt communities

## Contact

For issues, questions, or contributions, please visit:
- GitHub: https://github.com/arch-linux-gui/pearos-calendar
- Website: https://arkalinuxgui.org
- Discord: https://discord.com/invite/NgAFEw9Tkf
