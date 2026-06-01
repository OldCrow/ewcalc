// LinkView.swift
import SwiftUI

struct LinkView: View {
    @ObservedObject var adapter: LinkAdapter

    @State private var txPower:       Double
    @State private var txGain:        Double
    @State private var rxGain:        Double
    @State private var distance:      Double
    @State private var txHeight:      Double
    @State private var rxHeight:      Double
    @State private var frequency:     Double
    @State private var rxSensitivity: Double

    init(adapter: LinkAdapter) {
        self.adapter  = adapter
        _txPower      = State(initialValue: adapter.defaultTxPower)
        _txGain       = State(initialValue: adapter.defaultTxGain)
        _rxGain       = State(initialValue: adapter.defaultRxGain)
        _distance     = State(initialValue: adapter.defaultDistance)
        _txHeight     = State(initialValue: adapter.defaultTxHeight)
        _rxHeight     = State(initialValue: adapter.defaultRxHeight)
        _frequency    = State(initialValue: adapter.defaultFrequency)
        _rxSensitivity = State(initialValue: adapter.defaultRxSensitivity)
    }

    var body: some View {
        Form {
            Section("Transmitter") {
                InputRow("Tx power", unit: "dBm", value: $txPower,
                         in: -50...200)  { adapter.setTxPower($0) }
                InputRow("Tx gain", unit: "dB", value: $txGain,
                         in: -30...60)   { adapter.setTxGain($0) }
                InputRow("Frequency", unit: "MHz", value: $frequency,
                         in: 0.1...100000, step: 1)  { adapter.setFrequency($0) }
            }
            Section("Geometry") {
                InputRow("Distance", unit: "km", value: $distance,
                         in: 0.01...10000, step: 0.1, decimals: 3) { adapter.setDistance($0) }
                InputRow("Tx height", unit: "m", value: $txHeight,
                         in: 0.1...100000, step: 0.5, decimals: 1) { adapter.setTxHeight($0) }
                InputRow("Rx height", unit: "m", value: $rxHeight,
                         in: 0.1...100000, step: 0.5, decimals: 1) { adapter.setRxHeight($0) }
            }
            Section("Receiver") {
                InputRow("Rx gain", unit: "dB", value: $rxGain,
                         in: -30...60)   { adapter.setRxGain($0) }
                InputRow("Rx sensitivity", unit: "dBm", value: $rxSensitivity,
                         in: -200...0)   { adapter.setRxSensitivity($0) }
            }
            Section("Results") {
                ResultRow("Received power",  cStr(adapter.output.received_power_str))
                ResultRow("Path loss",       cStr(adapter.output.path_loss_str))
                ResultRow("Link margin",     cStr(adapter.output.link_margin_str))
                ResultRow("Fresnel crossover",cStr(adapter.output.fresnel_zone_str))
                ResultRow("Regime",          cStr(adapter.output.regime_str))
                ResultRow("Effective range", cStr(adapter.output.effective_range_str))
                ResultRow("Range regime",    cStr(adapter.output.range_regime_str))
            }
        }
        .formStyle(.grouped)
        .navigationTitle("Link Budget")
    }
}
