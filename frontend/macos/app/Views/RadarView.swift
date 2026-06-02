// RadarView.swift
import SwiftUI

struct RadarView: View {
    @ObservedObject var adapter: RadarAdapter

    @State private var txPower:       Double
    @State private var antennaGain:   Double
    @State private var targetRcs:     Double
    @State private var frequency:     Double
    @State private var systemLosses:  Double
    @State private var noiseFigure:   Double
    @State private var bandwidth:     Double
    @State private var requiredSnr:   Double
    @State private var timeBandwidth: Double
    @State private var numPulses:     Int

    init(adapter: RadarAdapter) {
        self.adapter   = adapter
        _txPower       = State(initialValue: adapter.defaultTxPower)
        _antennaGain   = State(initialValue: adapter.defaultAntennaGain)
        _targetRcs     = State(initialValue: adapter.defaultTargetRcs)
        _frequency     = State(initialValue: adapter.defaultFrequency)
        _systemLosses  = State(initialValue: adapter.defaultSystemLosses)
        _noiseFigure   = State(initialValue: adapter.defaultNoiseFigure)
        _bandwidth     = State(initialValue: adapter.defaultBandwidth)
        _requiredSnr   = State(initialValue: adapter.defaultRequiredSnr)
        _timeBandwidth = State(initialValue: adapter.defaultTimeBandwidth)
        _numPulses     = State(initialValue: adapter.defaultNumPulses)
    }

    var body: some View {
        Form {
            Section("Radar Parameters") {
                InputRow("Tx power", unit: "dBm", value: $txPower,
                         in: -50...200)  { adapter.setTxPower($0) }
                InputRow("Antenna gain", unit: "dBi", value: $antennaGain,
                         in: -30...60)   { adapter.setAntennaGain($0) }
                InputRow("Target RCS", unit: "dBsm", value: $targetRcs,
                         in: -40...60)   { adapter.setTargetRcs($0) }
                InputRow("Frequency", unit: "MHz", value: $frequency,
                         in: 1...100000, step: 10, decimals: 0) { adapter.setFrequency($0) }
                InputRow("System losses", unit: "dB", value: $systemLosses,
                         in: 0...30, step: 0.5)  { adapter.setSystemLosses($0) }
                InputRow("Noise figure", unit: "dB", value: $noiseFigure,
                         in: 0...30, step: 0.5)  { adapter.setNoiseFigure($0) }
                InputRow("Bandwidth", unit: "MHz", value: $bandwidth,
                         in: 0.001...10000, step: 0.1, decimals: 3) { adapter.setBandwidth($0) }
                InputRow("Required SNR", unit: "dB", value: $requiredSnr,
                         in: -10...50, step: 0.5)  { adapter.setRequiredSnr($0) }
            }
            Section("Signal Processing") {
                InputRow("Time-BW product", unit: "", value: $timeBandwidth,
                         in: 1...1000000, step: 10, decimals: 0) { adapter.setTimeBandwidth($0) }
                LabeledContent("Coherent pulses") {
                    HStack(spacing: 4) {
                        TextField("", value: $numPulses, format: .number)
                            .frame(width: 100).textFieldStyle(.roundedBorder).multilineTextAlignment(.trailing)
                        Stepper("", value: $numPulses, in: 1...100000)
                            .labelsHidden()
                        Spacer().frame(width: 38)
                    }
                }
                .onChange(of: numPulses) { adapter.setNumPulses($0) }
            }
            Section("Results") {
                ResultRow("Max range",                cStr(adapter.output.max_range_str))
                ResultRow("Two-way loss",             cStr(adapter.output.two_way_loss_str))
                ResultRow("PC gain",                  cStr(adapter.output.pc_gain_str))
                ResultRow("Coherent integration gain",cStr(adapter.output.ci_gain_str))
                ResultRow("LPI advantage",            cStr(adapter.output.lpi_advantage_str))
            }
        }
        .formStyle(.grouped)
        .navigationTitle("Radar")
    }
}
