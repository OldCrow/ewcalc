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
        _rxGainSignal = State(initialValue: 0.0)
        _rxGainJammer = State(initialValue: 0.0)
        _signalBw     = State(initialValue: adapter.defaultSignalBandwidth)
        _hopRange     = State(initialValue: adapter.defaultHopRange)
        _jsThreshold  = State(initialValue: adapter.defaultJsThreshold)
    }

    var body: some View {
        Form {
            Section("Signal") {
                InputRow("Signal ERP", unit: "dBm", value: $signalErp,
                         in: -100...200,
                         help: "Effective radiated power of the desired signal: transmitter power + antenna gain") { adapter.setSignalErp($0) }
                InputRow("Signal→Rx dist", unit: "km", value: $signalDist,
                         in: 0.01...10000, step: 0.1, decimals: 3) { adapter.setSignalDist($0) }
                InputRow("Signal Tx height", unit: "m", value: $signalHeight,
                         in: 0.1...100000, step: 0.5, decimals: 1) { adapter.setSignalHeight($0) }
                InputRow("Signal BW", unit: "MHz", value: $signalBw,
                         in: 0.001...1000, step: 0.001, decimals: 3,
                         help: "Occupied bandwidth of the target signal — used for partial-band jamming optimisation") { adapter.setSignalBandwidth($0) }
            }
            Section("Jammer") {
                InputRow("Jammer ERP", unit: "dBm", value: $jammerErp,
                         in: -100...200,
                         help: "Effective radiated power of the jammer toward the receiver") { adapter.setJammerErp($0) }
                InputRow("Jammer→Rx dist", unit: "km", value: $jammerDist,
                         in: 0.01...10000, step: 0.1, decimals: 3) { adapter.setJammerDist($0) }
                InputRow("Jammer height", unit: "m", value: $jammerHeight,
                         in: 0.1...100000, step: 0.5, decimals: 1) { adapter.setJammerHeight($0) }
                InputRow("Hop range", unit: "MHz", value: $hopRange,
                         in: 0...10000, step: 1,
                         help: "Total frequency-hopping bandwidth of the target signal — set to 0 for a non-hopping (single-channel) signal; partial-band results will show N/A") { adapter.setHopRange($0) }
                InputRow("J/S threshold", unit: "dB", value: $jsThreshold,
                         in: -30...30, step: 0.5,
                         help: "J/S level at which jamming is considered effective — used to calculate burnthrough range") { adapter.setJsThreshold($0) }
            }
            Section("Shared") {
                InputRow("Frequency", unit: "MHz", value: $frequency,
                         in: 0.1...100000, step: 1)  { adapter.setFrequency($0) }
                InputRow("Rx height", unit: "m", value: $rxHeight,
                         in: 0.1...100000, step: 0.5, decimals: 1) { adapter.setRxHeight($0) }
                InputRow("Rx gain → signal", unit: "dB", value: $rxGainSignal,
                         in: -30...60)  { adapter.setRxGainSignal($0) }
                InputRow("Rx gain → jammer", unit: "dB", value: $rxGainJammer,
                         in: -30...60)  { adapter.setRxGainJammer($0) }
            }
            Section("J/S Analysis") {
                ResultRow("J/S ratio",    cStr(adapter.output.js_ratio_str),
                          help: "Jammer-to-signal power ratio at the receiver input (dB)")
                ResultRow("Signal at Rx", cStr(adapter.output.signal_at_rx_str))
                ResultRow("Jammer at Rx", cStr(adapter.output.jammer_at_rx_str))
                ResultRow("Burnthrough",  cStr(adapter.output.burnthrough_range_str),
                          help: "Signal range at which J/S falls to the threshold — the jammer becomes ineffective beyond this distance")
            }
            Section("Partial-Band") {
                ResultRow("Optimum jammer BW", cStr(adapter.output.optimum_bw_str),
                          help: "Narrowing the jamming bandwidth increases instantaneous J/S at the cost of hit probability; this is the optimum trade-off")
                ResultRow("Duty cycle",        cStr(adapter.output.duty_cycle_str),
                          help: "Fraction of the hopping band covered by the optimum jamming bandwidth")
            }
        }
        .formStyle(.grouped)
        .navigationTitle("Jamming")
    }
}
