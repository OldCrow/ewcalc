# Changelog

All notable changes to this project are documented in this file.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [v1.1.0] — 2026-09-05

Coverage & Illustration release: two new calculators (Detection statistics,
Doppler & Resolution) across all three frontends, static geometry diagrams
on six panes, the DR/SQNR split on the Receiver pane, and the last three
formula-provenance deltas resolved (#68).

### Changed

- Digital dynamic range (Receiver pane) now reports the full-scale-to-LSB
  level ratio `20*log10(2^N)` ≈ 6.02·N dB per Adamy EW103 Sec 4.5.3
  (previously it reported the SQNR 6.02·N + 1.76 dB under the DR label);
  the SQNR is now shown as its own result row. Resolution of the 1.76 dB
  provenance delta by offering both quantities.
- Antenna gain <-> beamwidth rule-of-thumb constant changed from 30000 to
  29000 per Adamy EW103 Sec 3.7 p.70 — his stated choice among the
  Kraus/Tai-Pereira-family constants. Outputs shift by 0.15 dB (gain) /
  ~1.7% (beamwidth); the rule's intrinsic accuracy is ±1-2 dB.
- Radar/radio horizon keeps the exact k=4/3 constant 4.122 (Adamy rounds
  to 4.11); documented as a deliberate difference, no change.
- Windows UX parity pass (post-review): help tooltips on every input and
  result row across all ten panes (previously only the two new panes had
  them), the Propagation regime indicator converted from an InfoBar to a
  standard "Regime" result row (matching Link and the macOS/Linux
  presentation), the always-open "About CEP" InfoBar on Location replaced
  by per-row CEP tooltips (matching macOS), and the navigation pane
  widened so "Doppler & Resolution" no longer truncates.
- Windows field colour-coding gap (#62) closed as already implemented:
  the converter-based error styling (`FieldErrorToBrushConverter` on
  `BorderBrush`) shipped in v0.8.0 covers every input on all pages and is
  now verified at runtime (clean startup, red border on invalid fields,
  matching macOS/Linux). The stale "known limitation" notes in AGENTS.md
  and PLAN.md are removed; the failed dependency-property approach they
  described was superseded, not still pending.

### Added

- Static geometry diagrams (#72): ten hand-authored SVG masters with a
  checked-in PNG render pipeline (`scripts/render-diagrams.sh`),
  displayed as a "Geometry" section on the Jamming (self-protection and
  stand-off geometry), Propagation (two-ray, knife-edge, horizon/earth
  bulge), Location (AOA error fan, EEP vs CEP), Doppler & Resolution
  (resolution cell), Receiver (Friis cascade), and Antenna
  (beamwidth/gain trade) panes on macOS, Linux, and Windows (collapsed
  Expander sections; the PNGs are linked from the shared asset set, not
  duplicated). One theme-neutral transparent asset set; live plotting
  remains explicitly out of scope.
- Doppler & Resolution calculator (#70, #71): two-way Doppler shift,
  unambiguous range, first blind speed and unambiguous velocity (the
  Doppler dilemma product R_u·v_u = c·λ/8 is test-guarded), range
  resolution from waveform bandwidth, and cross-range resolution in
  azimuth and elevation. New `libew::radar` functions, a
  `DopplerPresenter` pane on all three frontends (macOS/Linux via the C
  bridge and Qt; Windows via a `DopplerAdapter` in `ewpresenter.net`),
  and a `format_mps` formatter.
- Detection statistics calculator (#69): required SNR via Albersheim's
  equation (nonfluctuating) and Shnidman's equation (Swerling 0–4),
  fluctuation loss, dwell time, hits per scan, and false-alarm rate.
  New `libew::radar` functions with a `Swerling` enum, a
  `DetectionPresenter` pane on all three frontends (Windows via a
  `DetectionAdapter` in `ewpresenter.net`), a
  `Seconds` unit type, and `format_seconds`/`format_hz`/`format_count`
  formatters. Shnidman's implementation is gated against an exact
  Marcum-Q/noncentral-chi-square oracle (`scripts/detection_oracle.py`);
  worst observed error 0.30 dB over the validation grid.
- `ewcalc_frontend` convenience target on macOS: `EWCALC_BUILD_FRONTEND=ON`
  now configures and builds the Xcode frontend with ad-hoc signing (#66);
  signed/packaged builds remain with `scripts/build-macos.sh`.

## [v1.0.2] — 2026-07-16

Static-analysis parity release: SwiftLint (macOS), a cppcheck baseline
sweep (Linux), and Roslyn analyzers/`dotnet format` (Windows) now match
the existing clang-tidy+cppcheck coverage for the C++ core, closing out
#43, #44, and #45.

### Added

- macOS: SwiftLint wired up as a standalone script (`scripts/lint-macos.sh`,
  matching the `lint-cpp.sh`/`lint-linux.sh` pattern) with a baseline
  `.swiftlint.yml`; codebase runs `--strict` clean (#43).
- Windows: root `.editorconfig`, `EnableNETAnalyzers`/`AnalysisLevel`/
  `EnforceCodeStyleInBuild` in `ewcalc-winui.csproj`, and a
  `dotnet format style` CI verification step (#44).
- `scripts/find-msbuild.ps1`: a shared MSBuild-location helper (vswhere
  first, falling back to a version-sorted filesystem scan of the standard
  VS install roots) used by both `scripts/build-windows.ps1` and
  `CMakeLists.txt`'s `EWCALC_BUILD_FRONTEND` target.

### Fixed

- Linux: all four cppcheck baseline findings resolved;
  `scripts/lint-linux.sh` now gated with `--error-exitcode=1` (#45).
- Windows: all 8 ViewModels now implement `IDisposable` and dispose their
  native `ewpresenter.net` adapter on page navigate-away (via
  `OnNavigatedFrom`) instead of relying on the GC finalizer; also removes
  redundant default-value field initializers flagged by the new analyzers
  (#44).
- MSBuild detection (`scripts/build-windows.ps1`,
  `EWCALC_BUILD_FRONTEND`) no longer fails silently when `vswhere.exe`'s
  cached instance-state reader lags the actual installer version after an
  in-place Visual Studio upgrade.

## [v1.0.1] — 2026-07-12

Icon/packaging patch release: Windows MSIX and Linux app icons were both
missing due to build/packaging gaps, and Linux gains `.deb` packaging.

### Added

- Linux `.deb`/`.rpm` packaging: `frontend/linux/CMakeLists.txt` now installs
  `assets/linux/ewcalc.desktop` and the `hicolor` icon set to their standard
  system locations and configures CPack (Debian and RPM metadata).
  `scripts/build-linux.sh --package deb|rpm` invokes `cpack -G DEB`/`-G RPM`
  directly rather than the previously nonfunctional `package` build target
  (no CPack config existed at all beforehand, so `--package deb`/`--package rpm`
  always failed).

### Fixed

- Linux showed a generic "gears" icon in the taskbar/window switcher instead
  of the EW Calculator icon: neither `main.cpp` nor `MainWindow.cpp` ever
  called `setWindowIcon()`, so the icon depended entirely on the window
  manager matching `WM_CLASS` to the installed `.desktop` entry — unreliable
  outside a fully XDG-integrated session (e.g. an un-integrated AppImage).
  The icon is now embedded via a Qt resource file
  (`frontend/linux/resources/icons.qrc`) at multiple sizes and set explicitly
  via `QApplication::setWindowIcon()`.
- Windows MSIX packages had no Start Menu/taskbar icon: `ewcalc-winui.csproj`
  had no item copying `Assets\**` to the build output, so
  `scripts/build-windows.ps1`'s fallback packaging path (used whenever the
  single-project MSIX `AppX` staging folder isn't present) produced a
  package whose manifest referenced logo/icon files that were never
  included as payload. Added an explicit `Content` item with
  `CopyToOutputDirectory=PreserveNewest` for `Assets\**`.
- Linux AppImage failed to launch on distros older than the CI build image,
  first with `version 'GLIBCXX_3.4.31' not found` (from `libstdc++`), then
  with `version 'GLIBC_2.38' not found` (from a transitively-bundled
  `libgcrypt.so.20`) once the first symptom was patched. Root cause: the
  AppImage was built on `ubuntu-24.04` (glibc 2.39 / GCC 13), and every
  library `linuxdeploy` bundles is linked against that build system's glibc/
  GLIBCXX — patching one bundled library at a time doesn't fix the next one
  that trips the same failure mode. `ci.yml`'s `build-linux` job now runs on
  `ubuntu-22.04` (the oldest Ubuntu LTS with `qt6-base-dev` via apt) so the
  whole AppImage is built against an older, more portable baseline.
  `scripts/build-linux.sh` still force-bundles `libstdc++`/`libgcc_s` via
  `linuxdeploy`'s `--library` flag (matching the existing `libOpenGL.so.0`
  workaround) as defense in depth. `ci.yml`'s Linux AppImage packaging steps
  also now run on `workflow_dispatch` (previously tag-push only), matching
  Windows/macOS, so a fix can be rebuilt without cutting a new tag.
- Linux Location tab had no sidebar icon: `"find-location"` isn't a standard
  XDG icon name and resolved to nothing on most icon themes; switched to
  `"mark-location"`, which is widely shipped.
- Linux "Reset to Defaults" was only reachable via the File menu, unlike
  macOS (toolbar button) and Windows (nav-pane footer button). Added a
  matching always-visible button below the sidebar.

## [v1.0.0] — 2026-07-11

Release Readiness milestone: final pre-tag audit fixes across all three
frontends, closing out issues #26–#29 and #37–#41.

### Added

- `CHANGELOG.md` (this file) and `CONTRIBUTING.md`.
- Linux per-field validation-error UI: spinboxes show a red outline and a
  tooltip describing the failure, matching the existing macOS/Windows
  treatment (`applyFieldError()` in `frontend/linux/src/PageUtils.h`) (#41).
- Presenter-level tests for `LocationPresenter`'s OR-validity semantics and
  its semi-major/semi-minor cross-field check, and a new `LinkPresenter`
  test section (previously untested outside bridge tests).
- `Doxyfile` and published API docs; `docs/formulas.md` formula/citation
  reference (#27, #28).

### Fixed

- `AntennaPresenter::set_gain()` now rejects gain below -6.35 dBi, the true
  domain floor of `beamwidth_from_gain()`; previously a legal in-range gain
  (e.g. -10 dBi) produced a beamwidth over 360° with no error shown. All
  three frontends' gain input bounds now match (-6.35 to 60 dBi) — including
  Windows' `GainBox.Setup()` call in `PageCodeBehinds.cs`, which sets the
  runtime bound in code and was silently overriding the XAML `Minimum`
  attribute back to -10 (WinUI parses XAML `NumberBox.Minimum` through
  float32, so real bounds must be set in code; see `PageHelper.cs`) (#37).
- Windows "Reset Inputs" no longer no-ops on the active page.
- `LocationPresenter`'s OR-validity exception (vs. AND on every other
  presenter) is now documented in `AGENTS.md`.
- Linux README no longer describes a nonexistent `applyFieldError()` helper.
- Linux `JammingPage` RX-gain spin boxes now seed from `rx_gain_signal_db()`/
  `rx_gain_jammer_db()` instead of hardcoding `0.0`.
- Removed a redundant dead-code clamp in `partial_band_jamming()`'s duty-cycle
  calculation.
- Linux `AppSettings::instance()` no longer relies on function-local-static
  destruction order relative to `QApplication`; the singleton is now
  intentionally leaked instead of torn down via `atexit()`.
- Corrected the LPI advantage citation in `radar.h` to Adamy EW102 (#39) and
  the stale pre-v0.7.0 partial-band jamming description in `jamming.h` (#38).

## [v0.9.0] — 2026-07-09

Infrastructure & Hardening milestone (4 issues, #22–#25).

### Added

- CI `sanitizers` job (ASan/UBSan).
- CI `static-analysis` job (clang-tidy + cppcheck).
- CI `coverage` job (Clang/llvm-cov), gated by a new `EWCALC_BUILD_COVERAGE` CMake option, with a 75% line-coverage threshold.
- Windows MSIX packaging (`Package.appxmanifest`, `-Package` wired into CI; signing documented in `frontend/windows/ewcalc-winui/README.md`).

### Changed

- Moved the C bridge (`ewcalc_bridge`) from `frontend/macos/bridge/` to a top-level, platform-agnostic `bridge/` component. It now builds unconditionally on all platforms.

### Fixed

- `.clang-tidy` config, previously a silent no-op, now genuinely lints `libew`/`ewpresenter`.

## [v0.8.0] — 2026-07-08

UX Parity & Accessibility milestone (7 issues, #15–#21).

### Added

- Per-field validation errors on all 8 Windows pages and on macOS (previously Propagation-only on Windows, absent on macOS).
- VoiceOver labels (macOS), `AutomationProperties.Name` (Windows), and `setAccessibleName` (Linux) across every input and output control on all three platforms.
- Help-text tooltips ported from macOS to Windows (`ToolTipService`) and Linux (`setToolTip`).
- User inputs persist across sessions on all three platforms (versioned, per-platform storage, with a "Reset to Defaults" action).
- "Copy results" clipboard button on each calculator page on all three platforms.

### Changed

- macOS text fields now clamp out-of-range typed input to their declared bounds.

## [v0.7.0] — 2026-07-07

Correctness milestone (8 issues).

### Added

- `lpi_advantage` and partial-band regression tests.

### Changed

- The Digital page's SNR-from-Eb/N₀ output is replaced with the SNR required to hit a target Eb/N₀.

### Fixed

- `partial_band_jamming()` no longer caps jamming bandwidth at the signal bandwidth for J/S ≥ 0 dB; it now widens correctly per Adamy EW101 ch. 10.
- `-0.0` values normalize to `0.0` in formatted output.
- Stage NF input ranges on macOS/Linux now match the presenter's `[0, 30]` dB validation.
- `ReceiverPresenter` sensitivity uses the cascaded NF from the stage chain when one is defined.
- Missing jamming RX-gain bridge getters added.
- Windows adapters no longer truncate input to 6 significant figures via `RoundInput`.

## [v0.6.6] — 2026-06-18

Architectural audit fixes.

### Fixed

- SFDR2 coefficient corrected to 1/2.
- Presenter validators reject non-finite values.
- Receiver stage gain validation.
- Public API invalid-input sentinels.
- Full 2-ray burnthrough inversion.
- `Dbsm - Dbsm -> Db`.
- Formatter dash sentinel consolidation.
- macOS receiver stage-state fix.
- Windows obstruction-height error binding and stage reindexing.

## [v0.6.5] — 2026-06-16

### Added

- Complete `FieldValidationError` coverage across all 8 Windows C++/CLI adapters (Antenna, Digital, Jamming were 0/N; Location, Receiver, Propagation were partial).

### Fixed

- `partial_band_jamming` domain guard assert.

## [v0.6.4] — 2026-06-16

Bug fixes from follow-on review.

### Fixed

- Finalizer use-after-free in all 8 C++/CLI adapters.
- `ewp_receiver_stage()`/`ewp_receiver_set_stages()` UB and null-pointer guards.
- `copy_str` production truncation sentinel.
- `lpi_advantage_str` missing DASH.
- `chip_rate < data_rate` DSSS guard.
- `LocationPresenter` EEP axis error field, plus independent sub-section validity.
- `ReceiverPresenter` `system_nf` now derives from cascaded NF.
- NaN stage NF fix.
- `RadarAdapter` FieldError properties.
- `LinkOutput` two-ray-regime booleans.

### Removed

- Dead code in `CallbackBridge.h`.

## [v0.6.3] — 2026-06-16

Architecture review remediation.

### Added

- C bridge per-field error accessors (`EwpFieldError`).
- Bridge and formatter integration tests.
- `PresenterBase<Derived>` CRTP base, eliminating setter boilerplate across all 8 presenters.

### Fixed

- C++/CLI adapter lifetime safety (double-free, use-after-free, stale GCHandle access, constructor exception safety).
- Presenter validation gaps closed (Jamming gains, Receiver stage NF, `num_pulses`/`adc_bits` error fields).
- cppcheck findings resolved (clean run).

## [v0.6.2] — 2026-06-07

### Added

- `ewpresenter` assertion test suite; harness registered in CI.

### Fixed

- `DigitalPresenter` validity split (invalid DSSS inputs no longer suppress Eb/N₀).

## [v0.6.0] — 2026-06-04

### Added

- Antenna calculator across all platforms.
- Knife-edge diffraction and SNR↔Eb/N₀ wired up.

[Unreleased]: ../../compare/v1.0.2...HEAD
[v1.0.2]: ../../releases/tag/v1.0.2
[v1.0.1]: ../../releases/tag/v1.0.1
[v1.0.0]: ../../releases/tag/v1.0.0
[v0.9.0]: ../../releases/tag/v0.9.0
[v0.8.0]: ../../releases/tag/v0.8.0
[v0.7.0]: ../../releases/tag/v0.7.0
[v0.6.6]: ../../releases/tag/v0.6.6
[v0.6.5]: ../../releases/tag/v0.6.5
[v0.6.4]: ../../releases/tag/v0.6.4
[v0.6.3]: ../../releases/tag/v0.6.3
[v0.6.2]: ../../releases/tag/v0.6.2
[v0.6.0]: ../../releases/tag/v0.6.0
