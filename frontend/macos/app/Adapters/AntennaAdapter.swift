// AntennaAdapter.swift
import Foundation

final class AntennaAdapter: ObservableObject {
    private let ref: EwpAntennaRef
    @Published private(set) var output: EwpAntennaOutput

    // Per-field validation state (#16).
    @Published private(set) var gainError:        EwpFieldError
    @Published private(set) var azBeamwidthError: EwpFieldError
    @Published private(set) var elBeamwidthError: EwpFieldError
    @Published private(set) var txPowerError:     EwpFieldError
    @Published private(set) var frequencyError:   EwpFieldError

    private static let _cb: EwpAntennaCallback = { out, ctx in
        guard let ctx else { return }
        Unmanaged<AntennaAdapter>.fromOpaque(ctx).takeUnretainedValue().output = out
    }

    init() {
        ref    = ewp_antenna_create()
        output = ewp_antenna_output(ref)
        gainError        = ewp_antenna_gain_error(ref)
        azBeamwidthError = ewp_antenna_az_beamwidth_error(ref)
        elBeamwidthError = ewp_antenna_el_beamwidth_error(ref)
        txPowerError     = ewp_antenna_tx_power_error(ref)
        frequencyError   = ewp_antenna_frequency_error(ref)
        // passUnretained: safe only because all adapters are `let` properties of
        // EwCalcStore (@StateObject owned by ewcalcApp) and live until process exit.
        // Do not move adapters to shorter-lived objects without switching to
        // passRetained/takeRetainedValue + clearing the callback in deinit.
        ewp_antenna_set_callback(ref, Self._cb, Unmanaged.passUnretained(self).toOpaque())
    }

    deinit { ewp_antenna_destroy(ref) }

    func setGain(_ dbi: Double) {
        ewp_antenna_set_gain(ref, dbi)
        gainError = ewp_antenna_gain_error(ref)
    }
    func setAzBeamwidth(_ deg: Double) {
        ewp_antenna_set_az_beamwidth(ref, deg)
        azBeamwidthError = ewp_antenna_az_beamwidth_error(ref)
    }
    func setElBeamwidth(_ deg: Double) {
        ewp_antenna_set_el_beamwidth(ref, deg)
        elBeamwidthError = ewp_antenna_el_beamwidth_error(ref)
    }
    func setTxPower(_ dbm: Double) {
        ewp_antenna_set_tx_power(ref, dbm)
        txPowerError = ewp_antenna_tx_power_error(ref)
    }
    func setFrequency(_ mhz: Double) {
        ewp_antenna_set_frequency(ref, mhz)
        frequencyError = ewp_antenna_frequency_error(ref)
    }

    var defaultGain:        Double { ewp_antenna_gain(ref) }
    var defaultAzBeamwidth: Double { ewp_antenna_az_beamwidth(ref) }
    var defaultElBeamwidth: Double { ewp_antenna_el_beamwidth(ref) }
    var defaultTxPower:     Double { ewp_antenna_tx_power(ref) }
    var defaultFrequency:   Double { ewp_antenna_frequency(ref) }
}
