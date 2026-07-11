# AGENTS.md

This file provides project-scoped guidance to AI agents and contributors working in this repository.

## Project Overview

`ewcalc` is a cross-platform Electronic Warfare engineering calculator. Three strict layers, each depending only on the layer below: a pure C++20 calculation core (`libew`), a platform-agnostic presenter/viewmodel layer (`ewpresenter`), and native-UI frontends per platform (SwiftUI / Qt6 / WinUI 3). See Architecture below for details.

## Session Start

**Requires CMake ≥ 3.20.** Before building, confirm the toolchain for this platform (see Platform-Specific Notes) is installed. On macOS, always unset Homebrew LLVM environment overrides first — Homebrew sets `CC`/`CXX`/`LDFLAGS` to Homebrew LLVM's libc++, which is ABI-incompatible with the macOS 13.0 deployment target used by the macOS frontend:

```bash
unset LDFLAGS CPPFLAGS CC CXX
```

The `scripts/build-macos.sh` script does this automatically; only needed if invoking CMake directly.

## Build Commands

The default CMake build produces `libew`, `ewpresenter`, and the test suite. To include a platform GUI target, set `EWCALC_BUILD_FRONTEND=ON` or use the platform build scripts instead (see Platform-Specific Notes).

### Build (core libs + presenter harness)
```bash
# macOS/Linux (single-config generators: specify build type at configure time)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```
```powershell
# Windows (multi-config generator: build type set at build time)
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel
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
```bash
bash scripts/build-macos.sh [--config Debug|Release] [--package]
bash scripts/build-linux.sh [--config Debug|Release] [--package deb|rpm|appimage]
scripts\build-windows.ps1 [-Config Release]
```

### CMake options
- `EWCALC_BUILD_TESTS` (default `ON`) — enables the test suite. The platform build scripts pass `-DEWCALC_BUILD_TESTS=OFF` for speed; re-enable for test runs.
- `EWCALC_BUILD_FRONTEND` (default `OFF`) — builds the native GUI target via CMake; normally driven by the platform scripts instead.

## Platform-Specific Notes

- **macOS:** Xcode (with Swift and SwiftUI support) from the Mac App Store. Minimum deployment target: macOS 13.0. For the core libs and tests only (no GUI), Xcode Command Line Tools (`xcode-select --install`) are sufficient.
- **Linux:** Qt6 base development libraries (`apt install qt6-base-dev` on Debian/Ubuntu, or equivalent). A C++20 compiler (GCC ≥ 12 or Clang ≥ 14) and CMake ≥ 3.20 are also required. The Qt6 frontend is feature-complete for the current calculator set.
- **Windows:** Visual Studio 2022 with the C++ and Windows App SDK workloads (for WinUI 3 support). Install from https://aka.ms/vs/17/release/vs_buildtools.exe, `winget install Microsoft.VisualStudio.2022.Community`, or `choco install visualstudio2022`. CMake ≥ 3.20: `winget install Kitware.CMake` or `choco install cmake`.

### Windows toolchain setup

> **Windows tool paths vary** by installation method (direct installer, `winget`, `chocolatey`, Microsoft Store, etc.). The paths below are common defaults — adjust for your installation. VS Build Tools and full VS editions use different default directories.

Activate the MSVC toolchain once per PowerShell session before building:

```powershell
# Default path for VS 2022 Build Tools. For full VS (Community/Professional/Enterprise),
# replace "BuildTools" with your edition under "C:\Program Files\Microsoft Visual Studio\2022\".
$vcvars = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
# Auto-detect any edition instead:
# $vsPath = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -property installationPath
# $vcvars = "$vsPath\VC\Auxiliary\Build\vcvars64.bat"
$envVars = cmd /c "`"$vcvars`" > nul && set"
foreach ($line in $envVars) {
    if ($line -match "^([^=]+)=(.*)$") {
        [System.Environment]::SetEnvironmentVariable($Matches[1], $Matches[2], 'Process')
    }
}
```

**One-time setup:**
- Visual Studio 2022 Build Tools (not full IDE) is sufficient. Install from https://aka.ms/vs/17/release/vs_buildtools.exe, `winget install Microsoft.VisualStudio.2022.BuildTools`, or `choco install visualstudio2022buildtools`.
  - Build Tools default path: `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\`
  - Full VS default path: `C:\Program Files\Microsoft Visual Studio\2022\{edition}\`
- **Smart App Control must be Off** (Windows Security → App & Browser Control → SAC settings). SAC blocks locally compiled executables and cannot be re-enabled without a Windows reset.
- CMake ≥ 3.20: https://cmake.org/download/, `winget install Kitware.CMake`, or `choco install cmake`.

## Architecture

Three strict layers — each layer only depends on layers below it:

```
frontend/{macos,linux,windows}   ← platform-native UI (SwiftUI / Qt6 / WinUI 3)
         ↓
bridge                           ← plain-C API over ewpresenter (platform-agnostic)
         ↓
ewpresenter                      ← presenter/viewmodel (platform-agnostic C++20)
         ↓
libew                            ← pure calculation library (no UI, no external deps)
```

`bridge` is optional: only the macOS Swift frontend consumes it today (Swift
cannot import C++ directly). Linux and Windows frontends link `ewpresenter`
directly.

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
5. `output().valid` is `false` whenever any input has a non-`none` FieldError. Exception: `LocationPresenter` has independent AOA, TDOA, and EEP sub-sections, so `valid` remains `true` when at least one sub-section can still produce output; invalid sub-sections dash only their own formatted strings.

No platform types cross the ewpresenter boundary. Frontends bind to `set_on_change` and read from `Output`.

`formatter.h` / `formatter.cpp` provide shared formatting helpers used by all presenters.

### bridge

A plain-C API (`bridge/ewcalc_bridge.h/.cpp`) over `ewpresenter`: opaque
handles, value-type output structs with fixed-size string fields, and C
function-pointer callbacks. Lives at the top level (sibling to `libew`,
`ewpresenter`, `frontend`) since it's platform-agnostic and consumed by both
`ewpresenter/tests/test_bridge.cpp` and the macOS Swift frontend.

### Test framework

Tests use a zero-dependency framework in `libew/tests/test_main.h`. Each test file is an independent executable. Key macros: `TEST_MAIN()`, `RUN_TEST(fn)`, `ASSERT_NEAR(actual, expected, tol)`, `ASSERT_TRUE(expr)`.

## Coding Conventions

- C++20, `-Wall -Wextra -Wpedantic -Werror` (GCC/Clang) or `/W4 /WX /permissive-` (MSVC).
- `#pragma once` throughout.
- No external dependencies in `libew` or `ewpresenter`.
- All quantity types from `libew::units` — never use bare `double` for RF quantities in these layers.
