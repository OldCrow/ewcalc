# Architecture

Full layer-by-layer detail behind the summary in AGENTS.md.

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

## libew

Header-only public API lives under `libew/include/libew/`. Each domain has its own subdirectory:

- `core/units.h` — strong-type wrappers (`Dbm`, `Db`, `Km`, `Mhz`, …) with compile-enforced arithmetic rules (e.g. `Dbm + Dbm` is a compile error). User-defined literals (`30.0_dBm`, `100.0_MHz`, etc.) are in `libew::units::literals`. Always use these types in `libew` and `ewpresenter` code.
- `core/constants.h` — physical constants.
- One header per domain: `propagation/`, `antenna/`, `link/`, `receiver/`, `jamming/`, `location/`, `radar/`, `digital/`.
- `libew.h` — umbrella include.

## ewpresenter

One presenter class per domain, each following the same pattern:

1. Stores raw `double` inputs with sensible defaults.
2. Each setter validates via `validation.h` helpers (`validate_bounds`, `validate_positive`, etc.) and records a `FieldError` per field.
3. Calls `recompute()`, which runs libew and populates an `Output` struct containing both raw typed values and pre-formatted `std::string` fields (e.g. `fspl_str`).
4. Fires an `std::function<void(const Output&)>` callback (`set_on_change`).
5. `output().valid` is `false` whenever any input has a non-`none` FieldError. Exception: `LocationPresenter` has independent AOA, TDOA, and EEP sub-sections, so `valid` remains `true` when at least one sub-section can still produce output; invalid sub-sections dash only their own formatted strings.

No platform types cross the ewpresenter boundary. Frontends bind to `set_on_change` and read from `Output`.

`formatter.h` / `formatter.cpp` provide shared formatting helpers used by all presenters.

## bridge

A plain-C API (`bridge/ewcalc_bridge.h/.cpp`) over `ewpresenter`: opaque
handles, value-type output structs with fixed-size string fields, and C
function-pointer callbacks. Lives at the top level (sibling to `libew`,
`ewpresenter`, `frontend`) since it's platform-agnostic and consumed by both
`ewpresenter/tests/test_bridge.cpp` and the macOS Swift frontend.

## Test framework

Tests use a zero-dependency framework in `libew/tests/test_main.h`. Each test file is an independent executable. Key macros: `TEST_MAIN()`, `RUN_TEST(fn)`, `ASSERT_NEAR(actual, expected, tol)`, `ASSERT_TRUE(expr)`.

## Frontends

One section per platform frontend. Moved here from AGENTS.md on 2026-09-07:
this is reference material for whoever is working on that frontend, not
steering needed in every session.

