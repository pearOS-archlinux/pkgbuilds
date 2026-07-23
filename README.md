<div align="center">
  <img
  src="https://github.com/user-attachments/assets/c6bec808-b8b5-42a6-a459-e05656e47c3c"
  width="64"
  style="border-radius: 99px;"
  alt="PearOS logo"></img>
  <h1 align="center">PearOS</h1>
</div>

# PearOS PKGBUILDS

[PearOS](https://pearos.xyz/) PKGBUILDS is a collection of [PKGBUILD](https://wiki.archlinux.org/index.php/PKGBUILD) scripts for building and installing packages on any Arch Linux-based operating system.

## Prerequisites

In order to use these PKGBUILD scripts, you will need to have the following tools and libraries installed on your system:

### General Build Tools
- [Git](https://git-scm.com/)
- [CMake](https://cmake.org/)
- [Ninja](https://ninja-build.org/)
- [pkgconf](https://archlinux.org/packages/extra/x86_64/pkgconf/)
- [Python](https://www.python.org/)
- [GCC](https://gcc.gnu.org/)
- [rsync](https://rsync.samba.org/)
- [xz](https://tukaani.org/xz/)

### Qt6 / KDE Frameworks
- qt6-base, qt6-tools, qt6-svg, qt6-translations, qt6-wayland
- extra-cmake-modules
- kcmutils, kconfig, kconfigwidgets, kcoreaddons, ki18n, kitemmodels, kservice, kwin, kwindowsystem

### Plasma / Wayland
- plasma-activities, plasma-workspace
- layer-shell-qt, wayland, wayland-protocols

### GRUB-specific
- autogen, device-mapper, freetype2, fuse2, gettext, help2man, texinfo, ttf-dejavu

### Other
- boost

## Usage

To build and install a package using one of the PKGBUILD scripts in this repository, follow these steps:

1. Clone this repository: `git clone https://github.com/pearOS-archlinux/pearos-pkgbuilds.git`
2. Navigate to the directory of the PKGBUILD script you want to use: `cd pearos-pkgbuilds/<package>`
3. Build the package: `makepkg -si`
4. Install the package: `sudo pacman -U <packagename>.pkg.tar.zst`

## Packages

| Package | Description |
|---------|-------------|
| `calamares` | Distribution-independent installer framework |
| `grub` | GNU GRand Unified Bootloader |
| `kscreenlocker` | pearOS dark theme for KDE screen locker |
| `neofetch` | CLI system information tool written in Bash |
| `pear-appstore` | Modern Qt6 app store for Arch-based systems |
| `pear-calamares-config` | Calamares config files for pearOS |
| `pearos-about` | pearOS NiceC0re system overview application (Qt6) |
| `pearos-bootsound` | Boot sound service for pearOS |
| `pearos-calculator` | pearOS calculator |
| `pearos-calendar` | Modern Qt6 calendar application for pearOS |
| `pearos-contacts` | Modern Qt6 contacts application for pearOS |
| `pearos-dock` | macOS Tahoe-style dock for KDE Plasma 6 |
| `pearos-grub-theme` | GRUB theme for pearOS NiceC0re |
| `pearos-icons` | Icons for pearOS |
| `pearos-keyring` | GPG keys for the pearOS package repository |
| `pearos-livecd-desktop` | Default pearOS installer in Electron |
| `pearos-muternvf` | Mutern fonts for pearOS |
| `pearos-notch` | Dynamic notch widget for Linux |
| `pearos-notes` | Modern Qt6 notes application for pearOS |
| `pearos-settings` | Settings for pearOS NiceC0re |
| `pearos-settings-app` | pearOS system settings application (Qt6/QML) |
| `pearos-todo` | Modern Qt6 to-do application for pearOS |
| `pearos-zshconfig` | pearOS NiceC0re zsh config file |
| `plasma-welcome` | pearOS NiceC0re welcome application (C++/Qt6) |
| `slimc0re-settings` | Settings for pearOS SlimC0re |
| `touchegg` | Multitouch gesture recognizer |

## Contributions

If you would like to contribute to PearOS PKGBUILDS, please fork this repository and submit a pull request with your changes. Please be sure to follow the [ArchLinux Packaging Guidelines](https://wiki.archlinux.org/title/Arch_package_guidelines) when creating or modifying PKGBUILD scripts.
