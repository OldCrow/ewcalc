# ewcalc — EW Engineering Calculator

A multi-platform electronic warfare engineering calculator covering antenna analysis,
RF propagation, link budgets, receiver performance, jamming analysis, emitter location,
radar, and spread-spectrum communications — based on the EW101 series by David Adamy.

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

**v0.6.4** — Bug fixes from follow-on review: finalizer use-after-free in all 8 C++/CLI adapters; `ewp_receiver_stage()`/`ewp_receiver_set_stages()` UB and null-pointer guards; `copy_str` production truncation sentinel; `lpi_advantage_str` missing DASH; `chip_rate < data_rate` DSSS guard; `LocationPresenter` EEP axis error field + independent sub-section validity; `ReceiverPresenter` `system_nf` now derives from cascaded NF; NaN stage NF fix; `RadarAdapter` FieldError properties; `LinkOutput` two-ray-regime booleans; `CallbackBridge.h` dead code removed.

**v0.6.3** — Architecture review remediation: C++/CLI adapter lifetime safety (double-free, use-after-free, stale GCHandle access, constructor exception safety); presenter validation gaps closed (Jamming gains, Receiver stage NF, `num_pulses`/`adc_bits` error fields); C bridge per-field error accessors (`EwpFieldError`); bridge and formatter integration tests; `PresenterBase<Derived>` CRTP base eliminates setter boilerplate across all 8 presenters; cppcheck clean.

**v0.6.2** — Bug fix: `DigitalPresenter` validity split (invalid DSSS inputs no longer suppress Eb/N₀); ewpresenter assertion test suite; harness registered in CI.

**v0.6.0** — Antenna calculator added across all platforms; knife-edge diffraction and SNR↔Eb/N₀ wired up.

- Phase 1 ✓ — `libew`: nine calculation modules, full test harness
- Phase 2 ✓ — `ewpresenter`: eight presenters; all libew functions surfaced
- Phase 3 ✓ — Windows frontend (WinUI 3 / C#) — Antenna page, diffraction input, extended outputs
- Phase 4 ✓ — macOS frontend (SwiftUI) — Antenna page, diffraction input, extended outputs
- Phase 5 ✓ — Linux frontend (Qt6 Widgets) — Antenna page, diffraction input, extended outputs

Release artifacts (`.dmg`, `.AppImage`) are attached to each
[GitHub Release](../../releases).
