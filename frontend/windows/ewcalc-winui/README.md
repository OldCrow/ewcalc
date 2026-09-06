# ewcalc-winui — Windows Frontend

WinUI 3 / C# frontend for ewcalc. Consumes `libew` and `ewpresenter`
through a C++/CLI interop assembly (`ewpresenter.net`).

## Project structure

```
ewcalc-winui.sln
├── ewpresenter.net/       C++/CLI (.vcxproj) — bridges native libs to C#
│   ├── MarshalHelper.h    std::string → System::String^, FieldError → enum
│   ├── PropagationAdapter.h/.cpp
│   ├── AntennaAdapter.h/.cpp
│   ├── LinkAdapter.h/.cpp
│   ├── ReceiverAdapter.h/.cpp   (includes stage chain management)
│   ├── JammingAdapter.h/.cpp
│   ├── LocationAdapter.h/.cpp
│   ├── RadarAdapter.h/.cpp
│   ├── DetectionAdapter.h/.cpp
│   ├── DopplerAdapter.h/.cpp
│   └── DigitalAdapter.h/.cpp
│
└── ewcalc-winui/          C# WinUI 3 (.csproj)
    ├── App.xaml            Loads XamlControlsResources (required for WinUI controls)
    ├── MainWindow.xaml     NavigationView shell
    ├── Helpers/
    │   └── FieldErrorConverter.cs   FieldValidationError → BorderBrush / tooltip
    ├── ViewModels/
│   ├── PropagationViewModel.cs
│   ├── AntennaViewModel.cs
│   ├── LinkViewModel.cs
│   ├── ReceiverViewModel.cs     (+ StageItemViewModel, RelayCommand)
│   ├── JammingLocationRadarViewModels.cs
│   ├── DetectionViewModel.cs
│   ├── DopplerViewModel.cs
│   └── DigitalViewModel.cs
    └── Views/
        ├── ResultRow.xaml/.cs       Shared label/value row control
        ├── PropagationPage.xaml/.cs
        ├── AntennaPage.xaml
        ├── LinkPage.xaml/.cs
        ├── ReceiverPage.xaml/.cs
        ├── JammingPage.xaml/.cs
        ├── LocationPage.xaml/.cs
        ├── RadarPage.xaml/.cs
        ├── DetectionPage.xaml
        ├── DopplerPage.xaml
        └── DigitalPage.xaml
```

## Build order

**Step 1 — Build the native libraries with CMake** (from the repo root):
```
cmake -B build -DEWCALC_BUILD_FRONTEND=OFF
cmake --build build --config Release
```
This produces `build\lib\Release\libew.lib` and `build\lib\Release\ewpresenter.lib`.

**Step 2 — Open `ewcalc-winui.sln` in Visual Studio 2022 or newer** (e.g.
VS 18 2026, with the "Desktop development with C++" and ".NET desktop
development" workloads, plus the Windows App SDK extension).

**Step 3 — Set configuration to `Release | x64` and build the solution.**
VS builds `ewpresenter.net` first (it links against the .lib files from Step 1),
then builds `ewcalc-winui` which references the resulting assembly.

**Step 4 — Run `ewcalc-winui`** directly from VS (F5) or from the output bin.

## Key design decisions

### Why C++/CLI rather than P/Invoke?
The ewpresenter interface uses `std::function` callbacks and `std::vector` for
the Friis stage chain. P/Invoke cannot marshal these types. C++/CLI gives us
direct C++ interop with the full type system, and the managed ref classes it
produces are first-class .NET objects — no unsafe code in the C# layer.

### Why NavigationView rather than TabView?
The calculators have very different visual weight and input count. A TabView
    would clip labels at typical window widths. NavigationView provides grouping
    (Propagation / Analysis), compact mode on narrow windows, and natural room for
    additional calculators or a Settings page without restructuring the shell.

### Binding strategy
- `x:Bind Mode=OneTime` for default values (NumberBox initial values).
- `x:Bind Mode=OneWay` for output TextBlocks — updated via INotifyPropertyChanged.
- `NumberBox.ValueChanged` → ViewModel setter → Adapter setter → native presenter.
- The presenter's `set_on_change` callback fires synchronously inside each setter;
  the ViewModel marshals it to the UI thread via `DispatcherQueue`.

### Receiver stage chain
`ReceiverViewModel.Stages` is an `ObservableCollection<StageItemViewModel>`.
Each item's `NoiseFigureDb` and `GainDb` setters call `PushStages()` which
rebuilds the full `StageInput[]` and calls `ReceiverAdapter.SetStages()`.
Add/Remove buttons are bound to `ICommand` properties.

## MSIX packaging and signing

`Package.appxmanifest` declares the app's package identity (`Identity`,
`Properties`, `Applications`, `Capabilities`) for MSIX packaging. Logo and
splash assets already exist under `Assets/` at the sizes the manifest
references (`Square44x44Logo`, `Square150x150Logo`, `Wide310x150Logo`,
`Square71x71Logo`, `Square310x310Logo`, `StoreLogo`, `SplashScreen`).

