# ewcalc — EW Engineering Calculator

[![CI](https://github.com/OldCrow/ewcalc/actions/workflows/ci.yml/badge.svg)](https://github.com/OldCrow/ewcalc/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

A multi-platform electronic warfare engineering calculator covering antenna analysis,
RF propagation, link budgets, receiver performance, jamming analysis, emitter location,
radar, and spread-spectrum communications — based on the EW101 series by David Adamy.

## Getting ewcalc

Pre-built artifacts for all three platforms are attached to each
[GitHub Release](../../releases):

| Platform | Artifact | Notes |
|----------|----------|-------|
| Windows | `.msix` | Unsigned by default; see [`frontend/windows/ewcalc-winui/README.md`](frontend/windows/ewcalc-winui/README.md#msix-packaging-and-signing) for sideloading a self-signed cert |
| macOS | `.dmg` | Signed and notarized — opens directly, no Gatekeeper prompt |
| Linux | `.AppImage` | `chmod +x`, then run directly; no installation required |

To build from source instead, see Building below.

## Architecture

Layered, with clean separation:

- **`libew`** — pure C++20 calculation library, no UI, no external dependencies
- **`ewpresenter`** — platform-agnostic presenter/viewmodel layer
- **`bridge/`** — plain-C API over `ewpresenter`, used by the macOS Swift frontend
  (Swift cannot import C++ directly); Linux and Windows link `ewpresenter` directly
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
| `digital` | Eb/N₀ ↔ SNR conversion (both directions), DSSS process gain, jamming margin, required J/S |

## ewpresenter

Eight presenters wrap the `libew` modules for use by any view layer:

| Presenter | Inputs | Key outputs |
|-----------|--------|-------------|
| `PropagationPresenter` | distance, frequency, antenna heights, obstruction height | FSPL, 2-ray loss, Fresnel zone, regime, earth bulge, radar horizon, knife-edge diffraction loss |
| `AntennaPresenter` | gain, az/el beamwidth, Tx power, frequency | ERP, beamwidth from gain, gain from beamwidth, wavelength |
| `LinkPresenter` | Tx power/gain, Rx gain, geometry, sensitivity | Received power, link margin, effective range |
| `ReceiverPresenter` | Bandwidth, NF, SNR, stage chain, ADC bits | Sensitivity, cascaded NF, system noise temp, SFDR, digital DR |
| `JammingPresenter` | Signal/jammer ERP, geometry, frequency, J/S threshold | J/S ratio, burnthrough range, partial-band optimum BW (N/A when hop range = 0) |
| `LocationPresenter` | Bearing error, range, TDOA timing error, EEP semi-axes | CEP (AOA, TDOA, and EEP methods) |
| `RadarPresenter` | Tx power, gain, RCS, frequency, NF, TB product | Max range, two-way loss, PC gain, coherent integration gain, LPI advantage |
| `DigitalPresenter` | SNR, bandwidth, data rate, chip rate, required Eb/N₀, impl. loss | Eb/N₀, SNR from Eb/N₀, DSSS process gain, jamming margin, required J/S |

Each presenter validates inputs, calls `libew`, and fires a `std::function` callback with formatted output strings. No platform types are exposed.

A console harness (`ewpresenter_harness`) exercises all presenters against default inputs.

## Building

Requires CMake ≥ 3.25 and a C++20 compiler. See [`AGENTS.md`](AGENTS.md#platform-specific-notes)
for per-platform toolchain prerequisites (Xcode, Qt6, Visual Studio, etc.).

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
| Windows | `scripts/build-windows.ps1` | unsigned `.msix` (`-Sign` for signed) |
| macOS | `scripts/build-macos.sh` | signed + notarized `.dmg` |
| Linux | `scripts/build-linux.sh` | `.AppImage` |

macOS packaging requires a Developer ID Application certificate in the keychain
and `xcrun notarytool` credentials stored under the `ewcalc-notarytool` profile.

### Formula reference

See [`docs/formulas.md`](docs/formulas.md) for the equations, units, assumptions, and references behind each calculator output.

### API documentation

`libew` and `ewpresenter` headers use Doxygen-style `///` comments. Build the
HTML API reference locally with:

```
doxygen Doxyfile
```

Output lands in `docs/html/index.html`. CI runs the same command on every push
and pull request and publishes `docs/html/` as a workflow artifact; it does not
publish to GitHub Pages yet (a reasonable future enhancement).

## Contributing

See [`CONTRIBUTING.md`](CONTRIBUTING.md) for the branch/PR workflow, build and
test instructions, and code style. See [`CHANGELOG.md`](CHANGELOG.md) for
release history. Found a security issue? See [`SECURITY.md`](SECURITY.md)
for how to report it privately.

## Current status

All three platform frontends — Windows (WinUI 3), macOS (SwiftUI), and Linux
(Qt6 Widgets) — are feature-complete across all eight calculators plus a
Reference page. See [`CHANGELOG.md`](CHANGELOG.md) for release history.

## License

MIT — see [`LICENSE`](LICENSE).
