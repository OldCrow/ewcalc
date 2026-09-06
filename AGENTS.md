# AGENTS.md

This file provides project-scoped guidance to AI agents and contributors working in this repository.

## Project Overview

`ewcalc` is a cross-platform Electronic Warfare engineering calculator. Three strict layers, each depending only on the layer below: a pure C++20 calculation core (`libew`), a platform-agnostic presenter/viewmodel layer (`ewpresenter`), and native-UI frontends per platform (SwiftUI / Qt6 / WinUI 3). See Architecture below for details.

## Session Start

**Requires CMake ≥ 3.25.** Before building, confirm the toolchain for this platform (see Platform-Specific Notes) is installed. On macOS, always unset Homebrew LLVM environment overrides first — Homebrew sets `CC`/`CXX`/`LDFLAGS` to Homebrew LLVM's libc++, which is ABI-incompatible with the macOS 13.0 deployment target used by the macOS frontend:

```bash
unset LDFLAGS CPPFLAGS CC CXX
```

The `scripts/build-macos.sh` script does this automatically; only needed if invoking CMake directly.

## Agent Workflow

- When reviewing repository state or "what's changed" (e.g., syncing after time away, catching up on a branch), start with `git diff --stat` and `git log` rather than reading full file contents. Read complete files only for items you've determined are directly relevant to the task at hand.
- For any subagent expected to run more than ~30 minutes, structure its brief to report interim progress at natural milestones (e.g., after each major deliverable) rather than running silently to a single final report.

## Build Commands

The default CMake build produces `libew`, `ewpresenter`, and the test suite. To include a platform GUI target, set `EWCALC_BUILD_FRONTEND=ON` or use the platform build scripts instead (see Platform-Specific Notes).

### Build (core libs + presenter harness)
```bash
# macOS/Linux
cmake --preset release
cmake --build build --parallel
```
```powershell
# Windows (multi-config generator: build type set at build time).
# Match -G to the installed VS: "Visual Studio 17 2022" or "Visual Studio 18 2026".
# scripts/find-msbuild.ps1 locates MSBuild only; it does not pick the generator.
cmake --preset release -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel
```
Manual alternative (no preset): `cmake -B build -DCMAKE_BUILD_TYPE=Release` (macOS/Linux) or
`cmake -B build -G "Visual Studio 17 2022" -A x64` (Windows).

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
- `EWCALC_BUILD_FRONTEND` (default `OFF`) — registers the `ewcalc_frontend` convenience target. On Windows it drives MSBuild; on macOS it configures and builds the standalone Xcode frontend project with ad-hoc signing (signed/packaged builds stay with `scripts/build-macos.sh`); on Linux it is informational only — the Qt6 frontend builds via `scripts/build-linux.sh`. The `frontend` preset (below) sets it `ON` in its own binary dir.

### CMake standard

Full rules: [CMake House Style](https://github.com/OldCrow/standards/blob/main/CMAKE-HOUSE-STYLE.md)
in the fleet standards repo; this section is self-sufficient for this repo. ewcalc deviations
(app, not a library — subprojects are never embedded elsewhere):
- Unconditional `-Werror` on ewcalc's own targets (no `PROJECT_IS_TOP_LEVEL`
  gate needed — there is no embedding consumer).
- Directory-scope coverage flags before `add_subdirectory` for
  `EWCALC_BUILD_COVERAGE`, deliberately — cross-cutting instrumentation, not
  per-target build config.
- No `install()` — platform scripts (`scripts/build-*.sh`/`.ps1`) own
  packaging (`.dmg`/`.deb`/`.rpm`/AppImage/`.msix`), not CMake.
- Presets (`CMakePresets.json`, schema 6, min CMake 3.25): `release` →
  `build/`, `debug` → `build-debug/`, `rel-with-debug` →
  `build-relwithdebinfo/`, plus the `frontend` extra → `build-frontend/`
  (Release + `EWCALC_BUILD_FRONTEND=ON`, own binaryDir so toggling the
  frontend never leaves a sticky cache variable in `build/`). No `generator`
  field in any preset — pass `-G` explicitly on Windows.

## Platform-Specific Notes

- **macOS:** Xcode (with Swift and SwiftUI support) from the Mac App Store. Minimum deployment target: macOS 13.0. For the core libs and tests only (no GUI), Xcode Command Line Tools (`xcode-select --install`) are sufficient.
- **Linux:** Qt6 base development libraries (`apt install qt6-base-dev` on Debian/Ubuntu, or equivalent). A C++20 compiler (GCC ≥ 12 or Clang ≥ 14) and CMake ≥ 3.25 are also required. The Qt6 frontend is feature-complete for the current calculator set.
- **Windows:** Visual Studio 2022 or newer (e.g. VS 18 2026) with the C++ and Windows App SDK workloads (for WinUI 3 support). Install from https://aka.ms/vs/17/release/vs_buildtools.exe, `winget install Microsoft.VisualStudio.2022.Community`, or `choco install visualstudio2022`. CMake ≥ 3.25: `winget install Kitware.CMake` or `choco install cmake`.

