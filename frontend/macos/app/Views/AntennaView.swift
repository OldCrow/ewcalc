// AntennaView.swift
import SwiftUI

struct AntennaView: View {
    @ObservedObject var adapter: AntennaAdapter

    @State private var gain:        Double
    @State private var azBeamwidth: Double
    @State private var elBeamwidth: Double
    @State private var txPower:     Double
    @State private var frequency:   Double

    init(adapter: AntennaAdapter) {
        self.adapter   = adapter
        _gain          = State(initialValue: adapter.defaultGain)
        _azBeamwidth   = State(initialValue: adapter.defaultAzBeamwidth)
        _elBeamwidth   = State(initialValue: adapter.defaultElBeamwidth)
        _txPower       = State(initialValue: adapter.defaultTxPower)
        _frequency     = State(initialValue: adapter.defaultFrequency)
    }

    private var resultsForCopy: [(label: String, value: String)] {
        [
            ("ERP",                 cStr(adapter.output.erp_str)),
            ("Beamwidth from gain", cStr(adapter.output.beamwidth_from_gain_str)),
            ("Gain from beamwidth", cStr(adapter.output.gain_from_beamwidth_str)),
            ("Wavelength",          cStr(adapter.output.wavelength_str)),
        ]
    }

    var body: some View {
        Form {
            Section("Antenna Parameters") {
                // Lower bound matches AntennaPresenter::set_gain's -6.35 dBi validation
                // floor (below that, beamwidth_from_gain() exceeds its valid domain and
                // returns beamwidth > 360° — see audit #1 / libew antenna.h).
                InputRow("Gain", unit: "dBi", value: $gain,
                         in: -6.35...60,
                         error: adapter.gainError,
                         help: "Antenna gain relative to an isotropic radiator") { adapter.setGain($0) }
                InputRow("Az beamwidth", unit: "deg", value: $azBeamwidth,
                         in: 0.1...360,
                         error: adapter.azBeamwidthError,
                         help: "Azimuth 3 dB beamwidth — used with elevation beamwidth to estimate gain") { adapter.setAzBeamwidth($0) }
                InputRow("El beamwidth", unit: "deg", value: $elBeamwidth,
                         in: 0.1...360,
                         error: adapter.elBeamwidthError,
                         help: "Elevation 3 dB beamwidth") { adapter.setElBeamwidth($0) }
                InputRow("Tx power", unit: "dBm", value: $txPower,
                         in: -30...100,
                         error: adapter.txPowerError,
                         help: "Transmitter output power — used to compute ERP") { adapter.setTxPower($0) }
                InputRow("Frequency", unit: "MHz", value: $frequency,
                         in: 0.1...100000, step: 10, decimals: 1,
                         error: adapter.frequencyError,
                         help: "Carrier frequency — used to compute free-space wavelength") { adapter.setFrequency($0) }
            }
            Section("Results") {
                ResultRow("ERP",                 cStr(adapter.output.erp_str),
                          help: "Effective Radiated Power = Tx power + antenna gain")
                ResultRow("Beamwidth from gain", cStr(adapter.output.beamwidth_from_gain_str),
                          help: "Approximate 3 dB beamwidth derived from gain (Tai & Pereira approximation)")
                ResultRow("Gain from beamwidth", cStr(adapter.output.gain_from_beamwidth_str),
                          help: "Approximate gain from az × el beamwidth: 10·log₁₀(30000 / (θ_az · θ_el))")
                ResultRow("Wavelength",          cStr(adapter.output.wavelength_str),
                          help: "Free-space wavelength at the given frequency")
            }
        }
        .formStyle(.grouped)
        .navigationTitle("Antenna")
        .toolbar {
            ToolbarItem(placement: .primaryAction) {
                CopyResultsButton(rows: resultsForCopy)
            }
        }
    }
}
