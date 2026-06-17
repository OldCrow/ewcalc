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
                         in: -50...200) { adapter.setTxPower($0) }
                InputRow("Antenna gain", unit: "dBi", value: $antennaGain,
                         in: -30...60,
                         help: "Same antenna used for transmit and receive — appears squared in the radar range equation") { adapter.setAntennaGain($0) }
                InputRow("Target RCS", unit: "dBsm", value: $targetRcs,
                         in: -40...60,
                         help: "Radar cross-section of the target (dB relative to 1 m²) — fighter ~0 to +10 dBsm, missile −10 dBsm") { adapter.setTargetRcs($0) }
                InputRow("Frequency", unit: "MHz", value: $frequency,
                         in: 1...100000, step: 10, decimals: 0) { adapter.setFrequency($0) }
                InputRow("System losses", unit: "dB", value: $systemLosses,
                         in: 0...30, step: 0.5,
                         help: "Combined one-way losses: feed line, atmospheric absorption, beam-shape, etc.") { adapter.setSystemLosses($0) }
                InputRow("Noise figure", unit: "dB", value: $noiseFigure,
                         in: 0...30, step: 0.5) { adapter.setNoiseFigure($0) }
                InputRow("Bandwidth", unit: "MHz", value: $bandwidth,
                         in: 0.001...10000, step: 0.1, decimals: 3,
                         help: "Matched-filter noise bandwidth — sets the thermal noise floor") { adapter.setBandwidth($0) }
                InputRow("Required SNR", unit: "dB", value: $requiredSnr,
                         in: -10...50, step: 0.5,
                         help: "Minimum SNR for the desired probability of detection / false-alarm rate") { adapter.setRequiredSnr($0) }
            }
            Section("Signal Processing") {
                InputRow("Time-BW product", unit: "", value: $timeBandwidth,
                         in: 1...1000000, step: 10, decimals: 0,
                         help: "Pulse compression ratio: pulse width × bandwidth — determines PC gain and LPI advantage") { adapter.setTimeBandwidth($0) }
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
                ResultRow("Max range",                cStr(adapter.output.max_range_str),
                          help: "Maximum detection range at the required SNR with no signal processing gain applied")
                ResultRow("Two-way loss",             cStr(adapter.output.two_way_loss_str))
                ResultRow("PC gain",                  cStr(adapter.output.pulse_compression_gain_str),
                          help: "Pulse compression SNR gain: 10·log₁₀(time-bandwidth product)")
                ResultRow("Coherent integration gain",cStr(adapter.output.coherent_integration_gain_str),
                          help: "Coherent integration SNR gain: 10·log₁₀(number of pulses)")
                ResultRow("LPI advantage",            cStr(adapter.output.lpi_advantage_str),
                          help: "Detection-range advantage an LPI waveform has against an intercept receiver: PC gain ÷ 4")
                ResultRow("Target RCS",               cStr(adapter.output.target_rcs_str),
                          help: "Target radar cross-section (dB relative to 1 m²) — shown here with dBsm suffix so the result panel is self-contained")
            }
        }
        .formStyle(.grouped)
        .navigationTitle("Radar")
    }
}
