# pearOS KScreenLocker

Plasma screen locker customized to match the pearOS dark theme.

## Requirements

- Arch Linux / Manjaro with KDE Plasma 6
- `plasma-workspace` (runtime dependency, already installed with KDE)

No compilation required — this is a pure QML/font/image theme.

## Install

```bash
git clone https://github.com/alxb421/pkgbuilds
cd pkgbuilds/kscreenlocker
make install
```

`make install` builds the package with `makepkg` and installs it with `pacman`.

### Manual install (without make)

```bash
makepkg -sf --noconfirm
sudo pacman -U pearos-kscreenlocker-theme-*.pkg.tar.zst
sudo /usr/share/pearos/apply-lockscreen-theme.sh
```

## After system updates

The package installs a pacman hook at `/usr/share/libalpm/hooks/pearos-kscreenlocker.hook`
that re-applies the theme automatically whenever `plasma-workspace` is updated. No manual
action needed.

## Development workflow

To apply changes quickly without rebuilding the package:

```bash
make deploy
```

This copies QML, fonts, and images directly to the system paths.

## Font configuration

Create `~/.config/extras/lockscreen/style.json`:

```json
{
  "lockscreen": {
    "fontFamily": "SF Pro",
    "fontWeight": "Semibold",
    "fontStyle": "Rails",
    "font-weight": 600
  }
}
```

| Key | Values |
|-----|--------|
| `fontFamily` | `SF Pro`, `New York` |
| `fontStyle` | `Soft`, `Rails`, `Stencil` (SF Pro only) |
| `fontWeight` | `Light`, `Medium`, `Semibold`, `Bold`, `Heavy` |
| `font-weight` | Number 100–900 (overrides `fontWeight` if set) |

## Avatar

The lock screen looks for the user avatar in this order:

1. `~/.face.icon`
2. The path provided by kscreenlocker (`kscreenlocker_userImage`)
3. `/usr/share/extras/.face.icon` (pearOS system default)
4. Initials circle (generated from the first letter of the username)

## Lock / unlock

```bash
# Lock
loginctl lock-session   # or Super+L

# Emergency unlock from a virtual terminal
Ctrl+Alt+F2
loginctl unlock-session
Ctrl+Alt+F1
```
