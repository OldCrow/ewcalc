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
| `core` | Strong-type units (`Dbm`, `Db`, `Km`, `Mhz`, `Kelvin`, …), constants |
| `propagation` | FSPL, 2-ray ground reflection, Fresnel zone, knife-edge diffraction, earth bulge, radar horizon range |
| `antenna` | ERP, gain conversions, beamwidth |
| `link` | One-way link budget, effective range |
| `receiver` | Sensitivity, cascaded NF (Friis), SFDR, digital DR, noise temperature conversions |
| `jamming` | J/S ratio, burnthrough range, partial-band optimization |
| `location` | CEP from AOA bearing error, TDOA timing error, and EEP |
| `radar` | Radar range equation, pulse compression, coherent integration gain, LPI advantage |
| `digital` | Eb/N₀ ↔ SNR conversion, DSSS process gain, jamming margin, required J/S |

## ewpresenter

Seven presenters wrap the `libew` modules for use by any view layer:

| Presenter | Inputs | Key outputs |
|-----------|--------|-------------|
| `PropagationPresenter` | distance, frequency, antenna heights | FSPL, 2-ray loss, Fresnel zone, regime, earth bulge, radar horizon |
| `LinkPresenter` | Tx power/gain, Rx gain, geometry, sensitivity | Received power, link margin, effective range |
| `ReceiverPresenter` | Bandwidth, NF, SNR, stage chain, ADC bits | Sensitivity, cascaded NF, system noise temp, SFDR, digital DR |
| `JammingPresenter` | Signal/jammer ERP, geometry, frequency, J/S threshold | J/S ratio, burnthrough range, partial-band optimum BW (N/A when hop range = 0) |
| `LocationPresenter` | Bearing error, range, TDOA timing error, EEP semi-axes | CEP (AOA, TDOA, and EEP methods) |
| `RadarPresenter` | Tx power, gain, RCS, frequency, NF, TB product | Max range, two-way loss, PC gain, coherent integration gain, LPI advantage |
| `DigitalPresenter` | SNR, bandwidth, data rate, chip rate, required Eb/N₀, impl. loss | Eb/N₀, DSSS process gain, jamming margin, required J/S |

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

**v0.5.0** — All three frontends at full feature parity.

- Phase 1 ✓ — `libew`: nine calculation modules, full test harness; assessment fixes applied
- Phase 2 ✓ — `ewpresenter`: seven presenters; all libew functions surfaced
- Phase 3 ✓ — Windows frontend (WinUI 3 / C#) — Digital/DSSS page, Reference panel, extended pages
- Phase 4 ✓ — macOS frontend (SwiftUI) — Digital/DSSS page, Reference panel, tooltips, TDOA baseline
- Phase 5 ✓ — Linux frontend (Qt6 Widgets) — Digital/DSSS page, Reference panel, extended pages

Release artifacts (`.dmg`, `.AppImage`) are attached to each
[GitHub Release](../../releases).
