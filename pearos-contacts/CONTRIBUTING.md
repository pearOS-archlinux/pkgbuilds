# Contributing to pearOS Contacts

Contributions are welcome and appreciated. To contribute:

- Code follows C++17 standards
- Proper error handling and logging
- Thread safety for concurrent operations
- Qt best practices for GUI code
- Comments for complex logic

### How to contribute

1. **Fork the repository.**
2. **Create a new branch** for your feature or bug fix:
   ```bash
   git checkout -b feature/your-feature-name
   ```
3. **Make your changes:**
   - Follow modern C++17 best practices
   - Use Qt6 APIs and conventions
   - Ensure code compiles without warnings
   - Test on your desktop environment (KDE, GNOME, or Xfce)
4. **Commit your changes** and push your branch:
   ```bash
   git commit -m "Add new feature or fix bug"
   git push -u origin feature/your-feature-name
   ```
5. **Open a Pull Request** describing your changes.

## Project structure

```
├── assets
│   ├── pearos-contacts.desktop
│   └── sidebar_icons (SVG)
├── build.sh
├── CMakeLists.txt
├── CONTRIBUTING.md
├── LICENSE
├── README.md
├── src
│   ├── gui
│   │   ├── mainwindow.cpp
│   │   ├── mainwindow.h
│   │   ├── contact_widget.cpp
│   │   └── contact_widget.h
│   ├── main.cpp
│   └── utils
│       ├── logger.h
│       ├── types.h
│       ├── contact_storage.cpp
│       └── contact_storage.h
├── stylesheet.qss
└── TODO.md
```

## Design decisions

### Modern C++

- **Smart pointers**: `std::unique_ptr` and `std::shared_ptr` for automatic memory management
- **Move semantics**: `std::move()` for efficient resource transfer
- **Auto type deduction**: Cleaner, more maintainable code
- **Range-based loops**: Clear iteration
- **Lambda functions**: Inline callbacks and signal connections

### Thread safety

- **Mutexes**: `std::mutex` and `std::lock_guard` for critical sections
- **Qt Concurrent**: `QtConcurrent::run()` for background operations
- **Signals and slots**: Qt's thread-safe communication

## Logging

The application includes a simple logging API:

```cpp
Logger::info("Information message");
Logger::warning("Warning message");
Logger::error("Error message");
Logger::debug("Debug message");
```

Logs are written to standard output and can be redirected for persistent logging.
