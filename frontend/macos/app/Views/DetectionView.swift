// DetectionView.swift
import SwiftUI

struct DetectionView: View {
    @ObservedObject var adapter: DetectionAdapter

    @State private var pd:           Double
    @State private var pfaExponent:  Double
    @State private var numPulses:    Int
    @State private var swerlingCase: Int
    @State private var beamwidth:    Double
    @State private var scanRate:     Double
    @State private var prf:          Double
    @State private var bandwidth:    Double

    init(adapter: DetectionAdapter) {
        self.adapter  = adapter
        _pd           = State(initialValue: adapter.defaultPd)
        _pfaExponent  = State(initialValue: adapter.defaultPfaExponent)
        _numPulses    = State(initialValue: adapter.defaultNumPulses)
        _swerlingCase = State(initialValue: adapter.defaultSwerlingCase)
        _beamwidth    = State(initialValue: adapter.defaultBeamwidth)
        _scanRate     = State(initialValue: adapter.defaultScanRate)
        _prf          = State(initialValue: adapter.defaultPrf)
        _bandwidth    = State(initialValue: adapter.defaultBandwidth)
    }

    private var resultsForCopy: [(label: String, value: String)] {
        [
            ("Required SNR (Shnidman)",         cStr(adapter.output.required_snr_str)),
            ("Required SNR (Albersheim, Sw0)",  cStr(adapter.output.required_snr_albersheim_str)),
            ("Fluctuation loss",                cStr(adapter.output.fluctuation_loss_str)),
            ("Dwell time",                      cStr(adapter.output.dwell_time_str)),
            ("Hits per scan",                   cStr(adapter.output.hits_per_scan_str)),
            ("False-alarm rate",                cStr(adapter.output.far_str))
        ]
    }

    var body: some View {
        Form {
            Section("Detection statistics") {
                InputRow("Probability of detection", unit: "", value: $pd,
                         in: 0.1...0.99, step: 0.01, decimals: 2,
                         error: adapter.pdError,
                         help: "Single-scan probability of detection — Shnidman's equation is valid for 0.1 to 0.99") { adapter.setPd($0) }
                InputRow("Pfa exponent", unit: "10^x", value: $pfaExponent,
                         in: -9...(-3), step: 1, decimals: 0,
                         error: adapter.pfaExponentError,
                         help: "False-alarm probability exponent x, Pfa = 10^x — e.g. −6 for Pfa = 10⁻⁶") { adapter.setPfaExponent($0) }
                LabeledContent("Pulses integrated") {
                    HStack(spacing: 4) {
                        TextField("", value: $numPulses, format: .number)
                            .frame(width: 100).textFieldStyle(.roundedBorder).multilineTextAlignment(.trailing)
                            .accessibilityLabel("Pulses integrated")
                            .accessibilityValue("\(numPulses)")
                        Stepper("", value: $numPulses, in: 1...100)
                            .labelsHidden()
                            .accessibilityLabel("Pulses integrated stepper")
                        Spacer().frame(width: 38)
                    }
                }
                .onChange(of: numPulses) { adapter.setNumPulses($0) }
                LabeledContent("Swerling case (0–4)") {
                    HStack(spacing: 4) {
                        TextField("", value: $swerlingCase, format: .number)
                            .frame(width: 100).textFieldStyle(.roundedBorder).multilineTextAlignment(.trailing)
                            .accessibilityLabel("Swerling case (0–4)")
                            .accessibilityValue("\(swerlingCase)")
                        Stepper("", value: $swerlingCase, in: 0...4)
                            .labelsHidden()
                            .accessibilityLabel("Swerling case stepper")
                        Spacer().frame(width: 38)
                    }
                }
                .onChange(of: swerlingCase) { adapter.setSwerlingCase($0) }
                ResultRow("Required SNR (Shnidman)",        cStr(adapter.output.required_snr_str),
                          help: "Single-pulse SNR required for the chosen Pd/Pfa and Swerling case, after noncoherent integration (Shnidman's equation)")
                ResultRow("Required SNR (Albersheim, Sw0)", cStr(adapter.output.required_snr_albersheim_str),
                          help: "Nonfluctuating-target reference from Albersheim's equation — compare against the Shnidman value to see the fluctuation penalty")
                ResultRow("Fluctuation loss",               cStr(adapter.output.fluctuation_loss_str),
                          help: "Extra SNR the fluctuating target needs over the nonfluctuating (Swerling 0) case")
            }
            Section("Scan timing") {
                InputRow("Azimuth beamwidth", unit: "deg", value: $beamwidth,
                         in: 0.1...45, step: 0.5,
                         error: adapter.beamwidthError,
                         help: "Antenna 3 dB azimuth beamwidth") { adapter.setBeamwidth($0) }
                InputRow("Scan rate", unit: "deg/s", value: $scanRate,
                         in: 1...720, step: 1,
                         error: adapter.scanRateError,
                         help: "Antenna rotation rate — 36 deg/s is a 10 s rotation") { adapter.setScanRate($0) }
                InputRow("PRF", unit: "Hz", value: $prf,
                         in: 10...1000000, step: 10, decimals: 0,
                         error: adapter.prfError,
                         help: "Pulse repetition frequency") { adapter.setPrf($0) }
                ResultRow("Dwell time",    cStr(adapter.output.dwell_time_str),
                          help: "Time the beam illuminates a point target each scan: beamwidth ÷ scan rate")
                ResultRow("Hits per scan", cStr(adapter.output.hits_per_scan_str),
                          help: "Pulses on target per scan: dwell time × PRF")
            }
            Section("False-alarm rate") {
                InputRow("Noise bandwidth", unit: "MHz", value: $bandwidth,
                         in: 0.001...10000, step: 0.1, decimals: 3,
                         error: adapter.bandwidthError,
                         help: "Receiver noise bandwidth — with Pfa, sets the false-alarm rate") { adapter.setBandwidth($0) }
                ResultRow("False-alarm rate", cStr(adapter.output.far_str),
                          help: "Average false alarms per second: Pfa × noise bandwidth")
            }
        }
        .formStyle(.grouped)
        .navigationTitle("Detection")
        .toolbar {
            ToolbarItem(placement: .primaryAction) {
                CopyResultsButton(rows: resultsForCopy)
            }
        }
    }
}
