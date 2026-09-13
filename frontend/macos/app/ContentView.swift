// ContentView.swift
import SwiftUI

enum AppSection: String, CaseIterable, Identifiable {
    case propagation = "Propagation"
    case link        = "Link Budget"
    case receiver    = "Receiver"
    case jamming     = "Comms Jamming"
    case location    = "Location"
    case radar       = "Radar"
    case detection   = "Detection"
    case doppler     = "Doppler & Resolution"
    case digital     = "Digital / DSSS"
    case antenna     = "Antenna"

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
        }
    }

    /// Ordered calculator pages — excludes the reference panel.
    static let calculators: [AppSection] = [
        .propagation, .antenna, .link, .receiver, .jamming, .location, .radar, .detection, .doppler, .digital
    ]
}

/// One reference page as listed in the sidebar. Pages are enumerated from
/// the data layer (#73): the sidebar renders whatever ewpresenter::refdata
/// publishes, so adding a page there adds a nav entry with no view change.
struct ReferencePageItem: Identifiable, Hashable {
    let index: Int
    let id: String
    let title: String

    /// Sidebar icon per stable page id. A reference page that mirrors a
    /// calculator's domain reuses that calculator's icon so the two read
    /// as the same concept; pages with no calculator counterpart get
    /// their own, and new pages fall back to a book.
    var icon: String {
        switch id {
        case "quick-values":    return "book"
        case "ref-propagation": return AppSection.propagation.icon
        case "ref-antennas":    return AppSection.antenna.icon
        case "ref-link":        return AppSection.link.icon
        case "ref-receiver":    return AppSection.receiver.icon
        case "ref-jamming":     return AppSection.jamming.icon
        case "ref-location":    return AppSection.location.icon
        case "ref-radar-det":   return AppSection.radar.icon
        case "ref-doppler":     return AppSection.doppler.icon
        case "ref-digital":     return AppSection.digital.icon
        case "ref-rcs":         return "airplane"
        case "ref-bands":       return "textformat.abc"
        case "ref-glossary":    return "character.book.closed"
        case "ref-db-units":    return "plusminus.circle"
        default:                return "book.closed"
        }
    }

    static let all: [ReferencePageItem] = (0..<ewp_ref_page_count()).compactMap { i in
        guard let id = ewp_ref_page_id(i).map({ String(cString: $0) }),
              let title = ewp_ref_page_title(i).map({ String(cString: $0) })
        else { return nil }
        return ReferencePageItem(index: i, id: id, title: title)
    }
}

/// Sidebar selection: a calculator or one reference page.
enum SidebarItem: Hashable {
    case calculator(AppSection)
    case reference(ReferencePageItem)
}

struct ContentView: View {
    @EnvironmentObject var store: EwCalcStore
    @State private var selection: SidebarItem? = .calculator(.propagation)
    @State private var showResetConfirmation = false

    var body: some View {
        NavigationSplitView {
            List(selection: $selection) {
                Section("Calculators") {
                    ForEach(AppSection.calculators) { section in
                        Label(section.rawValue, systemImage: section.icon)
                            .tag(SidebarItem.calculator(section))
                    }
                }
                Section("Reference") {
                    ForEach(ReferencePageItem.all) { page in
                        Label(page.title, systemImage: page.icon)
                            .tag(SidebarItem.reference(page))
                    }
                }
            }
            .listStyle(.sidebar)
            .navigationSplitViewColumnWidth(min: 200, ideal: 215)
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
        case .calculator(let section):
            switch section {
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
            }
        case .reference(let page):
            ReferenceView(pageIndex: page.index)
        case .none:
            Text("Select a calculator")
                .foregroundStyle(.secondary)
        }
    }
}
