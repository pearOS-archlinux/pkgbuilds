# hyprvisor — GPU driver manager

## Installing a driver

```
# 1. Detect the GPU and currently loaded driver
hyprvisor --detect

# 2. See what packages are recommended (no install)
hyprvisor --list

# 3. Install the recommended driver(s)
hyprvisor --install
```

**Run `--install` as a regular, sudo-capable user — not via `sudo hyprvisor
...`.** `runPacman()` escalates its own calls with `sudo` as needed; AUR
builds (`paru`/`yay`/`trizen`) refuse to run as root outright, so the process
itself must stay unprivileged. This matches a standard Arch live/install
environment where the session user already has (passwordless, in scripted
installs) sudo rights.

Already-installed packages are skipped either way.

### Unattended mode (`--noconfirm`)

For driving `hyprvisor` from an Arch install script with zero keyboard input:

```
hyprvisor --install --noconfirm
```

- Passes `--noconfirm` to both `pacman` and the AUR helper — no prompts.
- After installing any `*-dkms` package, automatically runs `mkinitcpio -P`
  so the module is picked up on next boot (official `nvidia`/`nvidia-open`
  packages ship a pacman hook that does this already; the AUR legacy
  branches — 470xx/390xx/340xx/580xx — may not, so hyprvisor does it itself).
- Still requires the invoking user to have working (passwordless) sudo —
  it does not prompt for or store a password itself.
- **Precompiled kernel-specific modules, when available.** For Turing+
  NVIDIA GPUs, some distros/mirrors ship a module prebuilt for one exact
  kernel package (e.g. `linux-cachyos-lts-nvidia-open`, `linux618-nvidia-open`)
  — no DKMS build, no kernel headers, no compile time. hyprvisor detects the
  running kernel package and checks whether `<kernel-package>-nvidia-open`
  exists; if so it's recommended ahead of the generic `nvidia-open-dkms`
  (which stays listed as a fallback). No such packages exist for the legacy
  branches (470xx/390xx/340xx/580xx) on any mirror checked so far — those
  always go through the AUR DKMS build.
- **Kernel headers for DKMS.** Any recommended package with `dkms` in its
  name (`nvidia-dkms`, `nvidia-open-dkms`, `nvidia-580xx-dkms`, ...) needs
  matching kernel headers to actually build — DKMS can't express that as a
  normal pacman dependency since it's tied to whatever kernel you're running.
  hyprvisor detects it via `pacman -Qo /usr/lib/modules/$(uname -r)` (works
  for `linux`, `linux-zen`, `linux-lts`, or any custom-numbered kernel
  package) and adds the matching `*-headers` package to the install list
  automatically, so `--install`/`--noconfirm` don't silently fail to build
  the module for lack of headers.
- **Incompatible driver cleanup.** If a package that's known-wrong for the
  detected GPU is installed (e.g. plain `nvidia` left over next to the
  correct `nvidia-580xx-dkms` on a Pascal card — two nvidia kernel modules
  registered at once is a real conflict, not just clutter), `hyprvisor --list`
  always prints it as a red `[!]` warning with the exact `pacman -R` command.
  `--install --noconfirm` goes further and removes it automatically
  (`sudo pacman -R <pkg> --noconfirm`) after the correct driver is confirmed
  installed. The JSON output (`--list --json`) exposes this as each driver
  entry's `"incompatible"` boolean.
