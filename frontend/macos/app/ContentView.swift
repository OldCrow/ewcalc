// ContentView.swift
import SwiftUI

enum AppSection: String, CaseIterable, Identifiable {
    case propagation = "Propagation"
    case link        = "Link Budget"
    case receiver    = "Receiver"
    case jamming     = "Jamming"
    case location    = "Location"
    case radar       = "Radar"
    case digital     = "Digital / DSSS"

    var id: String { rawValue }

    var icon: String {
        switch self {
        case .propagation: return "antenna.radiowaves.left.and.right"
        case .link:        return "link"
        case .receiver:    return "dot.radiowaves.left.and.right"
        case .jamming:     return "wifi.exclamationmark"
        case .location:    return "location.circle"
        case .radar:       return "scope"
        case .digital:     return "waveform.badge.plus"
        }
    }
}

struct ContentView: View {
    @EnvironmentObject var store: EwCalcStore
    @State private var selection: AppSection? = .propagation

    var body: some View {
        NavigationSplitView {
            List(AppSection.allCases, selection: $selection) { section in
                Label(section.rawValue, systemImage: section.icon)
                    .tag(section)
            }
            .listStyle(.sidebar)
            .navigationSplitViewColumnWidth(min: 155, ideal: 170)
        } detail: {
            detailView
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
        case .digital:     DigitalView(adapter: store.digital)
        case .none:
            Text("Select a calculator")
                .foregroundStyle(.secondary)
        }
    }
}
