// DopplerView.swift
import SwiftUI

struct DopplerView: View {
    @ObservedObject var adapter: DopplerAdapter

    @State private var frequency:   Double
    @State private var radialSpeed: Double
    @State private var prf:         Double
    @State private var bandwidth:   Double
    @State private var targetRange: Double
    @State private var beamwidthAz: Double
    @State private var beamwidthEl: Double

    init(adapter: DopplerAdapter) {
        self.adapter = adapter
        _frequency   = State(initialValue: adapter.defaultFrequency)
        _radialSpeed = State(initialValue: adapter.defaultRadialSpeed)
        _prf         = State(initialValue: adapter.defaultPrf)
        _bandwidth   = State(initialValue: adapter.defaultBandwidth)
        _targetRange = State(initialValue: adapter.defaultTargetRange)
        _beamwidthAz = State(initialValue: adapter.defaultBeamwidthAz)
        _beamwidthEl = State(initialValue: adapter.defaultBeamwidthEl)
    }

    private var resultsForCopy: [(label: String, value: String)] {
        [
            ("Doppler shift",             cStr(adapter.output.doppler_shift_str)),
            ("Unambiguous range",         cStr(adapter.output.unambiguous_range_str)),
            ("First blind speed",         cStr(adapter.output.blind_speed_str)),
            ("Unambiguous velocity (±)",  cStr(adapter.output.unambiguous_velocity_str)),
            ("Range resolution",          cStr(adapter.output.range_resolution_str)),
            ("Cross-range (az)",          cStr(adapter.output.cross_range_az_str)),
            ("Cross-range (el)",          cStr(adapter.output.cross_range_el_str))
        ]
    }

    var body: some View {
        Form {
            Section("Doppler & ambiguity") {
                InputRow("Carrier frequency", unit: "MHz", value: $frequency,
                         in: 1...100000, step: 100, decimals: 1,
                         error: adapter.frequencyError,
                         help: "Radar carrier frequency") { adapter.setFrequency($0) }
                InputRow("Radial speed (closing +)", unit: "m/s", value: $radialSpeed,
                         in: -3000...3000, step: 10, decimals: 1,
                         error: adapter.radialSpeedError,
                         help: "Target radial speed — positive closing, negative opening") { adapter.setRadialSpeed($0) }
                InputRow("PRF", unit: "Hz", value: $prf,
                         in: 10...1000000, step: 10, decimals: 0,
                         error: adapter.prfError,
                         help: "Pulse repetition frequency") { adapter.setPrf($0) }
                ResultRow("Doppler shift",            cStr(adapter.output.doppler_shift_str),
                          help: "Two-way Doppler shift: 2 × radial speed ÷ wavelength")
                ResultRow("Unambiguous range",        cStr(adapter.output.unambiguous_range_str),
                          help: "Maximum unambiguous range: c ÷ (2 × PRF)")
                ResultRow("First blind speed",        cStr(adapter.output.blind_speed_str),
                          help: "Radial speed whose Doppler shift equals the PRF — invisible to an MTI canceller")
                ResultRow("Unambiguous velocity (±)", cStr(adapter.output.unambiguous_velocity_str),
                          help: "Radial-velocity window measurable without aliasing: half the blind speed each way")
            }
            Section("Resolution") {
                InputRow("Waveform bandwidth", unit: "MHz", value: $bandwidth,
                         in: 0.001...10000, step: 0.1, decimals: 3,
                         error: adapter.bandwidthError,
                         help: "Compressed waveform bandwidth — sets the range resolution") { adapter.setBandwidth($0) }
                InputRow("Target range", unit: "km", value: $targetRange,
                         in: 0.1...5000, step: 10, decimals: 1,
                         error: adapter.targetRangeError,
                         help: "Range to the target — scales the cross-range cell size") { adapter.setTargetRange($0) }
                InputRow("Azimuth beamwidth", unit: "deg", value: $beamwidthAz,
                         in: 0.1...45, step: 0.5, decimals: 1,
                         error: adapter.beamwidthAzError,
                         help: "Antenna 3 dB azimuth beamwidth") { adapter.setBeamwidthAz($0) }
                InputRow("Elevation beamwidth", unit: "deg", value: $beamwidthEl,
                         in: 0.1...45, step: 0.5, decimals: 1,
                         error: adapter.beamwidthElError,
                         help: "Antenna 3 dB elevation beamwidth") { adapter.setBeamwidthEl($0) }
                ResultRow("Range resolution",  cStr(adapter.output.range_resolution_str),
                          help: "Range resolution: c ÷ (2 × bandwidth)")
                ResultRow("Cross-range (az)",  cStr(adapter.output.cross_range_az_str),
                          help: "Azimuth cross-range cell: range × azimuth beamwidth")
                ResultRow("Cross-range (el)",  cStr(adapter.output.cross_range_el_str),
                          help: "Elevation cross-range cell: range × elevation beamwidth")
            }
        }
        .formStyle(.grouped)
        .navigationTitle("Doppler & Resolution")
        .toolbar {
            ToolbarItem(placement: .primaryAction) {
                CopyResultsButton(rows: resultsForCopy)
            }
        }
    }
}
