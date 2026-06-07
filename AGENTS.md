# AGENTS.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

## Commands

### Build (core libs + presenter harness)
```
cmake -B build
cmake --build build --config Release
```

### Run all tests
```
ctest --test-dir build -C Release --output-on-failure
```

### Run a single test binary
Each test file compiles to its own executable under `build/bin/tests/`:
```
build/bin/tests/test_propagation
build/bin/tests/test_receiver
# etc.
```

### Run the presenter smoke harness
```
build/bin/ewpresenter_harness
```

### Platform frontend builds
```
bash scripts/build-macos.sh [--config Debug|Release] [--package]
bash scripts/build-linux.sh [--config Debug|Release] [--package deb|rpm|appimage]
scripts\build-windows.ps1 [-Config Release]
```

On macOS, always unset Homebrew LLVM overrides before building (the script does this automatically) to avoid ABI mismatches against the 13.0 deployment target.

## Architecture

Three strict layers — each layer only depends on layers below it:

```
frontend/{macos,linux,windows}   ← platform-native UI (SwiftUI / Qt6 / WinUI 3)
         ↓
ewpresenter                      ← presenter/viewmodel (platform-agnostic C++20)
         ↓
libew                            ← pure calculation library (no UI, no external deps)
```

Both `libew` and `ewpresenter` compile to static libs (`build/lib/`). Platform frontends link against them.

### libew

Header-only public API lives under `libew/include/libew/`. Each domain has its own subdirectory:

- `core/units.h` — strong-type wrappers (`Dbm`, `Db`, `Km`, `Mhz`, …) with compile-enforced arithmetic rules (e.g. `Dbm + Dbm` is a compile error). User-defined literals (`30.0_dBm`, `100.0_MHz`, etc.) are in `libew::units::literals`. Always use these types in `libew` and `ewpresenter` code.
- `core/constants.h` — physical constants.
- One header per domain: `propagation/`, `antenna/`, `link/`, `receiver/`, `jamming/`, `location/`, `radar/`, `digital/`.
- `libew.h` — umbrella include.

### ewpresenter

One presenter class per domain, each following the same pattern:

1. Stores raw `double` inputs with sensible defaults.
2. Each setter validates via `validation.h` helpers (`validate_bounds`, `validate_positive`, etc.) and records a `FieldError` per field.
3. Calls `recompute()`, which runs libew and populates an `Output` struct containing both raw typed values and pre-formatted `std::string` fields (e.g. `fspl_str`).
4. Fires an `std::function<void(const Output&)>` callback (`set_on_change`).
5. `output().valid` is `false` whenever any input has a non-`none` FieldError.

No platform types cross the ewpresenter boundary. Frontends bind to `set_on_change` and read from `Output`.

`formatter.h` / `formatter.cpp` provide shared formatting helpers used by all presenters.

### Test framework

Tests use a zero-dependency framework in `libew/tests/test_main.h`. Each test file is an independent executable. Key macros: `TEST_MAIN()`, `RUN_TEST(fn)`, `ASSERT_NEAR(actual, expected, tol)`, `ASSERT_TRUE(expr)`.

### CMake options
- `EWCALC_BUILD_TESTS` (default `ON`) — enables the test suite. The platform build scripts pass `-DEWCALC_BUILD_TESTS=OFF` for speed; re-enable for test runs.
- `EWCALC_BUILD_FRONTEND` (default `OFF`) — builds the native GUI target via CMake; normally driven by the platform scripts instead.

### C++ conventions
- C++20, `-Wall -Wextra -Wpedantic -Werror` (GCC/Clang) or `/W4 /WX /permissive-` (MSVC).
- `#pragma once` throughout.
- No external dependencies in `libew` or `ewpresenter`.
- All quantity types from `libew::units` — never use bare `double` for RF quantities in these layers.