### Windows toolchain setup

> **Windows tool paths vary** by installation method (direct installer, `winget`, `chocolatey`, Microsoft Store, etc.) and VS version/edition. The paths below are common defaults — adjust for your installation. VS Build Tools and full VS editions use different default directories, and the version-number path segment (`2022`, `18`, ...) varies by release.

`scripts/build-windows.ps1` and the CMake `EWCALC_BUILD_FRONTEND` target locate MSBuild automatically via `scripts/find-msbuild.ps1` (vswhere first, falling back to a filesystem scan of the standard VS install roots) — this is generic across VS versions/editions and isn't tied to any one machine's install. If `vswhere.exe` (`...\Installer\vswhere.exe`) is present but stale relative to an in-place VS upgrade, it can silently report no installations; `find-msbuild.ps1`'s fallback handles that automatically, but a Visual Studio Installer repair ("More" → "Repair") will also fix vswhere itself if you hit related issues outside this repo's scripts.

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
- CMake ≥ 3.25: https://cmake.org/download/, `winget install Kitware.CMake`, or `choco install cmake`.

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

The conventions below are scoped per target — the frontends are Swift, C#, and Qt6/C++ respectively, and only the Core rules apply repo-wide.

### Core (libew / ewpresenter / bridge)
- C++20, `-Wall -Wextra -Wpedantic -Werror` (GCC/Clang) or `/W4 /WX /permissive-` (MSVC).
- `#pragma once` throughout.
- No external dependencies in `libew` or `ewpresenter`.
- All quantity types from `libew::units` — never use bare `double` for RF quantities in these layers.
- Static analysis: `scripts/lint-cpp.sh` runs clang-tidy (rules in `.clang-tidy`, repo root) and cppcheck against `libew/`, `ewpresenter/`, and `bridge/`. Mirrors the `static-analysis` CI job — run it locally before pushing.

