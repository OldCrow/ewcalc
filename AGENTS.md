# AGENTS.md

This file provides project-scoped guidance to AI agents and contributors working in this repository.

## Project Overview

`ewcalc` is a cross-platform Electronic Warfare engineering calculator. Three strict layers, each depending only on the layer below: a pure C++20 calculation core (`libew`), a platform-agnostic presenter/viewmodel layer (`ewpresenter`), and native-UI frontends per platform (SwiftUI / Qt6 / WinUI 3). See Architecture below for details.

## Session Start

Fleet-wide session-start steps (architecture check, build-path selection):
[Session Start](https://github.com/OldCrow/standards/blob/main/SESSION-START.md).
The steps below are ewcalc's own additions, not a replacement for it.

**Requires CMake ≥ 3.25.** Before building, confirm the toolchain for this platform (see Platform-Specific Notes) is installed. On macOS, always unset Homebrew LLVM environment overrides first — Homebrew sets `CC`/`CXX`/`LDFLAGS` to Homebrew LLVM's libc++, which is ABI-incompatible with the macOS 13.0 deployment target used by the macOS frontend:

```bash
unset LDFLAGS CPPFLAGS CC CXX
```

The `scripts/build-macos.sh` script does this automatically; only needed if invoking CMake directly.

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
# Do not pin -G: CMake auto-selects the newest installed Visual Studio.
# scripts/build-windows.ps1 and CI both configure unpinned; a hard-coded
# "Visual Studio 17 2022" survives an in-place upgrade to VS 2026 as an
# empty husk and still resolves to it.
cmake --preset release
cmake --build build --config Release --parallel
```
Manual alternative (no preset): `cmake -B build -DCMAKE_BUILD_TYPE=Release` (macOS/Linux) or
`cmake -B build` (Windows).

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
  field in any preset, and no local generator pin on Windows either — that is
  the fleet rule, not an ewcalc deviation; see
  [Windows Toolchain](https://github.com/OldCrow/standards/blob/main/WINDOWS-TOOLCHAIN.md)
  §3. The WinUI 3 frontend is built by MSBuild against its own `.sln`, so it
  never depends on which CMake generator is selected.

## Platform-Specific Notes

- **macOS:** Xcode (with Swift and SwiftUI support) from the Mac App Store. Minimum deployment target: macOS 13.0. For the core libs and tests only (no GUI), Xcode Command Line Tools (`xcode-select --install`) are sufficient.
- **Linux:** Qt6 base development libraries **plus Qt SVG** (`apt install qt6-base-dev libqt6svg6` on Debian/Ubuntu, or equivalent). Qt SVG is a separate package that `qt6-base-dev` does not pull in, and the sidebar needs it at *runtime*: nav icons come from `QIcon::fromTheme`, current GNOME themes (Adwaita 46+, Yaru) carry most of those names only as SVG, and without the SVG iconengine `fromTheme` fails silently — the icons are simply absent, with no error. On a minimal aqtinstall setup it is the separate `qtsvg` *archive* (`--archives qtsvg`), not a `-m` module. A C++20 compiler (GCC ≥ 12 or Clang ≥ 14) and CMake ≥ 3.25 are also required. The Qt6 frontend is feature-complete for the current calculator set.
- **Windows:** toolchain floor, install routes, and the CMake version floor
  are in [WINDOWS-TOOLCHAIN.md §1](https://github.com/OldCrow/standards/blob/main/WINDOWS-TOOLCHAIN.md#1-one-time-setup). ewcalc additionally
  needs the **Windows App SDK workload** for WinUI 3, which the standard does
  not cover.

### Windows toolchain setup

The MSVC environment itself — one-time setup, Smart App Control, per-session
`vcvars64` activation, and the CMake version floor — follows
[WINDOWS-TOOLCHAIN.md](https://github.com/OldCrow/standards/blob/main/WINDOWS-TOOLCHAIN.md).
ewcalc additionally needs the Windows App SDK workload for WinUI 3, noted under
prerequisites above.

`scripts/build-windows.ps1` and the CMake `EWCALC_BUILD_FRONTEND` target locate
MSBuild themselves via `scripts/find-msbuild.ps1` — vswhere first, falling back
to a filesystem scan of the standard VS install roots — so they work across VS
versions and editions with no per-machine configuration. A stale `vswhere.exe`
can silently report no installations after an in-place VS upgrade; the fallback
covers that here, and a Visual Studio Installer repair ("More" → "Repair") fixes
vswhere itself for tools outside this repo.

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

`bridge` is optional: only the macOS Swift frontend consumes it (Swift cannot
import C++ directly) — Linux and Windows link `ewpresenter` directly. `libew`
is header-only; both `libew` and `ewpresenter` compile to static libs
(`build/lib/`) that platform frontends link against.

Header layout, the per-presenter validate/recompute/callback pattern, why
`bridge` exists, and the test framework's macros: `docs/ARCHITECTURE.md`.

## Coding Conventions

The conventions below are scoped per target — the frontends are Swift, C#, and Qt6/C++ respectively, and only the Core rules apply repo-wide.

### Core (libew / ewpresenter / bridge)
- C++20, `-Wall -Wextra -Wpedantic -Werror` (GCC/Clang) or `/W4 /WX /permissive-` (MSVC).
- `#pragma once` throughout.
- No external dependencies in `libew` or `ewpresenter`.
- All quantity types from `libew::units` — never use bare `double` for RF quantities in these layers.
- Static analysis: `scripts/lint-cpp.sh` runs clang-tidy (rules in `.clang-tidy`, repo root) and cppcheck against `libew/`, `ewpresenter/`, and `bridge/`. Mirrors the `static-analysis` CI job — run it locally before pushing.

### Frontends

Per-frontend implementation detail — the adapter class per presenter domain,
the C++/CLI interop and native-callback wiring, packaging and startup quirks,
and each frontend's lint script with its deliberate exclusions — lives in
[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md). Read it when working on a
frontend; it is not needed otherwise.

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

## Reading map — load on demand, not preemptively
- Mapping a calculator output to its underlying equation, source, or unit
  convention → `docs/formulas.md`.
- Layer/component detail beyond the summary above (header layout, presenter
  lifecycle, why `bridge` exists, test macros) → `docs/ARCHITECTURE.md`.
- CMake conventions in depth → [CMake House Style](https://github.com/OldCrow/standards/blob/main/CMAKE-HOUSE-STYLE.md).
- CI workflow conventions in depth → [CI House Style](https://github.com/OldCrow/standards/blob/main/CI-HOUSE-STYLE.md).
- MSVC environment setup, Smart App Control, `vcvars64` activation →
  [WINDOWS-TOOLCHAIN.md](https://github.com/OldCrow/standards/blob/main/WINDOWS-TOOLCHAIN.md).
- What each repo document is for, and how they cross-reference →
  [DOC-CONVENTIONS.md](https://github.com/OldCrow/standards/blob/main/DOC-CONVENTIONS.md).
- Session state, decisions, open questions → `PLAN.md`.

## Open Items
See PLAN.md for current status, in-progress work, and open questions.
