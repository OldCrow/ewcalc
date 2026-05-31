// PropagationView.swift
import SwiftUI

struct PropagationView: View {
    @ObservedObject var adapter: PropagationAdapter

    @State private var distance:  Double
    @State private var frequency: Double
    @State private var txHeight:  Double
    @State private var rxHeight:  Double

    init(adapter: PropagationAdapter) {
        self.adapter = adapter
        _distance  = State(initialValue: adapter.defaultDistance)
        _frequency = State(initialValue: adapter.defaultFrequency)
        _txHeight  = State(initialValue: adapter.defaultTxHeight)
        _rxHeight  = State(initialValue: adapter.defaultRxHeight)
    }

    var body: some View {
        Form {
            Section("Inputs") {
                InputRow("Distance", unit: "km", value: $distance,
                         in: 0.01...10000, step: 0.1, decimals: 3) { adapter.setDistance($0) }
                InputRow("Frequency", unit: "MHz", value: $frequency,
                         in: 0.1...100000, step: 1, decimals: 1) { adapter.setFrequency($0) }
                InputRow("Tx height", unit: "m", value: $txHeight,
                         in: 0.1...100000, step: 0.5, decimals: 1) { adapter.setTxHeight($0) }
                InputRow("Rx height", unit: "m", value: $rxHeight,
                         in: 0.1...100000, step: 0.5, decimals: 1) { adapter.setRxHeight($0) }
            }
            Section("Results") {
                ResultRow("FSPL",              cStr(adapter.output.fspl_str))
                ResultRow("2-ray loss",        cStr(adapter.output.two_ray_loss_str))
                ResultRow("Fresnel crossover", cStr(adapter.output.fresnel_zone_str))
                ResultRow("Path loss",         cStr(adapter.output.path_loss_str))
                ResultRow("Regime",            cStr(adapter.output.regime_str))
            }
        }
        .formStyle(.grouped)
        .navigationTitle("Propagation")
    }
}
