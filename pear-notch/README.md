# pear-notch

Notch for pearOS Linux (X11): always on top, startup

## Comportament

- **Always on top** — fereastra rămâne deasupra tuturor aplicațiilor
- **Skip taskbar** — won t show in the taskbar
- **Skip Cmd+Tab** — won t show in command+tab
- **No name** — window won t show any name

## Compile

```bash
./buid.sh
```

Deps: **X11**, **Cairo**, **librsvg** (libx11, cairo, librsvg-2.0, pkg-config).

## Run

```bash
./build/pear-notch
```

Stop with `Ctrl+C`.
