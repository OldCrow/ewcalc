# Changelog

All notable changes to this project are documented in this file.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Added

- `CHANGELOG.md` (this file) and `CONTRIBUTING.md`.
- Linux per-field validation-error UI: spinboxes show a red outline and a
  tooltip describing the failure, matching the existing macOS/Windows
  treatment (`applyFieldError()` in `frontend/linux/src/PageUtils.h`).
- Presenter-level tests for `LocationPresenter`'s OR-validity semantics and
  its semi-major/semi-minor cross-field check, and a new `LinkPresenter`
  test section (previously untested outside bridge tests).

### Fixed

- `AntennaPresenter::set_gain()` now rejects gain below -6.35 dBi, the true
  domain floor of `beamwidth_from_gain()`; previously a legal in-range gain
  (e.g. -10 dBi) produced a beamwidth over 360° with no error shown. All
  three frontends' gain input bounds now match (-6.35 to 60 dBi) — including
  Windows' `GainBox.Setup()` call in `PageCodeBehinds.cs`, which sets the
  runtime bound in code and was silently overriding the XAML `Minimum`
  attribute back to -10 (WinUI parses XAML `NumberBox.Minimum` through
  float32, so real bounds must be set in code; see `PageHelper.cs`).
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

[Unreleased]: ../../compare/v0.9.0...HEAD
[v0.9.0]: ../../releases/tag/v0.9.0
[v0.8.0]: ../../releases/tag/v0.8.0
[v0.7.0]: ../../releases/tag/v0.7.0
[v0.6.6]: ../../releases/tag/v0.6.6
[v0.6.5]: ../../releases/tag/v0.6.5
[v0.6.4]: ../../releases/tag/v0.6.4
[v0.6.3]: ../../releases/tag/v0.6.3
[v0.6.2]: ../../releases/tag/v0.6.2
[v0.6.0]: ../../releases/tag/v0.6.0
