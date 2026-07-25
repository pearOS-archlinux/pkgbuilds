#!/usr/bin/env bash
# Builds and installs pearos-magiclamp from source.
#
# Usage: Run this script from the root of the repository.
#   ./install.sh
#
# Supported distributions:
#   Arch Linux  (pacman)  — Manjaro, EndeavourOS, Garuda Linux, CachyOS
#   Fedora      (dnf)     — Kinoite, Bazzite, Nobara
#   openSUSE    (zypper)  — Tumbleweed
#   Debian      (apt)     — Kubuntu, KDE neon, MX Linux, Nitrux
#
# Requires Plasma 6.6 or later.

set -euo pipefail

# ── Colors ────────────────────────────────────────────────────────────────────

nc='\033[0m'
red='\033[0;31m'
green='\033[0;32m'
yellow='\033[1;33m'
white='\033[1;37m'
ol='\033[53m'
ul='\033[4m'

# ── Config ────────────────────────────────────────────────────────────────────

base="$(cd "$(dirname "$0")" && pwd)"
repo=pearos-magiclamp

# ── Plasma version check ──────────────────────────────────────────────────────

if command -v plasmashell &>/dev/null; then
    plasma_ver=$(plasmashell --version 2>/dev/null | grep -oP '\d+\.\d+(\.\d+)?' | head -1)
    plasma_major=$(echo "$plasma_ver" | cut -d. -f1)
    plasma_minor=$(echo "$plasma_ver" | cut -d. -f2)
    if [[ "$plasma_major" -lt 6 ]] || { [[ "$plasma_major" -eq 6 ]] && [[ "$plasma_minor" -lt 6 ]]; }; then
        echo -e "${red}[ERROR]${nc} This effect requires Plasma 6.6 or later."
        exit 1
    else
        echo -e "${green}[OK]${nc} Plasma $plasma_ver detected."
    fi
fi

# ── Dependencies ──────────────────────────────────────────────────────────────

if command -v pacman &>/dev/null; then
    pm=pacman
    pkgs=(base-devel cmake extra-cmake-modules kwin
          kconfig kconfigwidgets kcmutils kcoreaddons kwindowsystem
          qt6-base libdrm vulkan-headers)
    is_installed() { pacman -Qq "$1" &>/dev/null; }
    do_install()   { sudo pacman -S --needed "${missing[@]}"; }
elif command -v dnf &>/dev/null; then
    pm=dnf
    pkgs=(cmake extra-cmake-modules kwin-devel
          kf6-kconfig-devel kf6-kconfigwidgets-devel kf6-kcmutils-devel
          kf6-kcoreaddons-devel kf6-kwindowsystem-devel
          qt6-qtbase-devel libdrm-devel libepoxy-devel vulkan-headers)
    is_installed() { rpm -q "$1" &>/dev/null; }
    do_install()   { sudo dnf install "${missing[@]}"; }
elif command -v zypper &>/dev/null; then
    pm=zypper
    pkgs=(gcc-c++ cmake kf6-extra-cmake-modules kwin6-devel
          kf6-kconfig-devel kf6-kconfigwidgets-devel kf6-kcmutils-devel
          kf6-kcoreaddons-devel kf6-kwindowsystem-devel
          qt6-base-devel qt6-declarative-devel libdrm-devel libepoxy-devel
          vulkan-headers)
    is_installed() { rpm -q "$1" &>/dev/null; }
    do_install()   { sudo zypper --non-interactive install "${missing[@]}"; }
elif command -v apt &>/dev/null; then
    pm=apt
    pkgs=(build-essential cmake extra-cmake-modules kwin-dev
          libkf6config-dev libkf6configwidgets-dev libkf6coreaddons-dev
          libkf6kcmutils-dev libkf6windowsystem-dev
          qt6-base-dev libdrm-dev libvulkan-dev)
    is_installed() { dpkg -s "$1" &>/dev/null 2>&1; }
    do_install()   { sudo apt install "${missing[@]}"; }
else
    echo -e "${red}[ERROR]${nc} No supported package manager found (pacman, dnf, zypper, apt)."
    exit 1
fi

echo -e "\n${ul}${white}Checking dependencies [$pm]${nc}\n"

missing=()
for pkg in "${pkgs[@]}"; do
    if is_installed "$pkg"; then
        echo -e "  ${green}[OK]${nc} $pkg"
    else
        echo -e "  ${yellow}[MISSING]${nc} $pkg"
        missing+=("$pkg")
    fi
done

if [[ "${#missing[@]}" -gt 0 ]]; then
    echo -e "\n${yellow}Installing missing packages...${nc}\n"
    do_install
fi

# ── Build ─────────────────────────────────────────────────────────────────────

echo -e "\n${ul}${white}Building $repo${nc}\n"

cmake -S "$base" -B "$base/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr
cmake --build "$base/build" --parallel "$(nproc)"

if [[ "$EUID" -ne 0 ]]; then
    sudo cmake --install "$base/build"
else
    cmake --install "$base/build"
fi

echo -e "\n${green}${ol}$(basename "$0")${white}${ol} done!${nc}\n"
