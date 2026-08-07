# pearOS Window Tinter

A KWin 6 effect that tints window backgrounds with the average color of the
wallpaper behind them — the same vibrancy idea Filer implements for its own
sidebar, applied to every window by the compositor instead.

## How it works

1. `WallpaperSampler` reads the wallpaper path out of
   `plasma-org.kde.plasma.desktop-appletsrc`, decodes it once and downscales it
   to a small grid (`SampleResolution`, 256px long edge by default).
2. For each tracked window the effect maps the window's screen rectangle into
   image coordinates for the wallpaper's fill mode, then downsamples that
   region to a single pixel — a cheap average, the same trick as
   `DesktopWindow::averageColorForRect()` in Filer.
3. The window is redirected into an offscreen texture (`KWin::OffscreenEffect`)
   and drawn with a custom fragment shader that pulls background pixels toward
   that color.

The tint follows the window: move it over a blue part of the wallpaper and it
goes blue. No KWin script, no D-Bus relay — the compositor already knows where
every window is.

## Why the guards exist

macOS solves "tint the background but not the text" structurally. A window's
background is an `NSVisualEffectView`, a surface owned by the WindowServer; the
text and icons are layers *above* it. "Allow wallpaper tinting in windows" only
switches on the tint component of that backdrop, so it can never touch content.

A compositor gets a flat, already-composed client buffer. Background and text
are the same pixels by the time KWin sees them, so the separation has to be
approximated. Two guards do that, both in `shaders/tint_core.frag`:

- **Chroma guard** — saturated pixels are icons, thumbnails, photos. Never
  background. Scaled down between `ChromaLow` and `ChromaHigh`.
- **Edge guard** — high local contrast (4-neighbour luma probe) is text and
  edges. Flat background has none. Scaled down between `EdgeLow` and `EdgeHigh`.
- **Color key** (optional, off by default) — only tint pixels close to
  `KeyColor`. `KeyFromColorScheme` takes that color from KWin's *own* color
  scheme, which is right for Breeze/Qt apps and wrong for GTK, Electron and
  self-skinned apps. Hence `KeyStrength=0` by default.

### Known limits

- A thin untinted halo around glyphs. Invisible at 5–10% tint, visible at 40%.
- Windows whose background is a photo (image viewers, colorful web pages) get
  almost no tint — the guards read them as content. Usually the desired result.
- Pure black backgrounds (terminals) barely shift; `mix()` toward a tint has
  little room at zero luminance.
- Colour management: the shader replaces KWin's generated fragment stage, which
  means it does not carry the `TransformColorspace` path. Fine on SDR, expect
  wrong colours on an HDR output.
- Redirecting a window offscreen costs an extra render pass and blocks direct
  scanout. Blacklist video players (`mpv`, `vlc` are blacklisted by default).
- `OffscreenData` only refills its texture when `m_isDirty` is set, and that
  flag comes from `EffectWindow::windowDamaged` — real client damage. A resize
  reallocates the texture without filling it, so the effect drops the
  `OffscreenData` and re-`redirect()`s on every size change to get a fresh one
  (they start out dirty). That means one texture reallocation per resize step.

## Configuration

No KCM yet. Everything lives in `~/.config/kwinrc` under
`[Effect-PearosTinter]`, all values are percentages:

```ini
[Effect-PearosTinter]
TintAmount=8
ChromaGuard=100
ChromaLow=10
ChromaHigh=35
EdgeGuard=100
EdgeLow=2
EdgeHigh=12
KeyStrength=0
KeyFromColorScheme=true
KeyColor=#1e1e1e
KeyTolerance=12
KeySoftness=15
IncludeDialogs=true
SkipFullScreen=true
SuspendDuringResize=true
Blacklist=mpv,vlc,kwin_wayland,plasmashell
SampleResolution=256
```

`TintAmount=8` matches Filer's real-world defaults and is deliberately subtle —
bump it to 40 while checking that the effect works at all, then put it back.

Apply changes without a relogin:

```bash
qdbus org.kde.KWin /KWin org.kde.KWin.reconfigure
```

## Build and install

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build -j$(nproc)
sudo cmake --install build
```

Then enable it:

```bash
kwriteconfig6 --file kwinrc --group Plugins --key kwin4_effect_pearostinterEnabled true
qdbus org.kde.KWin /KWin org.kde.KWin.reconfigure
```

Or find "pearOS Window Tinter" in System Settings → Window Management →
Desktop Effects.

## Debugging

Effect-side output goes to journald, not to any redirected log:

```bash
journalctl _PID=$(pgrep kwin_wayland) --since "-5 min" --no-pager | grep -i tinter
```

## Roadmap

Path B (this effect) trades fidelity for coverage. Path A — tinting the blur
backdrop *below* the window content, the true macOS equivalent — needs a patched
blur effect and only works for windows that request blur behind.
