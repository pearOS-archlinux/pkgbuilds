# Contributing to ALG Welcome

Contributions are welcome and appreciated! To contribute:

- Code follows C++17 standards
- Proper error handling and logging
- Thread safety for concurrent operations
- Qt best practices for GUI code
- Comments for complex logic

### How to contribute
1. **Fork the Repository.**
2. **Create a New Branch** for your feature or bug fix:
   ```bash
   git checkout -b feature/your-feature-name
   ```
3. **Make Your Changes:**
   - Follow modern C++17 best practices
   - Use Qt6 APIs and conventions
   - Ensure code compiles without warnings
   - Test on your desktop environment (KDE, GNOME, or Xfce)
4. **Commit Your Changes** and push your branch:
   ```bash
   git commit -m "Add new feature or fix bug"
   git push -u origin feature/your-feature-name
   ```
5. **Open a Pull Request** describing your changes.


## Project Structure

```
├── assets
│   ├── pearos-notes.desktop
│   └── pearos-notes.png
├── build.sh
├── CMakeLists.txt
├── CONTRIBUTING.md
├── DEVELOPER_GUIDE.md
├── LICENSE
├── QT_REWRITE_SUMMARY.md
├── README.md
├── src
│   ├── core
│   │   ├── alpm_wrapper.cpp
│   │   ├── alpm_wrapper.h
│   │   ├── aur_helper.cpp
│   │   ├── aur_helper.h
│   │   ├── package_manager.cpp
│   │   └── package_manager.h
│   ├── gui
│   │   ├── home_widget.cpp
│   │   ├── home_widget.h
│   │   ├── installed_widget.cpp
│   │   ├── installed_widget.h
│   │   ├── mainwindow.cpp
│   │   ├── mainwindow.h
│   │   ├── package_card.cpp
│   │   ├── package_card.h
│   │   ├── package_details_dialog.cpp
│   │   ├── package_details_dialog.h
│   │   ├── search_widget.cpp
│   │   ├── search_widget.h
│   │   ├── settings_widget.cpp
│   │   ├── settings_widget.h
│   │   ├── updates_widget.cpp
│   │   └── updates_widget.h
│   ├── main.cpp
│   └── utils
│       ├── logger.h
│       └── types.h
├── stylesheet.qss
└── TODO.md

6 directories, 36 files
```

## Understanding the code
### Core
#### AlpmWrapper (Singleton)
Wraps libalpm functionality with thread-safe operations:
- Package searching
- Installed package enumeration
- Update detection
- Package information retrieval

#### AurHelper
Handles AUR integration:
- Package search via AUR RPC API
- Package information retrieval
- Update checking for AUR packages

#### PackageManager (Singleton)
Manages package operations with proper privilege escalation:
- Install/uninstall packages
- Update operations
- Automatic helper detection (yay/paru/pacman)
- Process management with signals

### GUI Components
- **MainWindow**: Tabbed interface container
- **HomeWidget**: Featured packages display
- **SearchWidget**: Package search with filtering
- **InstalledWidget**: Installed package browser
- **UpdatesWidget**: Update management
- **PackageCard**: Reusable package display widget
- **PackageDetailsDialog**: Detailed package information

## Design Decisions

### Modern C++ Features

- **Smart Pointers**: `std::unique_ptr` and `std::shared_ptr` for automatic memory management
- **Move Semantics**: `std::move()` for efficient resource transfer
- **Auto Type Deduction**: Cleaner, more maintainable code
- **Range-based Loops**: Cleaner iteration
- **Lambda Functions**: Inline callbacks and signal connections

### Thread Safety

- **Mutexes**: `std::mutex` and `std::lock_guard` for critical sections
- **Qt Concurrent**: `QtConcurrent::run()` for background operations
- **Signal/Slot**: Qt's thread-safe communication mechanism

### Singleton Pattern

Used for `AlpmWrapper` and `PackageManager` to ensure:
- Single libalpm handle instance
- Centralized package operation management
- Thread-safe access

## Logging

The application includes a comprehensive logging system:

```cpp
Logger::info("Information message");
Logger::warning("Warning message");
Logger::error("Error message");
Logger::debug("Debug message");
```

Logs are output to standard output and can be redirected for persistent logging.

## Package Helper Detection

The application automatically detects available package helpers in this order:

1. **yay** - Preferred for AUR support
2. **paru** - Alternative AUR helper
3. **pacman** - Fallback (official repos only)

## Privilege Escalation

Package operations require root privileges. The application uses `pkexec` (PolicyKit) for secure privilege escalation. Ensure PolicyKit is properly configured on your system.