### macOS Frontend (Swift / SwiftUI)
- One adapter class per presenter domain under `frontend/macos/app/Adapters/` (`PropagationAdapter`, `LinkAdapter`, `ReceiverAdapter`, `JammingAdapter`, `LocationAdapter`, `RadarAdapter`, `DetectionAdapter`, `DopplerAdapter`, `DigitalAdapter`, `AntennaAdapter`), each an `ObservableObject` wrapping the C bridge (`bridge/ewcalc_bridge.h`) — Swift cannot import C++ directly. Adapters are `let` properties of `EwCalcStore` (a `@StateObject` owned by the app), so `Unmanaged.passUnretained` in the C callback is safe only as long as adapters keep that lifetime; switch to `passRetained` and clear the callback in `deinit` before giving any adapter a shorter lifetime.
- Views live under `frontend/macos/app/Views/`.
- Static analysis: `scripts/lint-macos.sh` runs SwiftLint (`--strict`, rules in `.swiftlint.yml`, repo root) against `frontend/macos/app/`. `colon`/`comma`/`comment_spacing` are disabled and `identifier_name`/`type_name` carry an `excluded` list — see comments in `.swiftlint.yml` for why (the codebase's deliberate vertical-alignment style, Doxygen-style `///<` comments, and short unit-abbreviation parameter names like `km`/`db` are intentional, not lint debt). If SwiftLint isn't on `PATH`: on current macOS `brew install swiftlint` is bottled and fine; on macOS 13/Ventura only, Homebrew has no bottle for this formula — it would build the full Swift toolchain from source, which is impractically slow and can fail (see `fix-homebrew-source-build` skill) — so there, download the portable prebuilt binary from the official SwiftLint GitHub release (`portable_swiftlint.zip`) and place it on `PATH` instead.

### Windows Frontend (C# / WinUI 3)
- Static analysis: `.editorconfig` (repo root) enables Roslyn analyzers via `EnableNETAnalyzers`/`AnalysisLevel=latest-recommended`/`EnforceCodeStyleInBuild` in `ewcalc-winui.csproj` — severities default to warnings, not build-breaking, matching the C++ core's mirror-CI-locally philosophy but not (yet) its `-Werror` strictness; tighten via `.editorconfig` severity overrides once the codebase is verified clean against a rule. CI/local verification runs only `dotnet format style <sln> --no-restore --verify-no-changes` (see `.github/workflows/ci.yml`) — never bare `dotnet format` / `dotnet format whitespace`, since this codebase's deliberate multi-space vertical alignment (same convention as `.swiftlint.yml`'s exclusions) would otherwise be collapsed; `dotnet format`'s `analyzers` subcommand and standalone-csproj invocation also cannot reliably resolve `ewpresenter.net`'s C++/CLI types outside a full solution build context. Roslyn analyzer diagnostics (CA rules) are gated by the normal build instead.
- Interop with the native core goes through a C++/CLI adapter DLL, `ewpresenter.net` (`frontend/windows/ewcalc-winui/ewpresenter.net/`), with one adapter class per presenter domain (10 total: Antenna, Detection, Digital, Doppler, Jamming, Link, Location, Propagation, Radar, Receiver).
- Pattern: `NativeCallbacks.h` is a purely-native header (zero managed types) defining one `Make*CB` factory per presenter domain; each factory returns a `std::function` wrapping a plain C function pointer + `void*` cookie. These lambdas capture only native types and are compiled under `#pragma managed(push, off)` / `(pop)`, since the callback wiring must stay outside managed code. Each adapter (e.g. `AntennaAdapter`) allocates a `GCHandle` to itself as the cookie, registers a static native dispatch function (e.g. `AntennaDispatch`) via `presenter_->set_on_change(...)`, and that dispatch function resolves the `GCHandle` back to the managed instance to fire a .NET event. `MarshalHelper.h` centralizes `FieldError` → `FieldValidationError` enum mapping and UTF-8 string marshaling (explicit byte-decode, since `marshal_as` uses the ANSI code page and garbles multi-byte UTF-8).
- Read first: `NativeCallbacks.h` (the callback pattern itself), then `AntennaAdapter.h`/`.cpp` as the simplest concrete adapter, before touching any other adapter.
- Field colour-coding is implemented via `IValueConverter` on `BorderBrush` (`Helpers/FieldErrorConverter.cs` + `{x:Bind ...Error, Mode=OneWay, Converter=...}` on every input control) — the classic-binding approach, after an earlier dependency-property attempt crashed at startup (#62, closed). Keep new inputs on this pattern.
- The app must run packaged: the raw `ewcalc-winui.exe` crashes at startup in the WinAppSDK deployment auto-initializer (`REGDB_E_CLASSNOTREG`) because the project doesn't set `WindowsPackageType=None`. For a dev run, register the loose layout: `Add-AppxPackage -Register <bin>\AppxManifest.xml` (remove any installed MSIX of the app first), then launch via `shell:appsFolder\<PackageFamilyName>!App`.

### Linux Frontend (Qt6 / C++)
- One page class per presenter domain under `frontend/linux/src/pages/` (plus `ReferencePage`, a static help page with no presenter), hosted by `MainWindow` (sidebar `QListWidget` navigation + `QStackedWidget` page area).
- Static analysis: `scripts/lint-linux.sh` runs cppcheck (`--error-exitcode=1`) against `frontend/linux/`. Qt's macro-heavy style (`Q_OBJECT`, signal/slot syntax) does not require suppressions or a Qt-aware ruleset — verified clean.

## CI / Validation

Fleet-wide workflow rules (runner budget, bounded parallelism, ISA hazards on
hosted runners, action pinning):
[CI House Style](https://github.com/OldCrow/standards/blob/main/CI-HOUSE-STYLE.md).

Three workflows: `ci.yml`, `codeql.yml`, and `lint-workflows.yml`
(actionlint + zizmor at `--min-severity medium`, on workflow-file changes
only).

`ci.yml` runs one job per frontend — Windows/WinUI 3, macOS/SwiftUI on
Apple Silicon, Linux/Qt6 — plus sanitizers (ASan/UBSan), static analysis
(clang-tidy + cppcheck), Doxygen docs, and coverage. The frontend jobs each
own their platform's packaging: MSIX, signed+notarized DMG, AppImage, and
.deb, each with a build-provenance attestation.

Gating to keep straight when editing:

- **Packaging, signing, and attestation steps are gated on tag refs**, and
  `release` additionally needs all three frontend builds. A push or PR
  builds and tests only. This is a deliberate trade against the
  runner-budget rule — packaging regressions surface at release time, and
  `workflow_dispatch` is the escape hatch for testing them early.
- The monthly `schedule` canary catches drift with no code change to
  trigger it (runner images, Qt and WinAppSDK NuGet updates). Because
  packaging is tag-gated, scheduled runs re-validate build+test and never
  publish or sign.
- `permissions: contents: read` is workflow-wide; only `release` elevates
  to `contents: write`.

Unlike its five siblings, this repo carries GitHub **rulesets** (verified
2026-07-26): "Protect main" on the default branch requires status checks
and blocks force-push and deletion, and "Protect release tags" blocks
force-push and deletion on `v*`. The `main` ruleset has a user bypass actor
set to `always`, so a direct push reports `Bypassed rule violations ... 3 of
3 required status checks are expected` and **lands without those checks
having run**. Check a real CI run after pushing; the push's own success is
not evidence.

## Open Items
See PLAN.md for current status, in-progress work, and open questions.
