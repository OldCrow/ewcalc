// RadarJammingView.swift
import SwiftUI

struct RadarJammingView: View {
    @ObservedObject var adapter: RadarJammingAdapter

    @State private var geometry:     Int
    @State private var radarErp:     Double
    @State private var jammerErp:    Double
    @State private var targetRcs:    Double
    @State private var targetRange:  Double
    @State private var jammerRange:  Double
    @State private var mainbeamGain: Double
    @State private var sidelobeGain: Double
    @State private var jsThreshold:  Double

    init(adapter: RadarJammingAdapter) {
        self.adapter  = adapter
        _geometry     = State(initialValue: adapter.defaultGeometry)
        _radarErp     = State(initialValue: adapter.defaultRadarErp)
        _jammerErp    = State(initialValue: adapter.defaultJammerErp)
        _targetRcs    = State(initialValue: adapter.defaultTargetRcs)
        _targetRange  = State(initialValue: adapter.defaultTargetRange)
        _jammerRange  = State(initialValue: adapter.defaultJammerRange)
        _mainbeamGain = State(initialValue: adapter.defaultMainbeamGain)
        _sidelobeGain = State(initialValue: adapter.defaultSidelobeGain)
        _jsThreshold  = State(initialValue: adapter.defaultJsThreshold)
    }

    private var isStandOff: Bool { geometry == Int(EWP_RADAR_JAMMING_STAND_OFF) }

    private var resultsForCopy: [(label: String, value: String)] {
        [
            ("J/S",              cStr(adapter.output.js_ratio_str)),
            ("J/S margin",       cStr(adapter.output.js_margin_str)),
            ("Burnthrough range", cStr(adapter.output.burnthrough_range_str)),
            ("Status",           cStr(adapter.output.status_str))
        ]
    }

    var body: some View {
        Form {
            Section("Geometry") {
                Picker("Jamming geometry", selection: $geometry) {
                    Text("Self-protection").tag(Int(EWP_RADAR_JAMMING_SELF_PROTECTION))
                    Text("Stand-off").tag(Int(EWP_RADAR_JAMMING_STAND_OFF))
                }
                .pickerStyle(.segmented)
                .help("Self-protection: the jammer rides on the target. Stand-off: the jammer sits away from the target and enters the radar through its sidelobes")
                .onChange(of: geometry) { adapter.setGeometry($0) }
            }
            Section("Radar & target") {
                InputRow("Radar ERP", unit: "dBm", value: $radarErp,
                         in: -100...200,
                         error: adapter.radarErpError,
                         help: "Radar effective radiated power: peak transmit power + mainbeam antenna gain") { adapter.setRadarErp($0) }
                InputRow("Target RCS", unit: "dBsm", value: $targetRcs,
                         in: -40...60,
                         error: adapter.targetRcsError,
                         help: "Target radar cross section in dB relative to 1 m²") { adapter.setTargetRcs($0) }
                InputRow("Target range", unit: "km", value: $targetRange,
                         in: 0.01...10000, step: 1, decimals: 2,
                         error: adapter.targetRangeError,
                         help: "Radar-to-target range") { adapter.setTargetRange($0) }
            }
            Section("Jammer") {
                InputRow("Jammer ERP", unit: "dBm", value: $jammerErp,
                         in: -100...200,
                         error: adapter.jammerErpError,
                         help: "Jammer effective radiated power toward the radar — must lie within the radar's receiver bandwidth") { adapter.setJammerErp($0) }
                InputRow("J/S threshold", unit: "dB", value: $jsThreshold,
                         in: -30...30, step: 0.5,
                         error: adapter.jsThresholdError,
                         help: "J/S level at which jamming is considered effective — used for the margin and burnthrough range") { adapter.setJsThreshold($0) }
                if isStandOff {
                    InputRow("Jammer range", unit: "km", value: $jammerRange,
                             in: 0.01...10000, step: 1, decimals: 2,
                             error: adapter.jammerRangeError,
                             help: "Radar-to-jammer range (stand-off only)") { adapter.setJammerRange($0) }
                    InputRow("Mainbeam gain G_M", unit: "dB", value: $mainbeamGain,
                             in: -30...60,
                             error: adapter.mainbeamGainError,
                             help: "Radar antenna gain toward the target (mainbeam) — stand-off only") { adapter.setMainbeamGain($0) }
                    InputRow("Sidelobe gain G_S", unit: "dB", value: $sidelobeGain,
                             in: -30...60,
                             error: adapter.sidelobeGainError,
                             help: "Radar antenna gain toward the jammer (sidelobe) — stand-off only") { adapter.setSidelobeGain($0) }
                }
            }
            Section("J/S Analysis") {
                ResultRow("J/S",               cStr(adapter.output.js_ratio_str),
                          help: "Jammer-to-signal ratio at the radar receiver input (dB)")
                ResultRow("J/S margin",        cStr(adapter.output.js_margin_str),
                          help: "J/S minus the threshold — positive means the jammer meets the threshold")
                ResultRow("Burnthrough range", cStr(adapter.output.burnthrough_range_str),
                          help: "Range at which J/S falls to the threshold. Self-protection: the radar sees the target inside it. Stand-off: the radar-to-target range with the jammer held at its range")
                ResultRow("Status",            cStr(adapter.output.status_str),
                          help: "Whether the jammer meets the J/S threshold at the current target range")
            }
            DiagramSection(names: [isStandOff ? "jamming-stand-off" : "jamming-self-protection"])
        }
        .formStyle(.grouped)
        .navigationTitle("Radar Jamming")
        .toolbar {
            ToolbarItem(placement: .primaryAction) {
                CopyResultsButton(rows: resultsForCopy)
            }
        }
    }
}
