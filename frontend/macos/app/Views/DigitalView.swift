// DigitalView.swift
import SwiftUI

struct DigitalView: View {
    @ObservedObject var adapter: DigitalAdapter

    @State private var snr:                Double
    @State private var bandwidth:          Double
    @State private var dataRate:           Double
    @State private var chipRate:           Double
    @State private var requiredEbNo:       Double
    @State private var implementationLoss: Double

    init(adapter: DigitalAdapter) {
        self.adapter       = adapter
        _snr               = State(initialValue: adapter.defaultSnr)
        _bandwidth         = State(initialValue: adapter.defaultBandwidth)
        _dataRate          = State(initialValue: adapter.defaultDataRate)
        _chipRate          = State(initialValue: adapter.defaultChipRate)
        _requiredEbNo      = State(initialValue: adapter.defaultRequiredEbNo)
        _implementationLoss = State(initialValue: adapter.defaultImplementationLoss)
    }

    var body: some View {
        Form {
            Section("Digital Link") {
                InputRow("Data rate", unit: "Mbps", value: $dataRate,
                         in: 0.0001...10000, step: 0.01, decimals: 4) { adapter.setDataRate($0) }
                InputRow("Bandwidth", unit: "MHz", value: $bandwidth,
                         in: 0.001...10000, step: 0.1, decimals: 3) { adapter.setBandwidth($0) }
                InputRow("Received SNR", unit: "dB", value: $snr,
                         in: -30...60)  { adapter.setSnr($0) }
            }
            Section("DSSS") {
                InputRow("Chip rate", unit: "Mcps", value: $chipRate,
                         in: 0.0001...10000, step: 1, decimals: 3) { adapter.setChipRate($0) }
                InputRow("Required Eb/N₀", unit: "dB", value: $requiredEbNo,
                         in: -10...30, step: 0.5) { adapter.setRequiredEbNo($0) }
                InputRow("Impl. loss", unit: "dB", value: $implementationLoss,
                         in: 0...10, step: 0.5) { adapter.setImplementationLoss($0) }
            }
            Section("Results") {
                ResultRow("Eb/N₀",           cStr(adapter.output.eb_no_str))
                ResultRow("Process gain",     cStr(adapter.output.process_gain_str))
                ResultRow("Jamming margin",   cStr(adapter.output.jamming_margin_str))
                ResultRow("Required J/S",     cStr(adapter.output.required_js_str))
            }
        }
        .formStyle(.grouped)
        .navigationTitle("Digital / DSSS")
    }
}