`scripts/build-windows.ps1` builds, packages, and optionally signs in one
pass:

```powershell
.\scripts\build-windows.ps1 -Package                              # build + unsigned .msix
.\scripts\build-windows.ps1 -Package -Sign -CertThumbprint <thumb> # + sign
```

The unsigned `.msix` is written to `build\msix\`. CI (`.github/workflows/ci.yml`,
`build-windows` job) runs the same `-Package` flow on tagged commits and manual
(`workflow_dispatch`) runs, and uploads the `.msix` as a workflow artifact; the
`release` job attaches it to the GitHub Release. CI packaging is unsigned —
signing is a local/distribution step.

**`Identity/Publisher` must exactly match the Subject of the certificate used
to sign the package**, or installation fails. To generate a matching
self-signed certificate for local/test packaging:

```powershell
New-SelfSignedCertificate -Type Custom -Subject "CN=Your Name or Organization" `
    -KeyUsage DigitalSignature -FriendlyName "ewcalc test signing" `
    -CertStoreLocation "Cert:\CurrentUser\My" `
    -TextExtension @("*********={text}CA=false", "*********={text}****************3")
```

Replace `CN=Your Name or Organization` with your own identity, and update
`Identity/Publisher` in `Package.appxmanifest` to the same value — the two
must match exactly. If you fork this repo to build and sign your own copy,
use your own identity in both places rather than the upstream default;
otherwise your locally signed package won't match the checked-in manifest.

A self-signed certificate is sufficient for local sideloading and internal
testing (the target machine must trust the cert, e.g. via `Add-AppDevPackage.ps1`
or importing it into the Trusted People store). **Public distribution requires
a trusted certificate** from a CA (standard code-signing or EV), matching how
the macOS build requires a Developer ID Application certificate for
notarization (see `frontend/macos/CMakeLists.txt` and the `build-macos` job in
`.github/workflows/ci.yml`). There is no Store-specific step here — Microsoft
Store submission re-signs the package with its own certificate at ingestion.

## Current state (v1.1.0)

This frontend covers all eleven pages (Propagation, Antenna, Link Budget,
Receiver, Jamming, Location, Radar, Detection, Doppler & Resolution,
Digital/DSSS, Reference) at full parity with macOS and Linux.

## Completed in v1.1.0

- **Detection page** — new `DetectionAdapter` + `DetectionPage.xaml` backed by
  `DetectionPresenter` (required SNR via Albersheim/Shnidman with Swerling
  0–4, fluctuation loss, dwell time, hits per scan, false-alarm rate).
- **Doppler & Resolution page** — new `DopplerAdapter` + `DopplerPage.xaml`
  backed by `DopplerPresenter` (Doppler shift, unambiguous range/velocity,
  blind speed, range and cross-range resolution).
