# ewcalc — macOS Frontend

SwiftUI frontend for ewcalc. Consumes `libew` and `ewpresenter` through a
plain-C bridge (`ewcalc_bridge`, top-level `bridge/`) that Swift imports via
a bridging header.

## Project structure

```
frontend/macos/
├── app/
│   ├── BridgingHeader.h     Imports ewcalc_bridge.h into Swift
│   ├── ewcalcApp.swift      App entry point; injects EwCalcStore as environment object
│   ├── ContentView.swift    NavigationSplitView shell + AppSection enum (nav order)
│   ├── Info.plist           Bundle metadata; version pulled from CMake via MARKETING_VERSION
│   │
│   ├── Adapters/
│   │   ├── EwCalcStore.swift      Owns all ten adapter instances for the app lifetime
│   │   ├── SavedInputs.swift      Versioned UserDefaults persistence snapshot
│   │   ├── PropagationAdapter.swift
│   │   ├── AntennaAdapter.swift
│   │   ├── LinkAdapter.swift
│   │   ├── ReceiverAdapter.swift
│   │   ├── JammingAdapter.swift
│   │   ├── LocationAdapter.swift
│   │   ├── RadarAdapter.swift
│   │   ├── DetectionAdapter.swift
│   │   ├── DopplerAdapter.swift
│   │   └── DigitalAdapter.swift
│   │
│   └── Views/
│       ├── Shared.swift           ResultRow, InputRow, DiagramSection, and other reusable SwiftUI components
│       ├── PropagationView.swift
│       ├── AntennaView.swift
│       ├── LinkView.swift
│       ├── ReceiverView.swift
│       ├── JammingView.swift
│       ├── LocationView.swift
│       ├── RadarView.swift
│       ├── DetectionView.swift
│       ├── DopplerView.swift
│       ├── DigitalView.swift
│       └── ReferenceView.swift
│
└── CMakeLists.txt    Configures the Swift app bundle; lists SWIFT_SOURCES explicitly
```

The bridge itself lives at the repo-top-level `bridge/` (sibling to `libew`,
`ewpresenter`, and `frontend`), since it is platform-agnostic and consumed by
tests as well as this frontend:

```
bridge/
├── ewcalc_bridge.h      Plain-C API: opaque handles, value-type output structs, callbacks
├── ewcalc_bridge.cpp    Implements the C API on top of ewpresenter C++20 classes
└── CMakeLists.txt       Built as a static lib by the root CMake build
```

## Build

From the repo root:

```
bash scripts/build-macos.sh --config Release
```

The script:
1. Builds native libs (`libew`, `ewpresenter`, `ewcalc_bridge`) with CMake/Makefiles.
2. Configures an Xcode project in `build/macos-frontend/` via `cmake -G Xcode`.
3. Builds the `.app` bundle with `xcodebuild`.

For a local dev build without packaging:

```
bash scripts/build-macos.sh --config Debug
open build/macos-frontend/Debug/ewcalc.app
```

Distribution (signed + notarized DMG) requires a Developer ID Application certificate
in the keychain and `xcrun notarytool` credentials under the `ewcalc-notarytool` profile:

```
bash scripts/build-macos.sh --config Release --package
```

## Key design decisions

### Why a plain-C bridge rather than Obj-C++ or direct C++ import?

Swift cannot import C++ directly without a compatibility shim, and mixing
Obj-C++ with a pure CMake build adds toolchain complexity. A plain-C bridge
(`extern "C"`) is the lightest option: opaque `void*` handles, value-type
output structs with fixed-size `char[]` fields, and C function-pointer
callbacks. Swift imports the header directly via `BridgingHeader.h` with
no Obj-C involved.

### Why NavigationSplitView?

The calculators have different visual weight. A TabView clips labels at
typical window widths and doesn't support grouping. NavigationSplitView
gives a sidebar, grouped sections (Calculators / Reference), and a detail
pane — the natural macOS document-style layout for a multi-calculator app.

### Adapter pattern

