# Calculator Qt6

Calculator cu interfață inspirată de macOS: fără bordură de fereastră, colțuri rotunjite, butoane traffic light (roșu, galben, gri) în stânga sus.

## Cerințe

- CMake ≥ 3.16
- Qt6 (Widgets)

## Build

```bash
mkdir build && cd build
cmake ..
make
./calculator
```

## Comenzi

- **AC** – clear total
- **⌫** – șterge ultima cifră
- **%** – procent
- **±** – schimbă semnul
- **,** – zecimal
- **÷ × − +** – operatori (portocaliu)
- **enter** – calculează rezultatul

Butoanele pill (x↔y, R↓, R↑, drop) sunt decorative și nu modifică calculul.