## macOS Frontend (Swift / SwiftUI)
- One adapter class per presenter domain under `frontend/macos/app/Adapters/` (`PropagationAdapter`, `LinkAdapter`, `ReceiverAdapter`, `JammingAdapter`, `LocationAdapter`, `RadarAdapter`, `DetectionAdapter`, `DopplerAdapter`, `DigitalAdapter`, `AntennaAdapter`), each an `ObservableObject` wrapping the C bridge (`bridge/ewcalc_bridge.h`) — Swift cannot import C++ directly. Adapters are `let` properties of `EwCalcStore` (a `@StateObject` owned by the app), so `Unmanaged.passUnretained` in the C callback is safe only as long as adapters keep that lifetime; switch to `passRetained` and clear the callback in `deinit` before giving any adapter a shorter lifetime.
- Views live under `frontend/macos/app/Views/`.
- Static analysis: `scripts/lint-macos.sh` runs SwiftLint (`--strict`, rules in `.swiftlint.yml`, repo root) against `frontend/macos/app/`. `colon`/`comma`/`comment_spacing` are disabled and `identifier_name`/`type_name` carry an `excluded` list — see comments in `.swiftlint.yml` for why (the codebase's deliberate vertical-alignment style, Doxygen-style `///<` comments, and short unit-abbreviation parameter names like `km`/`db` are intentional, not lint debt). If SwiftLint isn't on `PATH`: on current macOS `brew install swiftlint` is bottled and fine; on macOS 13/Ventura only, Homebrew has no bottle for this formula — it would build the full Swift toolchain from source, which is impractically slow and can fail (see `fix-homebrew-source-build` skill) — so there, download the portable prebuilt binary from the official SwiftLint GitHub release (`portable_swiftlint.zip`) and place it on `PATH` instead.

## Windows Frontend (C# / WinUI 3)
- Static analysis: `.editorconfig` (repo root) enables Roslyn analyzers via `EnableNETAnalyzers`/`AnalysisLevel=latest-recommended`/`EnforceCodeStyleInBuild` in `ewcalc-winui.csproj` — severities default to warnings, not build-breaking, matching the C++ core's mirror-CI-locally philosophy but not (yet) its `-Werror` strictness; tighten via `.editorconfig` severity overrides once the codebase is verified clean against a rule. CI/local verification runs only `dotnet format style <sln> --no-restore --verify-no-changes` (see `.github/workflows/ci.yml`) — never bare `dotnet format` / `dotnet format whitespace`, since this codebase's deliberate multi-space vertical alignment (same convention as `.swiftlint.yml`'s exclusions) would otherwise be collapsed; `dotnet format`'s `analyzers` subcommand and standalone-csproj invocation also cannot reliably resolve `ewpresenter.net`'s C++/CLI types outside a full solution build context. Roslyn analyzer diagnostics (CA rules) are gated by the normal build instead.
- Interop with the native core goes through a C++/CLI adapter DLL, `ewpresenter.net` (`frontend/windows/ewcalc-winui/ewpresenter.net/`), with one adapter class per presenter domain (10 total: Antenna, Detection, Digital, Doppler, Jamming, Link, Location, Propagation, Radar, Receiver).
- Pattern: `NativeCallbacks.h` is a purely-native header (zero managed types) defining one `Make*CB` factory per presenter domain; each factory returns a `std::function` wrapping a plain C function pointer + `void*` cookie. These lambdas capture only native types and are compiled under `#pragma managed(push, off)` / `(pop)`, since the callback wiring must stay outside managed code. Each adapter (e.g. `AntennaAdapter`) allocates a `GCHandle` to itself as the cookie, registers a static native dispatch function (e.g. `AntennaDispatch`) via `presenter_->set_on_change(...)`, and that dispatch function resolves the `GCHandle` back to the managed instance to fire a .NET event. `MarshalHelper.h` centralizes `FieldError` → `FieldValidationError` enum mapping and UTF-8 string marshaling (explicit byte-decode, since `marshal_as` uses the ANSI code page and garbles multi-byte UTF-8).
- Read first: `NativeCallbacks.h` (the callback pattern itself), then `AntennaAdapter.h`/`.cpp` as the simplest concrete adapter, before touching any other adapter.
- Field colour-coding is implemented via `IValueConverter` on `BorderBrush` (`Helpers/FieldErrorConverter.cs` + `{x:Bind ...Error, Mode=OneWay, Converter=...}` on every input control) — the classic-binding approach, after an earlier dependency-property attempt crashed at startup (#62, closed). Keep new inputs on this pattern.
- The app must run packaged: the raw `ewcalc-winui.exe` crashes at startup in the WinAppSDK deployment auto-initializer (`REGDB_E_CLASSNOTREG`) because the project doesn't set `WindowsPackageType=None`. For a dev run, register the loose layout: `Add-AppxPackage -Register <bin>\AppxManifest.xml` (remove any installed MSIX of the app first), then launch via `shell:appsFolder\<PackageFamilyName>!App`.

## Linux Frontend (Qt6 / C++)
- One page class per presenter domain under `frontend/linux/src/pages/` (plus `ReferencePage`, a static help page with no presenter), hosted by `MainWindow` (sidebar `QListWidget` navigation + `QStackedWidget` page area).
- Static analysis: `scripts/lint-linux.sh` runs cppcheck (`--error-exitcode=1`) against `frontend/linux/`. Qt's macro-heavy style (`Q_OBJECT`, signal/slot syntax) does not require suppressions or a Qt-aware ruleset — verified clean.
