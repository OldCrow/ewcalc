// RadarAdapter.swift
import Foundation

final class RadarAdapter: ObservableObject {
    private let ref: EwpRadarRef
    @Published private(set) var output: EwpRadarOutput

    private static let _cb: EwpRadarCallback = { out, ctx in
        guard let ctx else { return }
        Unmanaged<RadarAdapter>.fromOpaque(ctx).takeUnretainedValue().output = out
    }

    init() {
        ref    = ewp_radar_create()
        output = ewp_radar_output(ref)
        // passUnretained: safe only because all adapters are `let` properties of
        // EwCalcStore (@StateObject owned by ewcalcApp) and live until process exit.
        // Do not move adapters to shorter-lived objects without switching to
        // passRetained/takeRetainedValue + clearing the callback in deinit.
        ewp_radar_set_callback(ref, Self._cb, Unmanaged.passUnretained(self).toOpaque())
    }

    deinit { ewp_radar_destroy(ref) }

    func setTxPower(_ dbm: Double)      { ewp_radar_set_tx_power(ref, dbm) }
    func setAntennaGain(_ dbi: Double)  { ewp_radar_set_antenna_gain(ref, dbi) }
    func setTargetRcs(_ dbsm: Double)   { ewp_radar_set_target_rcs(ref, dbsm) }
    func setFrequency(_ mhz: Double)    { ewp_radar_set_frequency(ref, mhz) }
    func setSystemLosses(_ db: Double)  { ewp_radar_set_system_losses(ref, db) }
    func setNoiseFigure(_ db: Double)   { ewp_radar_set_noise_figure(ref, db) }
    func setBandwidth(_ mhz: Double)    { ewp_radar_set_bandwidth(ref, mhz) }
    func setRequiredSnr(_ db: Double)   { ewp_radar_set_required_snr(ref, db) }
    func setTimeBandwidth(_ tb: Double) { ewp_radar_set_time_bandwidth(ref, tb) }
    func setNumPulses(_ n: Int)         { ewp_radar_set_num_pulses(ref, Int32(n)) }

    var defaultTxPower:      Double { ewp_radar_tx_power(ref) }
    var defaultAntennaGain:  Double { ewp_radar_antenna_gain(ref) }
    var defaultTargetRcs:    Double { ewp_radar_target_rcs(ref) }
    var defaultFrequency:    Double { ewp_radar_frequency(ref) }
    var defaultSystemLosses: Double { ewp_radar_system_losses(ref) }
    var defaultNoiseFigure:  Double { ewp_radar_noise_figure(ref) }
    var defaultBandwidth:    Double { ewp_radar_bandwidth(ref) }
    var defaultRequiredSnr:  Double { ewp_radar_required_snr(ref) }
    var defaultTimeBandwidth:Double { ewp_radar_time_bandwidth(ref) }
    var defaultNumPulses:    Int    { Int(ewp_radar_num_pulses(ref)) }
}
