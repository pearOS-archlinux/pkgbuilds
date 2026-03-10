#!/bin/bash

# Build script for pearOS ToDo Qt6 version

set -e

echo "==================================="
echo "pearOS ToDo - Qt6 Build Script"
echo "==================================="
echo ""

# Check for required dependencies
echo "Checking dependencies..."

command -v cmake >/dev/null 2>&1 || {
    echo "Error: cmake is not installed. Install with: sudo pacman -S cmake"
    exit 1
}

command -v qmake6 >/dev/null 2>&1 || command -v qmake >/dev/null 2>&1 || {
    echo "Error: Qt6 is not installed. Install with: sudo pacman -S qt6-base"
    exit 1
}

echo "All dependencies found!"
echo ""

# Clean previous build
if [ -d "build" ]; then
    echo "Cleaning previous build..."
    rm -rf build
fi

# Create build directory
echo "Creating build directory..."
mkdir -p build

# Copy PKGBUILD into build so packaging never touches the source tree
if [ -f PKGBUILD ]; then
    cp PKGBUILD build/
fi

cd build

# Configure with CMake
echo ""
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo ""
echo "Building..."
make -j$(nproc)

echo ""
echo "==================================="
echo "Build completed successfully!"
echo "==================================="
echo ""
echo "To run the application:"
echo "  ./build/pearos-todo"
echo ""
echo "To install system-wide:"
echo "  sudo make install (from build directory)"
echo ""
echo "To build an Arch package safely (without touching source tree):"
echo "  cd build && make pkg"
echo ""
