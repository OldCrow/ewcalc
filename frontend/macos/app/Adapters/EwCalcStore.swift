// EwCalcStore.swift
import AppKit
import Combine
import Foundation

/// Top-level store injected as an environment object.
/// Owns all eight presenter adapters for the lifetime of the app.
///
/// Also owns input persistence (#20): every adapter's current values are
/// captured into a `SavedInputs` snapshot and written to UserDefaults a short
/// while after any edit, and restored on launch before the first page is
/// shown. See `SavedInputs.swift` for the on-disk format and per-adapter
/// capture/apply logic.
final class EwCalcStore: ObservableObject {
    let propagation = PropagationAdapter()
    let link        = LinkAdapter()
    let receiver    = ReceiverAdapter()
    let jamming     = JammingAdapter()
    let location    = LocationAdapter()
    let radar       = RadarAdapter()
    let digital     = DigitalAdapter()
    let antenna     = AntennaAdapter()

    /// Bumped by `resetToDefaults()`. Every calculator page's @State input
    /// fields are read once, in that view's `init`, from its adapter — they
    /// don't otherwise observe the adapter for input changes. ContentView
    /// keys the detail view's identity on this counter so a reset forces
    /// every open page to be torn down and reconstructed, re-reading the
    /// now-restored default values instead of going stale.
    @Published private(set) var resetGeneration = 0

    /// The presenter's compiled-in defaults, captured once in `init()` before
    /// any saved data is restored — this is the only point at which every
    /// adapter is guaranteed to hold its factory value. "Reset to Defaults"
    /// re-applies this snapshot.
    ///
    /// Implicitly-unwrapped (rather than `let`): Swift's two-phase class init
    /// requires every stored property to hold a value before `self` can be
    /// passed anywhere (including into `SavedInputs(capturing:)`). An IUO
    /// with no initializer implicitly defaults to nil, which satisfies phase
    /// 1 immediately so `self` is usable on the very first line of `init()`.
    /// It is always assigned before any other method runs.
    private var factoryDefaults: SavedInputs!
    private var cancellables = Set<AnyCancellable>()

    private static let userDefaultsKey = "com.oldcrow.ewcalc.savedInputs"

    init() {
        factoryDefaults = SavedInputs(capturing: self)
        restoreInputs()
        observeChangesForAutosave()
        // Best-effort safety net: the debounced autosave below normally wins,
        // but flush synchronously on quit in case a change lands just before
        // termination.
        NotificationCenter.default.addObserver(
            forName: NSApplication.willTerminateNotification, object: nil, queue: nil
        ) { [weak self] _ in self?.saveInputs() }
    }

    /// Persists every adapter's current values to UserDefaults.
    func saveInputs() {
        let snapshot = SavedInputs(capturing: self)
        guard let data = try? JSONEncoder().encode(snapshot) else { return }
        UserDefaults.standard.set(data, forKey: Self.userDefaultsKey)
    }

    /// Applies any previously saved inputs. Silently does nothing (leaving
    /// the presenter's compiled-in defaults in place) if there is no saved
    /// data or it was written by an incompatible format version.
    private func restoreInputs() {
        guard
            let data = UserDefaults.standard.data(forKey: Self.userDefaultsKey),
            let saved = try? JSONDecoder().decode(SavedInputs.self, from: data),
            saved.formatVersion == SavedInputs.currentFormatVersion
        else { return }
        saved.apply(to: self)
    }

    /// Restores every calculator's compiled-in presenter defaults, clears any
    /// persisted data, and bumps `resetGeneration` so open pages remount and
    /// re-read fresh values from their adapters.
    func resetToDefaults() {
        factoryDefaults.apply(to: self)
        UserDefaults.standard.removeObject(forKey: Self.userDefaultsKey)
        resetGeneration += 1
    }

    /// Debounced autosave: any adapter publishing a change (a field edit or
    /// its resulting recompute) schedules a save a short delay later, so
    /// rapid edits (e.g. dragging a Stepper) coalesce into one write.
    private func observeChangesForAutosave() {
        Publishers.MergeMany(
            propagation.objectWillChange, link.objectWillChange, receiver.objectWillChange,
            jamming.objectWillChange, location.objectWillChange, radar.objectWillChange,
            digital.objectWillChange, antenna.objectWillChange
        )
        .debounce(for: .milliseconds(400), scheduler: RunLoop.main)
        .sink { [weak self] _ in self?.saveInputs() }
        .store(in: &cancellables)
    }
}