- **AMD GCN 1.0/1.1** (HD 7xxx, R7/R9 2xx) is the one GPU tier that needs a
  kernel cmdline param to use `amdgpu` at all (otherwise it silently falls
  back to the much weaker `radeon` driver). hyprvisor adds
  `amdgpu.si_support=1 amdgpu.cik_support=1` itself — to
  `GRUB_CMDLINE_LINUX_DEFAULT` in `/etc/default/grub` (then runs
  `grub-mkconfig`) and/or to every `options` line under
  `/boot/loader/entries/*.conf` (systemd-boot), whichever it finds. A
  `.hyprvisor.bak` backup is written before either file is touched, and it's
  idempotent (won't duplicate the params on a second run).

Nothing else touches bootloader config or modprobe blacklists — just package
install, the GCN param above, and initramfs rebuild.

### Options

| Flag | Effect |
|---|---|
| `--detect` | GPU detection + currently loaded kernel driver only, no recommendations |
| `--list [--prefer open\|proprietary] [--json]` | List available/recommended packages |
| `--simulate "<name>" [--prefer ...] [--json]` | Show what would install for a typed GPU name, no hardware needed |
| `--install [--prefer open\|proprietary] [--noconfirm]` | Install the recommended packages |
| `--prefer proprietary` | Force proprietary/legacy-AUR driver even where AUTO would default to open-source |
| `--prefer open` | Force open-source (nouveau/mesa) even where AUTO would default to a proprietary/open-kernel driver |

### Example (GTX 1050 Ti Gigabyte OC 4GB → Pascal)

```
$ hyprvisor --install --noconfirm

[hyprvisor] --noconfirm: installing unattended, no prompts.
[hyprvisor] Detected GPU(s):

  1. NVIDIA GeForce GTX 1050 Ti (driver: nouveau)
     Vendor  : NVIDIA
     PCI addr: 0000:01:00.0
     IDs     : 10de:1c82

[GPU 1] NVIDIA GeForce GTX 1050 Ti
  NVIDIA architecture : Pascal (GTX 10xx)
  Driver preference   : proprietary (nvidia-580xx-dkms, default)
  [!] Nvidia dropped Maxwell/Pascal/Volta support in driver 590.
      Use nvidia-580xx-dkms, not 'nvidia' (will fail to load → 800x600 fallback).

  Packages to install (AUR):
    + nvidia-580xx-dkms
    + nvidia-580xx-utils
[hyprvisor] Running (AUR): paru -S --needed --noconfirm nvidia-580xx-dkms nvidia-580xx-utils
[hyprvisor] Running: sudo mkinitcpio -P
```

With `--prefer open` on the same card, it installs `xf86-video-nouveau` +
`mesa` instead of `nvidia-580xx-dkms`.

---

## GUI

`gui/hyprvisor-gui.py` is a dark-mode GTK3 front-end for the CLI above.
Requires PyGObject (`python-gobject` / `pygobject3`, already needed for most
GTK desktops) and the `hyprvisor` binary built or in `PATH`.

```
python3 gui/hyprvisor-gui.py
```

- Detects the current GPU and pre-selects the recommended driver choice
  (nouveau / open kernel / proprietary) in a combo box — options with no
  packages for that GPU (e.g. "proprietary" on AMD/Intel) are hidden.
- Shows the exact package table (type, AUR, installed status) for whichever
  choice is selected.
- **Install** confirms, then installs official-repo packages via
  `pkexec pacman -S --needed`, and opens a terminal to run the AUR helper
  (`paru`/`yay`/`trizen`) interactively for AUR packages (AUR builds need a
  real prompt for the sudo password and build review).
- **Simulate…** opens a second window with a browsable list of common GPU
  models plus a search box — type or click any GPU name and it shows what
  hyprvisor would install for it via `hyprvisor --simulate "<name>"`, with
  no real hardware required and no Install button (it's hypothetical).
- **Warnings.** Above the package table, any unrelated driver (wrong vendor
  or wrong generation), incompatible-installed package, or kernel/headers
  version mismatch shows as a red `⚠` line, each with its own **Remove**
  button (`pkexec pacman -R`) — the same detection `--list` prints in the
  terminal, surfaced in the GUI too.
- **Clean All…** in the header bar removes every known GPU driver package
  (any vendor) for a fresh start — dry-runs `hyprvisor --clean` first to
  show exactly what would be removed in a confirmation dialog, then runs
  `hyprvisor --clean --noconfirm` for real if you confirm.

---

## Driver matrix by GPU

`hyprvisor` detects the GPU (PCI vendor/device ID + `lspci`), maps it to a
generation, and recommends packages to install via `pacman`/AUR helper.
Table below reflects current logic in `src/driver_manager.cpp` (default
`--prefer` = AUTO = proprietary for NVIDIA, open-source for AMD/Intel/S3/VIA).

## NVIDIA

Default (AUTO, no `--prefer` flag) auto-selects the best driver per
generation. `--prefer proprietary` / `--prefer open` override that default
where an alternative exists.

| Generation | Example cards (consumer / datacenter) | Default (AUTO) | `--prefer proprietary` | Notes |
|---|---|---|---|---|
| Blackwell | RTX 50xx / B100, B200, GB200 | `nvidia-open-dkms` (open kernel) | `nvidia`, `nvidia-utils` | |
| Ada Lovelace | RTX 40xx / L4, L40, H100, H200 | `nvidia-open-dkms` (open kernel) | `nvidia`, `nvidia-utils` | H100/H200 (Hopper) driver-compatible with Ada tier |
| Ampere | RTX 30xx / A100, A40, A30, A10, A16 | `nvidia-open-dkms` (open kernel) | `nvidia`, `nvidia-utils` | |
| Turing | RTX 20xx, GTX 16xx / T4 | `nvidia-open-dkms` (open kernel) | `nvidia`, `nvidia-utils` | |
| Volta | Titan V / V100 | `nvidia-580xx-dkms` (AUR) | same as default | Current `nvidia` (590+) dropped support |
| Pascal | GTX 10xx (incl. GTX 1050 Ti, any AIB/OC variant) / P100, P40, P4 | `nvidia-580xx-dkms` (AUR) | same as default | Current `nvidia` (590+) dropped support |
| Maxwell | GTX 750, GTX 9xx / M60, M40, M4 | `nvidia-580xx-dkms` (AUR) | same as default | Current `nvidia` (590+) dropped support |
| Kepler | GTX 600, GTX 700 / K80, K40, K20, K10 | `xf86-video-nouveau`, `mesa` | `nvidia-470xx-dkms` (AUR) | Legacy proprietary is AUR-only, unmaintained by Nvidia |
| Fermi | GTX 400, GTX 500 | `xf86-video-nouveau`, `mesa` | `nvidia-390xx-dkms` (AUR) | Legacy proprietary is AUR-only, unmaintained by Nvidia |
| Tesla G | GeForce 8xxx/9xxx, GT 2xx | `xf86-video-nouveau`, `mesa` | `nvidia-340xx-dkms` (AUR) | Legacy proprietary is AUR-only, unmaintained by Nvidia |
| Pre-Tesla | GeForce 6/7 and older | `xf86-video-nouveau`, `mesa` | No proprietary option |

## AMD

| Generation | Example cards | Recommended packages | Notes |
|---|---|---|---|
| RDNA 1-4 / Vega / Polaris / GCN 1.2-1.3 | RX 9000–5000, Vega, RX 400-590, R9 285/380/390/Fury | `mesa`, `xf86-video-amdgpu`, `vulkan-radeon` | `rocm-opencl-runtime` added for RDNA3/4; `amdvlk` removed (discontinued upstream) |
| GCN 1.0-1.1 | HD 7xxx, R7/R9 2xx | `mesa`, `xf86-video-amdgpu` | Needs `amdgpu.si_support=1` / `amdgpu.cik_support=1` kernel param |
| Evergreen/NI | HD 5000-6000 | `mesa`, `xf86-video-ati` | Uses `radeon` kernel module, no Vulkan |
| r600/r700 | HD 2000-4000 | `mesa`, `xf86-video-ati` | No Vulkan |
| R300-R500 | Radeon 9500-X1950 | `xf86-video-ati`, `mesa` | No Vulkan |
| R100/R200 | Radeon 7000-8500 (pre-2004) | `xf86-video-ati`, `mesa` | No real 3D acceleration |

## Intel

| Generation | Example cards | Recommended packages | Notes |
|---|---|---|---|
| Arc Battlemage | B570, B580 | `mesa`, `vulkan-intel`, `intel-compute-runtime`, `intel-media-driver` | Uses `xe` kernel driver (kernel ≥ 6.8) |
| Arc Alchemist | A380, A580, A750, A770 | `mesa`, `vulkan-intel`, `intel-compute-runtime`, `intel-media-driver` | `xe` (≥6.8) or `i915`+GuC; Resizable BAR recommended |
| Iris Xe (iGPU) | Tiger/Alder/Raptor Lake | `mesa`, `vulkan-intel`, `intel-media-driver` | — |
| UHD / HD 500-600 | Skylake–Comet Lake | `mesa`, `vulkan-intel`, `intel-media-driver` | — |
| HD 2000-6000 | Sandy/Ivy/Haswell/Broadwell | `mesa`, `libva-intel-driver` | modesetting DDX preferred over `xf86-video-intel` |
| GMA | pre-HD (2004-2008) | `xf86-video-intel`, `mesa` | Very limited 3D |

## S3 Graphics

| Sub-family | Example cards | Recommended packages | Notes |
|---|---|---|---|
| Chrome | Chrome 4xx/5xx, DeltaChrome, ProSavage | `xf86-video-openchrome` (AUR), `mesa` | No hardware 3D, software OpenGL only |
| Savage | Savage 3D/4/2000/MX/IX | `xf86-video-savage` (AUR) | DRI1 only, needs old kernel |
| ViRGE | ViRGE, Trio, Vision | `xf86-video-s3virge` (AUR) | No usable 3D |

## VIA Technologies (integrated)

| Chipset era | Recommended packages | Notes |
|---|---|---|
| UniChrome / UniChrome Pro / Chrome 9 | `xf86-video-openchrome` (AUR), `mesa` | No hardware 3D, no Vulkan |

## Virtual machines

| Hypervisor | Recommended packages |
|---|---|
| VirtualBox | `virtualbox-guest-utils`, `virtualbox-guest-modules-arch` |
| VMware | `open-vm-tools`, `xf86-video-vmware` |
| QEMU/KVM | `mesa`, `spice-vdagent` |

## Apple Silicon (Asahi Linux)

| Recommended packages | Notes |
|---|---|
| `mesa`, `linux-asahi` (AUR), `asahi-scripts` (AUR) | Needs Asahi pacman repo added first |

---
Only `recommended: true` packages listed above (default AUTO preference).
Full list per GPU (including optional lib32/DKMS/AUR alternates) shown by
running `hyprvisor` directly.
