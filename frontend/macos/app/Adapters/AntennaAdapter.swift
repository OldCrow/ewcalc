// AntennaAdapter.swift
import Foundation

final class AntennaAdapter: ObservableObject {
    private let ref: EwpAntennaRef
    @Published private(set) var output: EwpAntennaOutput

    private static let _cb: EwpAntennaCallback = { out, ctx in
        guard let ctx else { return }
        Unmanaged<AntennaAdapter>.fromOpaque(ctx).takeUnretainedValue().output = out
    }

    init() {
        ref    = ewp_antenna_create()
        output = ewp_antenna_output(ref)
        ewp_antenna_set_callback(ref, Self._cb, Unmanaged.passUnretained(self).toOpaque())
    }

    deinit { ewp_antenna_destroy(ref) }

    func setGain(_ dbi: Double)        { ewp_antenna_set_gain(ref, dbi) }
    func setAzBeamwidth(_ deg: Double) { ewp_antenna_set_az_beamwidth(ref, deg) }
    func setElBeamwidth(_ deg: Double) { ewp_antenna_set_el_beamwidth(ref, deg) }
    func setTxPower(_ dbm: Double)     { ewp_antenna_set_tx_power(ref, dbm) }
    func setFrequency(_ mhz: Double)   { ewp_antenna_set_frequency(ref, mhz) }

    var defaultGain:        Double { ewp_antenna_gain(ref) }
    var defaultAzBeamwidth: Double { ewp_antenna_az_beamwidth(ref) }
    var defaultElBeamwidth: Double { ewp_antenna_el_beamwidth(ref) }
    var defaultTxPower:     Double { ewp_antenna_tx_power(ref) }
    var defaultFrequency:   Double { ewp_antenna_frequency(ref) }
}
