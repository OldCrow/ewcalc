// LocationView.swift
import SwiftUI

struct LocationView: View {
    @ObservedObject var adapter: LocationAdapter

    @State private var rmsBearingError: Double
    @State private var aoaRange:        Double
    @State private var semiMajor:       Double
    @State private var semiMinor:       Double

    init(adapter: LocationAdapter) {
        self.adapter      = adapter
        _rmsBearingError  = State(initialValue: adapter.defaultRmsBearingError)
        _aoaRange         = State(initialValue: adapter.defaultAoaRange)
        _semiMajor        = State(initialValue: adapter.defaultSemiMajor)
        _semiMinor        = State(initialValue: adapter.defaultSemiMinor)
    }

    var body: some View {
        Form {
            Section("AOA (Angle of Arrival)") {
                InputRow("RMS bearing error", unit: "°", value: $rmsBearingError,
                         in: 0.01...45, step: 0.1, decimals: 2) { adapter.setRmsBearingError($0) }
                InputRow("Range", unit: "km", value: $aoaRange,
                         in: 0.1...10000, step: 1)               { adapter.setAoaRange($0) }
            }
            Section("EEP (Error Ellipse → CEP)") {
                InputRow("Semi-major 1σ", unit: "km", value: $semiMajor,
                         in: 0.001...1000, step: 0.1, decimals: 3) { adapter.setSemiMajor($0) }
                InputRow("Semi-minor 1σ", unit: "km", value: $semiMinor,
                         in: 0.001...1000, step: 0.1, decimals: 3) { adapter.setSemiMinor($0) }
            }
            Section("Results") {
                ResultRow("CEP (AOA)", cStr(adapter.output.cep_aoa_str))
                ResultRow("CEP (EEP)", cStr(adapter.output.cep_eep_str))
            }
        }
        .formStyle(.grouped)
        .navigationTitle("Location")
    }
}
