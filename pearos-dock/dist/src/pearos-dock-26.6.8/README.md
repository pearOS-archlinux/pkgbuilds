# Pear Dock

A macOS Tahoe-style dock for KDE Plasma 6, built on top of the default KDE task manager.

**Author:** Alexandru Balan (alex@pear-software.com)
**Forked from:** [WaveTask](https://github.com/vickoc911/org.kde.plasma.wavetask) by Victor Calles

---

## Features

- macOS Tahoe-style dock behavior
- Gaussian zoom effect on hover (icons grow from the bottom)
- Bouncing icon animation on app launch
- Icon reflection
- Pinder — integrated Nautilus (Files) launcher with recent files and quick navigation
- Downloads Arc — folder shortcut with popup file browser
- Trash — integrated trash with empty action
- Separator between task manager and dock integrations
- Skin system with Tahoe Dark theme
- Configurable icon size, zoom level, and amplitude
- Blur effect for custom skins
- Activity dots under running apps
- No minimize on click for active apps (macOS behavior)
- Grouped windows: does nothing if one is active, restores all if all minimized

## Requirements

- KDE Plasma 6.6 or later
- CMake and development packages (for building from source)

## Build from Source

```sh
git clone <repo-url>
cd PearDock
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
sudo make install
```

## Panel Setup

After installing:

1. Right-click on your desktop → **Enter Edit Mode**
2. Go to the **Layout** tab
3. Select **Panel for wavetask** from the dropdown
4. Click **Apply**
5. Set panel width to **Fit to Content**
6. Set alignment to **Center**

> **Tip:** To avoid icons being clipped during zoom, set the panel height to at least `icon size × max zoom factor`. For example, 48px icons at 150% zoom require a panel height of at least 72px. The extra space above the dock visual is transparent.

## Skins

- **Tahoe Dark** (default)

## Known Limitations

- Currently only works in the bottom panel position
- Icons that zoom beyond the panel height will be clipped by the compositor — increase panel height to compensate

## License

GPL-2.0+
