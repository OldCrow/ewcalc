// ContentView.swift
import SwiftUI

enum AppSection: String, CaseIterable, Identifiable {
    case propagation = "Propagation"
    case link        = "Link Budget"
    case receiver    = "Receiver"
    case jamming     = "Jamming"
    case location    = "Location"
    case radar       = "Radar"
    case detection   = "Detection"
    case doppler     = "Doppler & Resolution"
    case digital     = "Digital / DSSS"
    case antenna     = "Antenna"
    case reference   = "Reference"

    var id: String { rawValue }

    var icon: String {
        switch self {
        case .propagation: return "antenna.radiowaves.left.and.right"
        case .link:        return "link"
        case .receiver:    return "dot.radiowaves.left.and.right"
        case .jamming:     return "wifi.exclamationmark"
        case .location:    return "location.circle"
        case .radar:       return "scope"
        case .detection:   return "target"
        case .doppler:     return "waveform"
        case .digital:     return "waveform.badge.plus"
        case .antenna:     return "antenna.radiowaves.left.and.right.circle"
        case .reference:   return "book"
        }
    }

    /// Ordered calculator pages — excludes the reference panel.
    static let calculators: [AppSection] = [
        .propagation, .antenna, .link, .receiver, .jamming, .location, .radar, .detection, .doppler, .digital
    ]
}

struct ContentView: View {
    @EnvironmentObject var store: EwCalcStore
    @State private var selection: AppSection? = .propagation
    @State private var showResetConfirmation = false

    var body: some View {
        NavigationSplitView {
            List(selection: $selection) {
                Section("Calculators") {
                    ForEach(AppSection.calculators) { section in
                        Label(section.rawValue, systemImage: section.icon)
                            .tag(section)
                    }
                }
                Section("Reference") {
                    Label(AppSection.reference.rawValue, systemImage: AppSection.reference.icon)
                        .tag(AppSection.reference)
                }
            }
            .listStyle(.sidebar)
            .navigationSplitViewColumnWidth(min: 155, ideal: 170)
        } detail: {
            // .id() forces every page to be torn down and rebuilt after a
            // reset (#20), so each view's @State input fields — read once,
            // in init, from the adapter — pick up the restored defaults.
            detailView
                .id(store.resetGeneration)
                .toolbar {
                    ToolbarItem(placement: .automatic) {
                        Button("Reset to Defaults") { showResetConfirmation = true }
                            .help("Restore every calculator's inputs to their built-in defaults")
                            .accessibilityLabel("Reset all calculators to default values")
                    }
                }
                .confirmationDialog(
                    "Reset all calculators to their default values?",
                    isPresented: $showResetConfirmation,
                    titleVisibility: .visible
                ) {
                    Button("Reset to Defaults", role: .destructive) { store.resetToDefaults() }
                    Button("Cancel", role: .cancel) {}
                } message: {
                    Text("This clears any saved inputs and cannot be undone.")
                }
        }
    }

    @ViewBuilder
    private var detailView: some View {
        switch selection {
        case .propagation: PropagationView(adapter: store.propagation)
        case .link:        LinkView(adapter: store.link)
        case .receiver:    ReceiverView(adapter: store.receiver)
        case .jamming:     JammingView(adapter: store.jamming)
        case .location:    LocationView(adapter: store.location)
        case .radar:       RadarView(adapter: store.radar)
        case .detection:   DetectionView(adapter: store.detection)
        case .doppler:     DopplerView(adapter: store.doppler)
        case .digital:     DigitalView(adapter: store.digital)
        case .antenna:     AntennaView(adapter: store.antenna)
        case .reference:   ReferenceView()
        case .none:
            Text("Select a calculator")
                .foregroundStyle(.secondary)
        }
    }
}
