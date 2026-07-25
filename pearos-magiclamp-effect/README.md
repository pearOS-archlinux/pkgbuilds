# Pearos Magic Lamp (Plasma 6.6+)

<div align="center">
  <a href="https://youtu.be/i8tt4JLWGy8">
    <img src="https://github.com/user-attachments/assets/616a9a82-1fd1-4477-a436-9be6003c80f2" alt="Demo" />
  </a>
</div>

[Demo](https://youtu.be/i8tt4JLWGy8)

Pearos Magic Lamp is a window minimization effect for KWin. Whenever a window is minimized, it'll get sucked down into the dock/panel. Compared to the Magic Lamp effect shipped with KWin, this effect features a smoother, more „curvy“ animation and more configuration options. Additionally, this fork improves upon both the built-in effect and the original by [Vlad Zahorodnii](https://github.com/zzag/kwin-effects-yet-another-magic-lamp) with fixed multi-monitor support — correctly identifying the target panel when minimizing, regardless of which screen the window is on or how panels are distributed across displays.

This project was originally created by [Vlad Zahorodnii](https://github.com/zzag/kwin-effects-yet-another-magic-lamp), who maintained it through Plasma 5.27. Since the original repository has not been updated in several years, this fork continues development independently, focusing exclusively on Wayland under Plasma 6. X11 is no longer supported.

### Installation

#### Quick install

Clone the repository and run the install script — it will check and install all required dependencies automatically, then build and install the effect.

```yaml
git clone https://github.com/koryboc/pearos-magiclamp.git
cd pearos-magiclamp
chmod +x ./install.sh
./install.sh
```

Supports Arch Linux, Fedora, openSUSE Tumbleweed, and Debian, as well as most distributions based on these.

#### Build from source

You will need the following dependencies to build this effect:
* CMake
* any C++20 enabled compiler
* Qt 6: Core, DBus, Gui, Widgets
* KWin (Plasma 6.6 or later)
* KDE Frameworks 6:
    - Config
    - ConfigWidgets
    - CoreAddons
    - Extra CMake Modules
    - KCMUtils
    - WindowSystem
* libdrm
* Vulkan headers

On Arch Linux → Manjaro, EndeavourOS, Garuda Linux, CachyOS

```yaml
sudo pacman -S \
    base-devel \
    cmake \
    extra-cmake-modules \
    kwin \
    kconfig \
    kconfigwidgets \
    kcmutils \
    kcoreaddons \
    kwindowsystem \
    qt6-base \
    libdrm \
    vulkan-headers
```

On Fedora → Kinoite, Bazzite, Nobara

```yaml
sudo dnf install \
    cmake \
    extra-cmake-modules \
    kwin-devel \
    kf6-kconfig-devel \
    kf6-kconfigwidgets-devel \
    kf6-kcmutils-devel \
    kf6-kcoreaddons-devel \
    kf6-kwindowsystem-devel \
    qt6-qtbase-devel \
    libdrm-devel \
    libepoxy-devel \
    vulkan-headers
```

On openSUSE Tumbleweed

```yaml
sudo zypper install \
    gcc-c++ \
    cmake \
    kf6-extra-cmake-modules \
    kwin6-devel \
    kf6-kconfig-devel \
    kf6-kconfigwidgets-devel \
    kf6-kcmutils-devel \
    kf6-kcoreaddons-devel \
    kf6-kwindowsystem-devel \
    qt6-base-devel \
    qt6-declarative-devel \
    libdrm-devel \
    libepoxy-devel \
    vulkan-headers
```

On Debian → Kubuntu, KDE neon, MX Linux, Nitrux

> **Note:** This effect requires KWin 6.6 or later. Building on Debian-based distributions may
> fail if the KWin version available in the repositories does not meet this requirement.

```yaml
sudo apt install \
    build-essential \
    cmake \
    extra-cmake-modules \
    kwin-dev \
    libkf6config-dev \
    libkf6configwidgets-dev \
    libkf6coreaddons-dev \
    libkf6kcmutils-dev \
    libkf6windowsystem-dev \
    qt6-base-dev \
    libdrm-dev \
    libvulkan-dev
```

After you installed all the required dependencies, you can build
the effect:

```yaml
git clone https://github.com/koryboc/pearos-magiclamp.git
cd pearos-magiclamp
mkdir build && cd build
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr
make
sudo make install
```


#### Building the effect against older Plasma versions

This fork targets **Plasma 6.6 and later** exclusively. For older Plasma (5.x) releases, use the original repository by [zzag](https://github.com/zzag/kwin-effects-yet-another-magic-lamp), which provides dedicated branches per Plasma version.

**Step 1** — Clone the original repository:

```yaml
git clone https://github.com/zzag/kwin-effects-yet-another-magic-lamp.git
cd kwin-effects-yet-another-magic-lamp
```

**Step 2** — Check out the branch matching your Plasma version:

```yaml
git checkout Plasma/5.27
```

Replace `5.27` with your target version (e.g. `5.24`, `5.23`).


### Using the effect

Open **System Settings** and navigate to:

**Animations** → **Window minimize**

Select **"Pearos Magic Lamp"** and click **Apply**.

Forked from https://github.com/koryboc/kwin-effects-yet-another-magic-lamp-reloaded
