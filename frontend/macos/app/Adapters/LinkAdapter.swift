// LinkAdapter.swift
import Foundation

final class LinkAdapter: ObservableObject {
    private let ref: EwpLinkRef
    @Published private(set) var output: EwpLinkOutput

    // Per-field validation state (#16).
    @Published private(set) var txPowerError:       EwpFieldError
    @Published private(set) var txGainError:        EwpFieldError
    @Published private(set) var rxGainError:        EwpFieldError
    @Published private(set) var distanceError:      EwpFieldError
    @Published private(set) var txHeightError:      EwpFieldError
    @Published private(set) var rxHeightError:      EwpFieldError
    @Published private(set) var frequencyError:     EwpFieldError
    @Published private(set) var rxSensitivityError: EwpFieldError

    private static let _cb: EwpLinkCallback = { out, ctx in
        guard let ctx else { return }
        Unmanaged<LinkAdapter>.fromOpaque(ctx).takeUnretainedValue().output = out
    }

    init() {
        ref    = ewp_link_create()
        output = ewp_link_output(ref)
        txPowerError       = ewp_link_tx_power_error(ref)
        txGainError        = ewp_link_tx_gain_error(ref)
        rxGainError        = ewp_link_rx_gain_error(ref)
        distanceError      = ewp_link_distance_error(ref)
        txHeightError      = ewp_link_tx_height_error(ref)
        rxHeightError      = ewp_link_rx_height_error(ref)
        frequencyError     = ewp_link_frequency_error(ref)
        rxSensitivityError = ewp_link_rx_sensitivity_error(ref)
        // passUnretained: safe only because all adapters are `let` properties of
        // EwCalcStore (@StateObject owned by ewcalcApp) and live until process exit.
        // Do not move adapters to shorter-lived objects without switching to
        // passRetained/takeRetainedValue + clearing the callback in deinit.
        ewp_link_set_callback(ref, Self._cb, Unmanaged.passUnretained(self).toOpaque())
    }

    deinit { ewp_link_destroy(ref) }

    func setTxPower(_ dbm: Double) {
        ewp_link_set_tx_power(ref, dbm)
        txPowerError = ewp_link_tx_power_error(ref)
    }
    func setTxGain(_ db: Double) {
        ewp_link_set_tx_gain(ref, db)
        txGainError = ewp_link_tx_gain_error(ref)
    }
    func setRxGain(_ db: Double) {
        ewp_link_set_rx_gain(ref, db)
        rxGainError = ewp_link_rx_gain_error(ref)
    }
    func setDistance(_ km: Double) {
        ewp_link_set_distance(ref, km)
        distanceError = ewp_link_distance_error(ref)
    }
    func setTxHeight(_ m: Double) {
        ewp_link_set_tx_height(ref, m)
        txHeightError = ewp_link_tx_height_error(ref)
    }
    func setRxHeight(_ m: Double) {
        ewp_link_set_rx_height(ref, m)
        rxHeightError = ewp_link_rx_height_error(ref)
    }
    func setFrequency(_ mhz: Double) {
        ewp_link_set_frequency(ref, mhz)
        frequencyError = ewp_link_frequency_error(ref)
    }
    func setRxSensitivity(_ dbm: Double) {
        ewp_link_set_rx_sensitivity(ref, dbm)
        rxSensitivityError = ewp_link_rx_sensitivity_error(ref)
    }

    var defaultTxPower:       Double { ewp_link_tx_power(ref) }
    var defaultTxGain:        Double { ewp_link_tx_gain(ref) }
    var defaultRxGain:        Double { ewp_link_rx_gain(ref) }
    var defaultDistance:      Double { ewp_link_distance(ref) }
    var defaultTxHeight:      Double { ewp_link_tx_height(ref) }
    var defaultRxHeight:      Double { ewp_link_rx_height(ref) }
    var defaultFrequency:     Double { ewp_link_frequency(ref) }
    var defaultRxSensitivity: Double { ewp_link_rx_sensitivity(ref) }
}
