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
                         in: 0.0001...10000, step: 0.01, decimals: 4,
                         help: "Information bit rate — shared between the Eb/N₀ conversion and DSSS process gain") { adapter.setDataRate($0) }
                InputRow("Bandwidth", unit: "MHz", value: $bandwidth,
                         in: 0.001...10000, step: 0.1, decimals: 3,
                         help: "Receiver noise bandwidth — wider than the data rate gives Eb/N₀ > SNR") { adapter.setBandwidth($0) }
                InputRow("Received SNR", unit: "dB", value: $snr,
                         in: -30...60,
                         help: "Carrier-to-noise ratio measured in the noise bandwidth") { adapter.setSnr($0) }
            }
            Section("DSSS") {
                InputRow("Chip rate", unit: "Mcps", value: $chipRate,
                         in: 0.0001...10000, step: 1, decimals: 3,
                         help: "Spread-spectrum chipping rate — determines the spreading bandwidth and process gain") { adapter.setChipRate($0) }
                InputRow("Required Eb/N₀", unit: "dB", value: $requiredEbNo,
                         in: -10...30, step: 0.5,
                         help: "Minimum energy-per-bit to noise density for acceptable BER — typically 10–13 dB for BPSK/QPSK") { adapter.setRequiredEbNo($0) }
                InputRow("Impl. loss", unit: "dB", value: $implementationLoss,
                         in: 0...10, step: 0.5,
                         help: "Practical losses from non-ideal code synchronisation, filter roll-off, etc. — typically 1–3 dB") { adapter.setImplementationLoss($0) }
            }
            Section("Results") {
                ResultRow("Eb/N₀",           cStr(adapter.output.eb_no_str),
                          help: "Energy-per-bit to noise density: SNR + 10·log₁₀(bandwidth / data rate)")
                ResultRow("SNR (from Eb/No)", cStr(adapter.output.snr_from_eb_no_str),
                          help: "Carrier SNR recovered from Eb/N₀ — inverse of the Eb/N₀ conversion")
                ResultRow("Process gain",     cStr(adapter.output.process_gain_str),
                          help: "DSSS spreading gain: 10·log₁₀(chip rate / data rate)")
                ResultRow("Jamming margin",   cStr(adapter.output.jamming_margin_str),
                          help: "Process gain minus required Eb/N₀ minus implementation losses — positive means spreading gain exceeds jammer advantage")
                ResultRow("Required J/S",     cStr(adapter.output.required_js_str),
                          help: "J/S a jammer must achieve to overcome the spreading gain — negative means the jammer has an inherent advantage")
            }
        }
        .formStyle(.grouped)
        .navigationTitle("Digital / DSSS")
    }
}
