// LocationView.swift
import SwiftUI

struct LocationView: View {
    @ObservedObject var adapter: LocationAdapter

    @State private var rmsBearingError: Double
    @State private var aoaRange:        Double
    @State private var rmsTimeError:    Double
    @State private var baseline:        Double
    @State private var semiMajor:       Double
    @State private var semiMinor:       Double

    init(adapter: LocationAdapter) {
        self.adapter      = adapter
        _rmsBearingError  = State(initialValue: adapter.defaultRmsBearingError)
        _aoaRange         = State(initialValue: adapter.defaultAoaRange)
        _rmsTimeError     = State(initialValue: adapter.defaultRmsTimeError)
        _baseline         = State(initialValue: adapter.defaultBaseline)
        _semiMajor        = State(initialValue: adapter.defaultSemiMajor)
        _semiMinor        = State(initialValue: adapter.defaultSemiMinor)
    }

    var body: some View {
        Form {
            Section("AOA (Angle of Arrival)") {
                InputRow("RMS bearing error", unit: "°", value: $rmsBearingError,
                         in: 0.01...45, step: 0.1, decimals: 2,
                         help: "RMS angular error of the direction-finding receivers") { adapter.setRmsBearingError($0) }
                InputRow("Range", unit: "km", value: $aoaRange,
                         in: 0.1...10000, step: 1,
                         help: "Slant range from receivers to emitter — shared by AOA and TDOA calculations") { adapter.setAoaRange($0) }
            }
            Section("TDOA (Time Difference of Arrival)") {
                InputRow("RMS timing error", unit: "ns", value: $rmsTimeError,
                         in: 0.001...100000, step: 1, decimals: 3,
                         help: "RMS TDOA measurement error — converts to a range-difference error via speed of light") { adapter.setRmsTimeError($0) }
                InputRow("Baseline", unit: "km", value: $baseline,
                         in: 0.1...10000, step: 1,
                         help: "Receiver separation distance — wider baseline reduces CEP: CEP = c·σ_t·R / (2·B)") { adapter.setBaseline($0) }
            }
            Section("EEP (Error Ellipse → CEP)") {
                InputRow("Semi-major 1σ", unit: "km", value: $semiMajor,
                         in: 0.001...1000, step: 0.1, decimals: 3,
                         help: "Semi-major axis of the 1σ error ellipse (must be ≥ semi-minor)") { adapter.setSemiMajor($0) }
                InputRow("Semi-minor 1σ", unit: "km", value: $semiMinor,
                         in: 0.001...1000, step: 0.1, decimals: 3,
                         help: "Semi-minor axis of the 1σ error ellipse") { adapter.setSemiMinor($0) }
            }
            Section("Results") {
                ResultRow("CEP (AOA)",  cStr(adapter.output.cep_aoa_str),
                          help: "50% Circular Error Probable from angle-of-arrival: 1.2 × range × tan(RMS error)")
                ResultRow("CEP (TDOA)", cStr(adapter.output.cep_tdoa_str),
                          help: "50% Circular Error Probable from TDOA: c·σ_t·R / (2·B) — improves with wider baseline or shorter range")
                ResultRow("CEP (EEP)",  cStr(adapter.output.cep_eep_str),
                          help: "CEP from an Elliptical Error Probable: 0.59 × (semi-major + semi-minor)")
            }
        }
        .formStyle(.grouped)
        .navigationTitle("Location")
    }
}
