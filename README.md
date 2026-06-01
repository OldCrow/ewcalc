# ewcalc — EW Engineering Calculator

A multi-platform electronic warfare engineering calculator covering RF link budgets,
propagation models, receiver performance, jamming analysis, emitter location, and
basic radar — based on the EW101 series by David Adamy.

## Architecture

Three layers with clean separation:

- **`libew`** — pure C++20 calculation library, no UI, no external dependencies
- **`ewpresenter`** — platform-agnostic presenter/viewmodel layer
- **`frontend/`** — platform-native UIs: WinUI 3 (Windows), SwiftUI (macOS), Qt6 (Linux)

## libew modules

| Module | Content |
|--------|---------|
| `core` | Strong-type units (`Dbm`, `Db`, `Km`, `Mhz`, …), constants |
| `propagation` | FSPL, 2-ray ground reflection, Fresnel zone, knife-edge diffraction |
| `antenna` | ERP, gain conversions, beamwidth |
| `link` | One-way link budget, effective range |
| `receiver` | Sensitivity, cascaded noise figure (Friis), SFDR, digital DR |
| `jamming` | J/S ratio, burnthrough range, partial-band optimization |
| `location` | CEP from AOA bearing error, TDOA, and EEP |
| `radar` | Radar range equation, pulse compression, coherent integration gain |

## ewpresenter

Six presenters wrap the `libew` modules for use by any view layer:

| Presenter | Inputs | Key outputs |
|-----------|--------|-------------|
| `PropagationPresenter` | distance, frequency, antenna heights | FSPL, 2-ray loss, Fresnel zone, regime |
| `LinkPresenter` | Tx power/gain, Rx gain, geometry, sensitivity | Received power, link margin, effective range |
| `ReceiverPresenter` | Bandwidth, NF, SNR, stage chain, ADC bits | Sensitivity, cascaded NF, SFDR, digital DR |
| `JammingPresenter` | Signal/jammer ERP, geometry, frequency | J/S ratio, partial-band optimum BW |
| `LocationPresenter` | Bearing error, range, EEP semi-axes | CEP (AOA and EEP methods) |
| `RadarPresenter` | Tx power, gain, RCS, frequency, NF | Max range, two-way loss, PC/integration gain |

Each presenter validates inputs, calls `libew`, and fires a `std::function` callback with formatted output strings. No platform types are exposed.

A console harness (`ewpresenter_harness`) exercises all presenters against default inputs.

## Building

### Core libraries and tests

```
cmake -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

To run the presenter harness:

```
build/bin/Release/ewpresenter_harness
```

### Platform frontends

Platform build scripts are in `scripts/`. Each builds the native libs then the
frontend, and accepts `--config Debug|Release` and (on Linux/macOS) `--package`
to produce a distributable artifact.

| Platform | Script | Package output |
|----------|--------|----------------|
| Windows | `scripts/build-windows.ps1` | (MSIX, future) |
| macOS | `scripts/build-macos.sh` | signed + notarized `.dmg` |
| Linux | `scripts/build-linux.sh` | `.AppImage` |

macOS packaging requires a Developer ID Application certificate in the keychain
and `xcrun notarytool` credentials stored under the `ewcalc-notarytool` profile.

## Current status

**v0.3.0** — All five phases complete.

- Phase 1 ✓ — `libew`: eight calculation modules, full test suite
- Phase 2 ✓ — `ewpresenter`: six presenters, formatter, validation, console harness
- Phase 3 ✓ — Windows frontend (WinUI 3 / C++/WinRT)
- Phase 4 ✓ — macOS frontend (SwiftUI, C bridge, signed and notarized `.dmg`)
- Phase 5 ✓ — Linux frontend (Qt6 Widgets, AppImage)

Release artifacts (`.dmg`, `.AppImage`) are attached to each
[GitHub Release](../../releases).
