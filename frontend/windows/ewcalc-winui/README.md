# ewcalc-winui — Phase 3.1 Windows Frontend

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
│   └── DigitalAdapter.h/.cpp
│
└── ewcalc-winui/          C# WinUI 3 (.csproj)
    ├── App.xaml            Loads XamlControlsResources — the key Phase 3.1 unlock
    ├── MainWindow.xaml     NavigationView shell
    ├── Helpers/
    │   └── FieldErrorConverter.cs   FieldValidationError → BorderBrush / tooltip
    ├── ViewModels/
│   ├── PropagationViewModel.cs
│   ├── AntennaViewModel.cs
│   ├── LinkViewModel.cs
│   ├── ReceiverViewModel.cs     (+ StageItemViewModel, RelayCommand)
│   ├── JammingLocationRadarViewModels.cs
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
        └── DigitalPage.xaml
```

## Build order

**Step 1 — Build the native libraries with CMake** (from the repo root):
```
cmake -B build -DEWCALC_BUILD_GUI=OFF
cmake --build build --config Release
```
This produces `build\lib\Release\libew.lib` and `build\lib\Release\ewpresenter.lib`.

**Step 2 — Open `ewcalc-winui.sln` in Visual Studio 2022** (17.x, with the
"Desktop development with C++" and ".NET desktop development" workloads, plus
the Windows App SDK extension).

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

## Current state (v0.6.3)

This frontend covers all nine calculator pages (Propagation, Antenna, Link Budget,
Receiver, Jamming, Location, Radar, Digital/DSSS, Reference) at v0.6 parity with macOS.

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
