# Contributing to ewcalc

## Workflow

1. Branch off `main` (e.g. `feat/<short-description>`, `fix/<short-description>`).
2. Open a pull request against `main`. CI (`.github/workflows/ci.yml`) must pass:
   `build-windows`, `build-macos`, `build-linux`, `sanitizers`, `static-analysis`, and `coverage`.
3. PRs are squash-merged. The squash commit message is `<PR title> (#<PR number>)`
   (see `git log` for examples, e.g. `v0.9.0 — Infrastructure & Hardening (#36)`).
4. Releases follow a squash-merged milestone PR, then a separate
   `chore: bump version to vX.Y.Z` commit and tag on `main`. Add a matching
   entry to `CHANGELOG.md`.

## Build prerequisites

See `AGENTS.md` for per-platform prerequisites (CMake, compiler, and
per-frontend toolchain requirements) and the macOS Homebrew-LLVM caveat under
"Session Start". Don't duplicate that guidance here — it's kept in one place
to avoid drift.

## Running tests

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build -C Release --output-on-failure
```

Each test file compiles to its own executable under `build/bin/tests/`
(e.g. `build/bin/tests/test_antenna`) and can be run directly.

## Sanitizer, static analysis, and coverage builds

These mirror the `sanitizers`, `static-analysis`, and `coverage` jobs in
`.github/workflows/ci.yml`. Run them locally before opening a PR that touches
`libew` or `ewpresenter`:

**Sanitizers (ASan/UBSan):**
```bash
cmake -B build-san -DEWCALC_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
cmake --build build-san --parallel
ctest --test-dir build-san --output-on-failure
```

**Static analysis (clang-tidy + cppcheck):** clang-tidy uses the repo's
`.clang-tidy` config (`WarningsAsErrors` set, so any enabled-check violation
fails the run):
```bash
cmake -B build-lint -DEWCALC_BUILD_TESTS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
find libew/src ewpresenter/src -name '*.cpp' -print0 | xargs -0 clang-tidy -p build-lint
cppcheck --enable=warning,style,performance,portability \
  --std=c++20 --language=c++ --inline-suppr --error-exitcode=1 \
  -I libew/include -I ewpresenter/include libew/src ewpresenter/src
```

**Coverage (Clang/llvm-cov, 75% line-coverage gate):**
```bash
cmake -B build-cov -DEWCALC_BUILD_TESTS=ON -DEWCALC_BUILD_COVERAGE=ON \
  -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build build-cov --parallel
ctest --test-dir build-cov --output-on-failure
```
See the `coverage` job in `.github/workflows/ci.yml` for the `llvm-profdata`/`llvm-cov`
report-generation steps.

## Code style

- C++20, `-Wall -Wextra -Wpedantic -Werror` (GCC/Clang) or `/W4 /WX /permissive-` (MSVC).
- `#pragma once` in every header.
- No external dependencies in `libew` or `ewpresenter`.
- Use `libew::units` strong types (`Dbm`, `Db`, `Km`, `Mhz`, …) for every RF
  quantity in `libew` and `ewpresenter` — never a bare `double`.
- Document public API headers with Doxygen-style `///` comments
  (`@file`, `@brief`, `@param`, `@return`), matching the existing headers
  under `libew/include/libew/`.

## Adding a new calculator

Follow the three-layer pattern used by every existing domain. The `antenna`
domain is a complete, working example to copy from at each layer:

1. **`libew`** — add `libew/include/libew/<domain>/<domain>.h` (Doxygen-commented,
   `libew::units` types only) and `libew/src/<domain>.cpp`. Add
   `libew/tests/test_<domain>.cpp` using the framework in
   `libew/tests/test_main.h` (`TEST_MAIN()`, `RUN_TEST`, `ASSERT_NEAR`,
   `ASSERT_TRUE`), with a `// Source: ...` comment above each formula's test
   block. Compare against `libew/include/libew/antenna/antenna.h`,
   `libew/src/antenna.cpp`, and `libew/tests/test_antenna.cpp`.
2. **`ewpresenter`** — add `ewpresenter/include/ewpresenter/<domain>_presenter.h`
   and `ewpresenter/src/<domain>_presenter.cpp`, following the
   `PresenterBase<Derived>` pattern (see `ewpresenter/include/ewpresenter/presenter_base.h`
   and `ewpresenter/include/ewpresenter/antenna_presenter.h`): validated
   setters, a `recompute()` that calls `libew` and fills an `Output` struct
   with formatted strings, and an `on_change` callback.
3. **Per-platform adapter/view** — reference the antenna implementation on
   each platform:
   - macOS: `frontend/macos/app/Adapters/AntennaAdapter.swift`,
     `frontend/macos/app/Views/AntennaView.swift`.
   - Windows: `frontend/windows/ewcalc-winui/ewpresenter.net/AntennaAdapter.cpp/.h`
     (C++/CLI bridge), `frontend/windows/ewcalc-winui/ewcalc-winui/ViewModels/AntennaViewModel.cs`,
     `.../Views/AntennaPage.xaml`.
   - Linux: `frontend/linux/src/pages/AntennaPage.cpp/.h` (Qt6).
4. If the macOS frontend needs the new domain, also add functions to the
   plain-C `bridge/ewcalc_bridge.h/.cpp` API and cover them in
   `ewpresenter/tests/test_bridge.cpp`.
