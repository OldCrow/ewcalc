# ewcalc — EW Engineering Calculator

A multi-platform electronic warfare engineering calculator covering RF link budgets,
propagation models, receiver performance, jamming analysis, emitter location, and
basic radar — based on the EW101 series by David Adamy.

## Architecture

Three layers with clean separation:

- **`libew`** — pure C++20 calculation library, no UI, no external dependencies
- **`ewpresenter`** — platform-agnostic presenter/viewmodel layer (Phase 2)
- **`frontend/`** — platform-native UIs: WinUI 3 (Windows), AppKit (macOS), Qt6 (Linux)

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

## Building

```
cmake -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

## Current status

Phase 1 — libew core library and tests.
