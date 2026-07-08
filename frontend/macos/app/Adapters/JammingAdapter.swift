// JammingAdapter.swift
import Foundation

final class JammingAdapter: ObservableObject {
    private let ref: EwpJammingRef
    @Published private(set) var output: EwpJammingOutput

    // Per-field validation state (#16).
    @Published private(set) var signalErpError:       EwpFieldError
    @Published private(set) var jammerErpError:       EwpFieldError
    @Published private(set) var signalDistError:      EwpFieldError
    @Published private(set) var jammerDistError:      EwpFieldError
    @Published private(set) var signalHeightError:    EwpFieldError
    @Published private(set) var jammerHeightError:    EwpFieldError
    @Published private(set) var rxHeightError:        EwpFieldError
    @Published private(set) var frequencyError:       EwpFieldError
    @Published private(set) var rxGainSignalError:    EwpFieldError
    @Published private(set) var rxGainJammerError:    EwpFieldError
    @Published private(set) var signalBandwidthError: EwpFieldError
    @Published private(set) var hopRangeError:        EwpFieldError
    @Published private(set) var jsThresholdError:     EwpFieldError

    private static let _cb: EwpJammingCallback = { out, ctx in
        guard let ctx else { return }
        Unmanaged<JammingAdapter>.fromOpaque(ctx).takeUnretainedValue().output = out
    }

    init() {
        ref    = ewp_jamming_create()
        output = ewp_jamming_output(ref)
        signalErpError       = ewp_jamming_signal_erp_error(ref)
        jammerErpError       = ewp_jamming_jammer_erp_error(ref)
        signalDistError      = ewp_jamming_signal_dist_error(ref)
        jammerDistError      = ewp_jamming_jammer_dist_error(ref)
        signalHeightError    = ewp_jamming_signal_height_error(ref)
        jammerHeightError    = ewp_jamming_jammer_height_error(ref)
        rxHeightError        = ewp_jamming_rx_height_error(ref)
        frequencyError       = ewp_jamming_frequency_error(ref)
        rxGainSignalError    = ewp_jamming_rx_gain_signal_error(ref)
        rxGainJammerError    = ewp_jamming_rx_gain_jammer_error(ref)
        signalBandwidthError = ewp_jamming_signal_bandwidth_error(ref)
        hopRangeError        = ewp_jamming_hop_range_error(ref)
        jsThresholdError     = ewp_jamming_js_threshold_error(ref)
        // passUnretained: safe only because all adapters are `let` properties of
        // EwCalcStore (@StateObject owned by ewcalcApp) and live until process exit.
        // Do not move adapters to shorter-lived objects without switching to
        // passRetained/takeRetainedValue + clearing the callback in deinit.
        ewp_jamming_set_callback(ref, Self._cb, Unmanaged.passUnretained(self).toOpaque())
    }

    deinit { ewp_jamming_destroy(ref) }

    func setSignalErp(_ dbm: Double) {
        ewp_jamming_set_signal_erp(ref, dbm)
        signalErpError = ewp_jamming_signal_erp_error(ref)
    }
    func setJammerErp(_ dbm: Double) {
        ewp_jamming_set_jammer_erp(ref, dbm)
        jammerErpError = ewp_jamming_jammer_erp_error(ref)
    }
    func setSignalDist(_ km: Double) {
        ewp_jamming_set_signal_dist(ref, km)
        signalDistError = ewp_jamming_signal_dist_error(ref)
    }
    func setJammerDist(_ km: Double) {
        ewp_jamming_set_jammer_dist(ref, km)
        jammerDistError = ewp_jamming_jammer_dist_error(ref)
    }
    func setSignalHeight(_ m: Double) {
        ewp_jamming_set_signal_height(ref, m)
        signalHeightError = ewp_jamming_signal_height_error(ref)
    }
    func setJammerHeight(_ m: Double) {
        ewp_jamming_set_jammer_height(ref, m)
        jammerHeightError = ewp_jamming_jammer_height_error(ref)
    }
    func setRxHeight(_ m: Double) {
        ewp_jamming_set_rx_height(ref, m)
        rxHeightError = ewp_jamming_rx_height_error(ref)
    }
    func setFrequency(_ mhz: Double) {
        ewp_jamming_set_frequency(ref, mhz)
        frequencyError = ewp_jamming_frequency_error(ref)
    }
    func setRxGainSignal(_ db: Double) {
        ewp_jamming_set_rx_gain_signal(ref, db)
        rxGainSignalError = ewp_jamming_rx_gain_signal_error(ref)
    }
    func setRxGainJammer(_ db: Double) {
        ewp_jamming_set_rx_gain_jammer(ref, db)
        rxGainJammerError = ewp_jamming_rx_gain_jammer_error(ref)
    }
    func setSignalBandwidth(_ mhz: Double) {
        ewp_jamming_set_signal_bandwidth(ref, mhz)
        signalBandwidthError = ewp_jamming_signal_bandwidth_error(ref)
    }
    func setHopRange(_ mhz: Double) {
        ewp_jamming_set_hop_range(ref, mhz)
        hopRangeError = ewp_jamming_hop_range_error(ref)
    }
    func setJsThreshold(_ db: Double) {
        ewp_jamming_set_js_threshold(ref, db)
        jsThresholdError = ewp_jamming_js_threshold_error(ref)
    }

    var defaultSignalErp:       Double { ewp_jamming_signal_erp(ref) }
    var defaultJammerErp:       Double { ewp_jamming_jammer_erp(ref) }
    var defaultSignalDist:      Double { ewp_jamming_signal_dist(ref) }
    var defaultJammerDist:      Double { ewp_jamming_jammer_dist(ref) }
    var defaultSignalHeight:    Double { ewp_jamming_signal_height(ref) }
    var defaultJammerHeight:    Double { ewp_jamming_jammer_height(ref) }
    var defaultRxHeight:        Double { ewp_jamming_rx_height(ref) }
    var defaultFrequency:       Double { ewp_jamming_frequency(ref) }
    var defaultRxGainSignal:    Double { ewp_jamming_rx_gain_signal(ref) }
    var defaultRxGainJammer:    Double { ewp_jamming_rx_gain_jammer(ref) }
    var defaultSignalBandwidth: Double { ewp_jamming_signal_bandwidth(ref) }
    var defaultHopRange:        Double { ewp_jamming_hop_range(ref) }
    var defaultJsThreshold:     Double { ewp_jamming_js_threshold(ref) }
}
