// JammingView.swift
import SwiftUI

struct JammingView: View {
    @ObservedObject var adapter: JammingAdapter

    @State private var signalErp:      Double
    @State private var jammerErp:      Double
    @State private var signalDist:     Double
    @State private var jammerDist:     Double
    @State private var signalHeight:   Double
    @State private var jammerHeight:   Double
    @State private var rxHeight:       Double
    @State private var frequency:      Double
    @State private var rxGainSignal:   Double
    @State private var rxGainJammer:   Double
    @State private var signalBw:       Double
    @State private var hopRange:       Double
    @State private var jsThreshold:    Double

    init(adapter: JammingAdapter) {
        self.adapter  = adapter
        _signalErp    = State(initialValue: adapter.defaultSignalErp)
        _jammerErp    = State(initialValue: adapter.defaultJammerErp)
        _signalDist   = State(initialValue: adapter.defaultSignalDist)
        _jammerDist   = State(initialValue: adapter.defaultJammerDist)
        _signalHeight = State(initialValue: adapter.defaultSignalHeight)
        _jammerHeight = State(initialValue: adapter.defaultJammerHeight)
        _rxHeight     = State(initialValue: adapter.defaultRxHeight)
        _frequency    = State(initialValue: adapter.defaultFrequency)
        _rxGainSignal = State(initialValue: adapter.defaultRxGainSignal)
        _rxGainJammer = State(initialValue: adapter.defaultRxGainJammer)
        _signalBw     = State(initialValue: adapter.defaultSignalBandwidth)
        _hopRange     = State(initialValue: adapter.defaultHopRange)
        _jsThreshold  = State(initialValue: adapter.defaultJsThreshold)
    }

    private var resultsForCopy: [(label: String, value: String)] {
        [
            ("J/S ratio",           cStr(adapter.output.js_ratio_str)),
            ("Signal at Rx",        cStr(adapter.output.signal_at_rx_str)),
            ("Jammer at Rx",        cStr(adapter.output.jammer_at_rx_str)),
            ("Burnthrough",         cStr(adapter.output.burnthrough_range_str)),
            ("Optimum jammer BW",   cStr(adapter.output.optimum_bw_str)),
            ("Duty cycle",          cStr(adapter.output.duty_cycle_str))
        ]
    }

