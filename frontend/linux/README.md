# ewcalc — Linux Frontend (Phase 5)

Qt6 Widgets frontend for ewcalc. Consumes `libew` and `ewpresenter`
directly as C++ static libraries — no interop layer required.

## Project structure

```
frontend/linux/
├── CMakeLists.txt     Finds Qt6, imports native libs, lists sources
└── src/
    ├── main.cpp
    ├── MainWindow.h/.cpp      QMainWindow shell: sidebar QListWidget + QStackedWidget
    ├── PageUtils.h            Shared helpers: makeSpinBox, makeResultRow, field-error styling
    └── pages/
        ├── PropagationPage.h/.cpp
        ├── AntennaPage.h/.cpp
        ├── LinkPage.h/.cpp
        ├── ReceiverPage.h/.cpp
        ├── JammingPage.h/.cpp
        ├── LocationPage.h/.cpp
        ├── RadarPage.h/.cpp
        ├── DigitalPage.h/.cpp
        └── ReferencePage.h/.cpp
```

## Build

From the repo root:

```
bash scripts/build-linux.sh --config Release
```

The script:
1. Builds native libs (`libew`, `ewpresenter`) with CMake.
2. Configures and builds the Qt frontend via `cmake -S frontend/linux`.

Prerequisites:

```
sudo apt-get install build-essential cmake ninja-build qt6-base-dev libgl-dev libopengl0
```

For a local dev build:

```
bash scripts/build-linux.sh --config Debug
build/linux-frontend/ewcalc
```

AppImage packaging (CI only, requires `linuxdeploy`):

```
bash scripts/build-linux.sh --config Release --package appimage
```

## Key design decisions

### Direct C++ presenter access

The Linux frontend holds `ewpresenter` presenter instances directly as
class members — no interop layer, no bridge. Each page constructs its
presenter, calls `set_on_change()` with a lambda that updates result
`QLabel`s, and wires `QDoubleSpinBox::valueChanged` to the presenter
setters. This is the simplest possible architecture when the UI and
native library share the same process and ABI.

### Qt Widgets over Qt Quick/QML

Qt Widgets gives a native-looking desktop UI with minimal setup. Qt Quick
requires a QML runtime and a significantly larger deployment footprint.
For a calculation-focused desktop tool, Widgets' form-like layout (labels,
spinboxes, result rows) is a natural fit and needs no declarative binding
machinery.

### Navigation: sidebar + stacked widget

`MainWindow` uses a `QListWidget` as a sidebar and a `QStackedWidget` for
the page area. Non-selectable header rows (greyed, smaller font) separate
the Calculators and Reference groups. A `navToStack_` index array maps
sidebar row indices to stack indices, skipping header/spacer rows cleanly.

### Field error styling

`PageUtils.h` provides `applyFieldError()` which sets the border colour of
a `QDoubleSpinBox` red when the presenter reports a validation error, and
clears it when the input is valid. This keeps all validation logic in the
presenter layer — the Qt layer only reacts to the `FieldError` value it
receives from the output struct.

## Current state (v0.6.4)

Nine calculator pages: Propagation, Antenna, Link Budget, Receiver,
Jamming, Location, Radar, Digital/DSSS, and Reference.

No frontend changes in v0.6.4 (or v0.6.3) — all work was in the core
library and presenter layer (see root README).
