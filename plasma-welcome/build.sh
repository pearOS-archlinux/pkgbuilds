#!/bin/bash
# Build script for pearOS welcome application
# Creates a standalone executable using PyInstaller

set -e

echo "Building pearOS welcome application..."

# Add ~/.local/bin to PATH if it exists
if [ -d "$HOME/.local/bin" ]; then
    export PATH="$HOME/.local/bin:$PATH"
fi

# Install requirements if requirements.txt exists
if [ -f "requirements.txt" ]; then
    echo "Installing requirements from requirements.txt..."
    pip install --break-system-packages -r requirements.txt || python -m pip install --break-system-packages -r requirements.txt
    echo "Requirements installed."
fi

# Check if PyInstaller is installed
if ! command -v pyinstaller &> /dev/null; then
    echo "PyInstaller not found. Attempting to install..."
    
    # Try to install via pacman first (Arch Linux)
    if command -v pacman &> /dev/null; then
        echo "Trying to install via pacman..."
        sudo pacman -S --noconfirm python-pyinstaller 2>/dev/null || {
            echo "PyInstaller not available via pacman. Installing via pip..."
            pip install --break-system-packages pyinstaller || python -m pip install --break-system-packages pyinstaller
            # Add ~/.local/bin to PATH after pip install
            if [ -d "$HOME/.local/bin" ]; then
                export PATH="$HOME/.local/bin:$PATH"
            fi
        }
    else
        # For non-Arch systems, try pip with --break-system-packages
        pip install --break-system-packages pyinstaller || python -m pip install --break-system-packages pyinstaller
        # Add ~/.local/bin to PATH after pip install
        if [ -d "$HOME/.local/bin" ]; then
            export PATH="$HOME/.local/bin:$PATH"
        fi
    fi
    
    # Verify installation
    if ! command -v pyinstaller &> /dev/null; then
        echo "ERROR: Failed to install PyInstaller. Please install it manually:"
        echo "  sudo pacman -S python-pyinstaller"
        echo "  or"
        echo "  pip install --break-system-packages pyinstaller"
        echo ""
        echo "If installed via pip, make sure ~/.local/bin is in your PATH:"
        echo "  export PATH=\"\$HOME/.local/bin:\$PATH\""
        exit 1
    fi
fi

# Clean previous builds
echo "Cleaning previous builds..."
rm -rf build/ dist/ *.spec

# Build with PyInstaller
echo "Building executable..."
pyinstaller \
    --name="pearos-welcome" \
    --onefile \
    --windowed \
    --icon=assets/welcome.png \
    --add-data="assets:assets" \
    --add-data="styles.qss:." \
    --hidden-import=PySide6 \
    --hidden-import=BlurWindow \
    --exclude-module=PyQt5 \
    --exclude-module=PyQt6 \
    --collect-all=PySide6 \
    main.py

echo ""
echo "Build complete! Executable is in dist/pearos-welcome"
echo "To install system-wide, run:"
echo "  sudo cp dist/pearos-welcome /usr/bin/"
echo "  sudo chmod +x /usr/bin/pearos-welcome"