Each Swift adapter class wraps the C bridge for one presenter. It:
- Calls `ewp_<module>_create()` on init and `ewp_<module>_destroy()` on deinit.
- Registers a C callback that dispatches back to the main actor via
  `DispatcherQueue`-free Swift concurrency (the bridge fires synchronously,
  so the adapter publishes `@Published` output properties directly).
- Exposes typed setters (`setDistance(_ km: Double)` etc.) that call
  the corresponding `ewp_<module>_set_*` C functions.

### SWIFT_SOURCES must be maintained explicitly

`CMakeLists.txt` lists Swift source files explicitly (not via glob). When
adding a new Swift file, add it to the `SWIFT_SOURCES` list in
`frontend/macos/CMakeLists.txt`.

## Current state (v1.1.0)

Eleven pages: Propagation, Antenna, Link Budget, Receiver, Jamming,
Location, Radar, Detection, Doppler & Resolution, Digital/DSSS, and
Reference.

v1.1.0 (coverage & illustration, #69–#72): new `DetectionView` and
`DopplerView` with their adapters and bridge functions (required SNR via
Albersheim/Shnidman with Swerling 0–4, fluctuation loss, scan timing,
false-alarm rate; Doppler shift, PRF ambiguity, blind speed, range and
cross-range resolution); collapsible `DiagramSection` geometry-diagram
groups on six pages; an SQNR result row on the Receiver page from the
core's DR/SQNR split; and help tooltips filled in on the fields that
lacked them.

v1.0.0 (release readiness, #37): `AntennaView`'s gain `InputRow` lower bound
tightened from -10 to -6.35 dBi to match `AntennaPresenter::set_gain()`'s
validation floor — below -6.35 dBi, `beamwidth_from_gain()` falls outside its
valid domain and the presenter now rejects the input rather than displaying a
beamwidth over 360°.

v0.9.0 relocates the C bridge from `frontend/macos/bridge/` to the top-level,
platform-agnostic `bridge/` (#22) — no Swift-visible API change.

v0.8.0 (UX parity & accessibility, #16/#17/#19/#20/#21):
- Per-field validation errors from `EwpFieldError` are now published by every
  adapter and shown as a red field outline plus a tooltip message (#16).
- `InputRow` clamps out-of-range input to its declared bounds on every
  commit; presenter-side errors remain surfaced for checks a fixed range
  can't express, e.g. semi-minor > semi-major in `LocationView` (#17).
- VoiceOver labels/values added across all input rows, result rows, and the
  few hand-built controls (ADC bits, coherent pulses, receiver stage rows,
  Reference copy buttons) (#19).
- User inputs persist across launches via a versioned `SavedInputs` snapshot
  in `UserDefaults`, restored before the first page renders; a toolbar
  "Reset to Defaults" action restores the presenter's compiled-in defaults
  and clears the saved data (#20).
- Each calculator page has a "Copy Results" toolbar button that copies all
  result fields to the pasteboard as `Label: Value` lines (#21).

v0.7.0 adds `ewp_jamming_rx_gain_signal`/`ewp_jamming_rx_gain_jammer` bridge
getters so `JammingView` reads live presenter values instead of hardcoded
`0.0` (#12); tightens the Receiver stage NF range to 0–30 dB (#9); and
updates `DigitalView` for the renamed required-SNR-for-Eb/N₀ output (#10).

macOS v0.6.6 fixes Receiver stage row state after removal, corrects the SFDR2
help text, and documents the adapter lifetime contract for `passUnretained`.

No macOS bridge or Swift changes in v0.6.5 — all work was in the Windows
C++/CLI adapter headers (see root README).

Bridge fixes in v0.6.4: `ewp_receiver_stage()` bounds check (negative/OOB index UB); `ewp_receiver_set_stages()` null-pointer and negative-count guards; `copy_str` production truncation sentinel; `ewp_location_eep_axis_error()` accessor added.

Bridge improvements in v0.6.3: `EwpFieldError` enum and per-field error
accessors for all 8 presenters (60 functions); `assert` in `copy_str` to
catch future buffer overflows in debug builds; radar output struct field
renames (`pulse_compression_gain_str`, `coherent_integration_gain_str`);
bridge integration test suite added.
