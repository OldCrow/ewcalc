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
                         in: 0.01...10000, step: 0.1, decimals: 3,
                         help: "Total path length between transmitter and receiver") { adapter.setDistance($0) }
                InputRow("Frequency", unit: "MHz", value: $frequency,
                         in: 0.1...100000, step: 1, decimals: 1,
                         help: "Carrier frequency — path loss scales as f² in free space") { adapter.setFrequency($0) }
                InputRow("Tx height", unit: "m", value: $txHeight,
                         in: 0.1...100000, step: 0.5, decimals: 1,
                         help: "Transmit antenna height above ground — determines the Fresnel zone crossover distance") { adapter.setTxHeight($0) }
                InputRow("Rx height", unit: "m", value: $rxHeight,
                         in: 0.1...100000, step: 0.5, decimals: 1,
                         help: "Receive antenna height above ground — determines the Fresnel zone crossover distance") { adapter.setRxHeight($0) }
            }
            Section("Results") {
                ResultRow("FSPL",              cStr(adapter.output.fspl_str),
                          help: "Free-space path loss: 32.44 + 20·log₁₀(km) + 20·log₁₀(MHz) — valid below the Fresnel zone crossover")
                ResultRow("2-ray loss",        cStr(adapter.output.two_ray_loss_str),
                          help: "Two-ray ground-reflection loss: 120 + 40·log₁₀(km) − 20·log₁₀(h_tx) − 20·log₁₀(h_rx) — loss scales as d⁴ beyond Fresnel crossover")
                ResultRow("Fresnel crossover", cStr(adapter.output.fresnel_zone_str),
                          help: "Range at which ground reflection begins to dominate: h_tx·h_rx·f / 24 000 km")
                ResultRow("Path loss",         cStr(adapter.output.path_loss_str),
                          help: "Applicable loss for this geometry — FSPL below the Fresnel crossover, 2-ray above it")
                ResultRow("Regime",            cStr(adapter.output.regime_str),
                          help: "Propagation model in use: LOS (free-space) or 2-ray (ground reflection)")
                ResultRow("Earth bulge (mid)", cStr(adapter.output.earth_bulge_str),
                          help: "Height the earth surface rises at the path midpoint under a standard k = 4/3 atmosphere")
                ResultRow("Radar horizon",     cStr(adapter.output.horizon_range_str),
                          help: "Maximum visibility range combining both antenna heights: 4.12 × (√h_tx + √h_rx) km")
            }
        }
        .formStyle(.grouped)
        .navigationTitle("Propagation")
    }
}