- **SQNR row** on the Receiver page, from the core's DR/SQNR split.
- **Geometry sections** (#72) — collapsed Expanders on six pages showing the
  shared diagram PNGs, MSBuild-linked from `assets/diagrams/png/` rather
  than duplicated.
- **Help tooltips** on every input and result row across all pages, matching
  the macOS/Linux help strings; the four error-bound tooltips fall back to
  help text via `ConverterParameter` instead of showing an empty square.
- The Propagation regime InfoBar became a standard "Regime" result row, the
  Location "About CEP" InfoBar became per-row CEP tooltips, and the nav pane
  widened so "Doppler & Resolution" fits.
- #62 (field colour-coding) closed as already implemented since v0.8.0 and
  verified at runtime; the app must run packaged — see AGENTS.md's Windows
  Frontend notes for the loose-layout dev-run recipe.

## Completed in v1.0.0

Release-readiness fixes (#37): `AntennaPage.xaml`'s `GainBox` `Minimum` was tightened
to -6.35 dBi to match `AntennaPresenter::set_gain()`'s validation floor, but
`AntennaPage()`'s constructor in `PageCodeBehinds.cs` called `GainBox.Setup(-10.0, 60.0)`
after `InitializeComponent()`, silently reverting the runtime bound back to -10 — WinUI
parses XAML `NumberBox.Minimum`/`Maximum` through `float32`, so the real bound has
always had to be set in code via `PageHelper.Setup()` (the XAML value alone is
cosmetic). Fixed to `GainBox.Setup(-6.35, 60.0)`; audited every other page's
`Setup()` call against its XAML bounds and found no other mismatches. Also fixed
`ResetInputsButton_Click` (`MainWindow.xaml.cs`) no-op'ing on the active page, and
removed a stale comment and dead `RoundInput()` helper.

## Completed in v0.9.0

Added `Package.appxmanifest` (Identity/Properties/Applications/Capabilities)
for MSIX packaging, wired `-Package` into the `build-windows` CI job on tagged
and manual runs with `.msix` artifact upload, and documented signing (#24).
The v0.5 changelog entry below has been corrected: the manifest did not exist
until this release.

## Completed in v0.8.0

All 8 calculator pages (Propagation, Antenna, Link, Receiver, Jamming, Location, Radar,
Digital) now: surface per-field `FieldValidationError` state via `BorderBrush`/tooltip
(previously Propagation-only); set `AutomationProperties.Name` on every input and output
control, including per-row names for the Receiver's dynamic Friis stage list; persist
inputs to `%LOCALAPPDATA%\ewcalc\settings.json` (`Helpers/SettingsService.cs`, debounced
save-on-change plus a flush on window close), with a "Reset saved inputs" action in the
nav pane footer; and add a "Copy results" clipboard button (`Helpers/ClipboardHelper.cs`)
that copies all output fields as `Label: Value` lines.

## Completed in v0.7.0

Removed the `RoundInput()` 6-sig-fig truncation wrapper from every adapter setter
(`ReceiverAdapter.h`, `RadarAdapter.h`, `LinkAdapter.h`, and all others), eliminating
cross-platform input divergence from macOS/Linux (#13). Renamed the Digital page's
SNR-from-Eb/N₀ output to the SNR required for a target Eb/N₀ in `DigitalViewModel.cs`,
`DigitalAdapter.h`, and `DigitalPage.xaml` (#10).

## Completed in v0.6.6

Propagation now binds `ObstructionHeightError` to the obstruction-height `NumberBox`.
Receiver stage labels now reindex after removal. `FieldValidationError.NotFinite`
was added for non-finite presenter inputs.

## Completed in v0.6.5

All 8 C++/CLI adapter headers now expose a complete set of `FieldValidationError` properties. `AntennaAdapter` (5 fields), `DigitalAdapter` (6 fields), and `JammingAdapter` (13 fields) had zero; `LocationAdapter`, `ReceiverAdapter`, and `PropagationAdapter` were missing recently-added fields. All adapters now match `LinkAdapter` and `RadarAdapter` for per-field error exposure.

## Completed in v0.6.4

Finalizer use-after-free fixed in all 8 adapters (`set_on_change(nullptr)` added to each `!AdapterName()` finalizer). `RadarAdapter` gains 10 `FieldValidationError` properties matching `LinkAdapter`. `LinkOutput` gains `TwoRayRegime` and `RangeTwoRayRegime` booleans for conditional UI formatting. `CallbackBridge.h` deleted (dead code).

## Completed in v0.6.3

All 8 C++/CLI adapter lifetime bugs fixed: GCHandle double-free (`GC::SuppressFinalize` in destructors), use-after-free (`set_on_change(nullptr)` before `delete presenter_`), stale handle access (`IsAllocated` guard in dispatch functions), and constructor exception safety. Presenter validation gaps closed (Jamming receive gain bounds, Receiver stage NF, `num_pulses`/`adc_bits` moved to setter error fields).

## Completed in v0.6.2

DigitalPresenter DSSS validity split — invalid chip rate, required Eb/N₀, or implementation loss no longer suppress the Eb/N₀ output (DSSS fields show N/A instead).

## Completed in v0.6

- **Antenna page** — new `AntennaAdapter` + `AntennaPage.xaml` backed by `AntennaPresenter`
  (ERP, beamwidth from gain, gain from beamwidth, wavelength).
- **Knife-edge diffraction** — new obstruction height input and diffraction loss output on
  the Propagation page.
- **SNR from Eb/N₀** — additional output on the Digital/DSSS page.

## Completed in v0.5 (from planned)

- **Digital / DSSS page** — new `DigitalAdapter` + `DigitalPage.xaml` backed
  by `DigitalPresenter` (Eb/N₀, DSSS process gain, jamming margin, required J/S).
- **Reference panel** — static read-only page with copy-to-clipboard, matching
  the macOS Reference panel content (antenna gain, SLL, noise floor, RCS, Eb/N₀).
- **Extended outputs on existing pages** — earth bulge + radar horizon
  (Propagation); system noise temp (Receiver); J/S threshold + burnthrough
  (Jamming); TDOA CEP (Location); LPI advantage (Radar).
- **hop\_range = 0** — allow zero in the Jamming page NumberBox; partial-band
  results show N/A (presenter already handles this correctly).
- MSIX packaging identity (`Package.appxmanifest`) for Store submission.
