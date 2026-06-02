// JammingAdapter.swift
import Foundation

final class JammingAdapter: ObservableObject {
    private let ref: EwpJammingRef
    @Published private(set) var output: EwpJammingOutput

    private static let _cb: EwpJammingCallback = { out, ctx in
        guard let ctx else { return }
        Unmanaged<JammingAdapter>.fromOpaque(ctx).takeUnretainedValue().output = out
    }

    init() {
        ref    = ewp_jamming_create()
        output = ewp_jamming_output(ref)
        ewp_jamming_set_callback(ref, Self._cb, Unmanaged.passUnretained(self).toOpaque())
    }

    deinit { ewp_jamming_destroy(ref) }

    func setSignalErp(_ dbm: Double)        { ewp_jamming_set_signal_erp(ref, dbm) }
    func setJammerErp(_ dbm: Double)        { ewp_jamming_set_jammer_erp(ref, dbm) }
    func setSignalDist(_ km: Double)        { ewp_jamming_set_signal_dist(ref, km) }
    func setJammerDist(_ km: Double)        { ewp_jamming_set_jammer_dist(ref, km) }
    func setSignalHeight(_ m: Double)       { ewp_jamming_set_signal_height(ref, m) }
    func setJammerHeight(_ m: Double)       { ewp_jamming_set_jammer_height(ref, m) }
    func setRxHeight(_ m: Double)           { ewp_jamming_set_rx_height(ref, m) }
    func setFrequency(_ mhz: Double)        { ewp_jamming_set_frequency(ref, mhz) }
    func setRxGainSignal(_ db: Double)      { ewp_jamming_set_rx_gain_signal(ref, db) }
    func setRxGainJammer(_ db: Double)      { ewp_jamming_set_rx_gain_jammer(ref, db) }
    func setSignalBandwidth(_ mhz: Double)  { ewp_jamming_set_signal_bandwidth(ref, mhz) }
    func setHopRange(_ mhz: Double)         { ewp_jamming_set_hop_range(ref, mhz) }
    func setJsThreshold(_ db: Double)       { ewp_jamming_set_js_threshold(ref, db) }

    var defaultSignalErp:       Double { ewp_jamming_signal_erp(ref) }
    var defaultJammerErp:       Double { ewp_jamming_jammer_erp(ref) }
    var defaultSignalDist:      Double { ewp_jamming_signal_dist(ref) }
    var defaultJammerDist:      Double { ewp_jamming_jammer_dist(ref) }
    var defaultSignalHeight:    Double { ewp_jamming_signal_height(ref) }
    var defaultJammerHeight:    Double { ewp_jamming_jammer_height(ref) }
    var defaultRxHeight:        Double { ewp_jamming_rx_height(ref) }
    var defaultFrequency:       Double { ewp_jamming_frequency(ref) }
    var defaultSignalBandwidth: Double { ewp_jamming_signal_bandwidth(ref) }
    var defaultHopRange:        Double { ewp_jamming_hop_range(ref) }
    var defaultJsThreshold:     Double { ewp_jamming_js_threshold(ref) }
}