    var body: some View {
        Form {
            Section("Signal") {
                InputRow("Signal ERP", unit: "dBm", value: $signalErp,
                         in: -100...200,
                         error: adapter.signalErpError,
                         help: "Effective radiated power of the desired signal: transmitter power + antenna gain") { adapter.setSignalErp($0) }
                InputRow("Signal→Rx dist", unit: "km", value: $signalDist,
                         in: 0.01...10000, step: 0.1, decimals: 3,
                         error: adapter.signalDistError,
                         help: "Ground range from the signal transmitter to the receiver") { adapter.setSignalDist($0) }
                InputRow("Signal Tx height", unit: "m", value: $signalHeight,
                         in: 0.1...100000, step: 0.5, decimals: 1,
                         error: adapter.signalHeightError,
                         help: "Signal transmitter antenna height above ground — determines the propagation model (LOS or 2-ray) and Fresnel zone crossover distance") { adapter.setSignalHeight($0) }
                InputRow("Signal BW", unit: "MHz", value: $signalBw,
                         in: 0.001...1000, step: 0.001, decimals: 3,
                         error: adapter.signalBandwidthError,
                         help: "Occupied bandwidth of the target signal — used for partial-band jamming optimisation") { adapter.setSignalBandwidth($0) }
            }
            Section("Jammer") {
                InputRow("Jammer ERP", unit: "dBm", value: $jammerErp,
                         in: -100...200,
                         error: adapter.jammerErpError,
                         help: "Effective radiated power of the jammer toward the receiver") { adapter.setJammerErp($0) }
                InputRow("Jammer→Rx dist", unit: "km", value: $jammerDist,
                         in: 0.01...10000, step: 0.1, decimals: 3,
                         error: adapter.jammerDistError,
                         help: "Ground range from the jammer to the receiver") { adapter.setJammerDist($0) }
                InputRow("Jammer height", unit: "m", value: $jammerHeight,
                         in: 0.1...100000, step: 0.5, decimals: 1,
                         error: adapter.jammerHeightError,
                         help: "Jammer antenna height above ground — determines the propagation model (LOS or 2-ray) and Fresnel zone crossover distance") { adapter.setJammerHeight($0) }
                InputRow("Hop range", unit: "MHz", value: $hopRange,
                         in: 0...10000, step: 1,
                         error: adapter.hopRangeError,
                         help: "Total frequency-hopping bandwidth of the target signal — set to 0 for a non-hopping (single-channel) signal; partial-band results will show N/A") { adapter.setHopRange($0) }
                InputRow("J/S threshold", unit: "dB", value: $jsThreshold,
                         in: -30...30, step: 0.5,
                         error: adapter.jsThresholdError,
                         help: "J/S level at which jamming is considered effective — used to calculate burnthrough range") { adapter.setJsThreshold($0) }
            }
            Section("Shared") {
                InputRow("Frequency", unit: "MHz", value: $frequency,
                         in: 0.1...100000, step: 1,
                         error: adapter.frequencyError,
                         help: "Carrier frequency — used for both signal and jammer path-loss calculations") { adapter.setFrequency($0) }
                InputRow("Rx height", unit: "m", value: $rxHeight,
                         in: 0.1...100000, step: 0.5, decimals: 1,
                         error: adapter.rxHeightError,
                         help: "Receiver antenna height above ground — determines the propagation model (LOS or 2-ray) and Fresnel zone crossover distance") { adapter.setRxHeight($0) }
                InputRow("Rx gain → signal", unit: "dB", value: $rxGainSignal,
                         in: -30...60,
                         error: adapter.rxGainSignalError,
                         help: "Receive antenna gain toward the signal transmitter — use the main lobe gain if the receiver antenna is pointed at the signal") { adapter.setRxGainSignal($0) }
                InputRow("Rx gain → jammer", unit: "dB", value: $rxGainJammer,
                         in: -30...60,
                         error: adapter.rxGainJammerError,
                         help: "Receive antenna gain toward the jammer — a directional antenna with low sidelobes can reject an off-axis jammer by 20–30 dB; sidelobes are typically −13 to −20 dBc") { adapter.setRxGainJammer($0) }
            }
            Section("J/S Analysis") {
                ResultRow("J/S ratio",    cStr(adapter.output.js_ratio_str),
                          help: "Jammer-to-signal power ratio at the receiver input (dB)")
                ResultRow("Signal at Rx", cStr(adapter.output.signal_at_rx_str),
                          help: "Received signal power at the receiver input")
                ResultRow("Jammer at Rx", cStr(adapter.output.jammer_at_rx_str),
                          help: "Received jammer power at the receiver input")
                ResultRow("Burnthrough",  cStr(adapter.output.burnthrough_range_str),
                          help: "Signal range at which J/S falls to the threshold — the jammer becomes ineffective beyond this distance")
            }
            Section("Partial-Band") {
                ResultRow("Optimum jammer BW", cStr(adapter.output.optimum_bw_str),
                          help: "Narrowing the jamming bandwidth increases instantaneous J/S at the cost of hit probability; this is the optimum trade-off")
                ResultRow("Duty cycle",        cStr(adapter.output.duty_cycle_str),
                          help: "Fraction of the hopping band covered by the optimum jamming bandwidth")
            }
            DiagramSection(names: ["jamming-self-protection", "jamming-stand-off"])
        }
        .formStyle(.grouped)
        .navigationTitle("Jamming")
        .toolbar {
            ToolbarItem(placement: .primaryAction) {
                CopyResultsButton(rows: resultsForCopy)
            }
        }
    }
}
