# ewcalc — macOS Frontend (Phase 4)

SwiftUI frontend for ewcalc. Consumes `libew` and `ewpresenter` through a
plain-C bridge (`ewcalc_bridge`) that Swift imports via a bridging header.

## Project structure

```
frontend/macos/
├── bridge/
│   ├── ewcalc_bridge.h      Plain-C API: opaque handles, value-type output structs, callbacks
│   ├── ewcalc_bridge.cpp    Implements the C API on top of ewpresenter C++20 classes
│   └── CMakeLists.txt       Built as a static lib by the native CMake step
│
├── app/
│   ├── BridgingHeader.h     Imports ewcalc_bridge.h into Swift
│   ├── ewcalcApp.swift      App entry point; injects EwCalcStore as environment object
│   ├── ContentView.swift    NavigationSplitView shell + AppSection enum (nav order)
│   ├── Info.plist           Bundle metadata; version pulled from CMake via MARKETING_VERSION
│   │
│   ├── Adapters/
│   │   ├── EwCalcStore.swift      Owns all nine adapter instances for the app lifetime
│   │   ├── PropagationAdapter.swift
│   │   ├── AntennaAdapter.swift
│   │   ├── LinkAdapter.swift
│   │   ├── ReceiverAdapter.swift
│   │   ├── JammingAdapter.swift
│   │   ├── LocationAdapter.swift
│   │   ├── RadarAdapter.swift
│   │   └── DigitalAdapter.swift
│   │
│   └── Views/
│       ├── Shared.swift           ResultRow, InputRow, and other reusable SwiftUI components
│       ├── PropagationView.swift
│       ├── AntennaView.swift
│       ├── LinkView.swift
│       ├── ReceiverView.swift
│       ├── JammingView.swift
│       ├── LocationView.swift
│       ├── RadarView.swift
│       ├── DigitalView.swift
│       └── ReferenceView.swift
│
└── CMakeLists.txt    Configures the Swift app bundle; lists SWIFT_SOURCES explicitly
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

## Current state (v0.6.3)

Nine calculator pages: Propagation, Antenna, Link Budget, Receiver,
Jamming, Location, Radar, Digital/DSSS, and Reference.

Bridge improvements in v0.6.3: `EwpFieldError` enum and per-field error
accessors for all 8 presenters (60 functions); `assert` in `copy_str` to
catch future buffer overflows in debug builds; radar output struct field
renames (`pulse_compression_gain_str`, `coherent_integration_gain_str`);
bridge integration test suite added.
