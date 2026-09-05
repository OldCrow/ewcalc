// DopplerAdapter.swift
import Foundation

final class DopplerAdapter: ObservableObject {
    private let ref: EwpDopplerRef
    @Published private(set) var output: EwpDopplerOutput

    // Per-field validation state (#16).
    @Published private(set) var frequencyError:   EwpFieldError
    @Published private(set) var radialSpeedError: EwpFieldError
    @Published private(set) var prfError:         EwpFieldError
    @Published private(set) var bandwidthError:   EwpFieldError
    @Published private(set) var targetRangeError: EwpFieldError
    @Published private(set) var beamwidthAzError: EwpFieldError
    @Published private(set) var beamwidthElError: EwpFieldError

    private static let _cb: EwpDopplerCallback = { out, ctx in
        guard let ctx else { return }
        Unmanaged<DopplerAdapter>.fromOpaque(ctx).takeUnretainedValue().output = out
    }

    init() {
        ref    = ewp_doppler_create()
        output = ewp_doppler_output(ref)
        frequencyError   = ewp_doppler_frequency_error(ref)
        radialSpeedError = ewp_doppler_radial_speed_error(ref)
        prfError         = ewp_doppler_prf_error(ref)
        bandwidthError   = ewp_doppler_bandwidth_error(ref)
        targetRangeError = ewp_doppler_target_range_error(ref)
        beamwidthAzError = ewp_doppler_beamwidth_az_error(ref)
        beamwidthElError = ewp_doppler_beamwidth_el_error(ref)
        // passUnretained: safe only because all adapters are `let` properties of
        // EwCalcStore (@StateObject owned by ewcalcApp) and live until process exit.
        // Do not move adapters to shorter-lived objects without switching to
        // passRetained/takeRetainedValue + clearing the callback in deinit.
        ewp_doppler_set_callback(ref, Self._cb, Unmanaged.passUnretained(self).toOpaque())
    }

    deinit { ewp_doppler_destroy(ref) }

    func setFrequency(_ mhz: Double) {
        ewp_doppler_set_frequency(ref, mhz)
        frequencyError = ewp_doppler_frequency_error(ref)
    }
    func setRadialSpeed(_ mps: Double) {
        ewp_doppler_set_radial_speed(ref, mps)
        radialSpeedError = ewp_doppler_radial_speed_error(ref)
    }
    func setPrf(_ hz: Double) {
        ewp_doppler_set_prf(ref, hz)
        prfError = ewp_doppler_prf_error(ref)
    }
    func setBandwidth(_ mhz: Double) {
        ewp_doppler_set_bandwidth(ref, mhz)
        bandwidthError = ewp_doppler_bandwidth_error(ref)
    }
    func setTargetRange(_ km: Double) {
        ewp_doppler_set_target_range(ref, km)
        targetRangeError = ewp_doppler_target_range_error(ref)
    }
    func setBeamwidthAz(_ deg: Double) {
        ewp_doppler_set_beamwidth_az(ref, deg)
        beamwidthAzError = ewp_doppler_beamwidth_az_error(ref)
    }
    func setBeamwidthEl(_ deg: Double) {
        ewp_doppler_set_beamwidth_el(ref, deg)
        beamwidthElError = ewp_doppler_beamwidth_el_error(ref)
    }

    var defaultFrequency:   Double { ewp_doppler_frequency(ref) }
    var defaultRadialSpeed: Double { ewp_doppler_radial_speed(ref) }
    var defaultPrf:         Double { ewp_doppler_prf(ref) }
    var defaultBandwidth:   Double { ewp_doppler_bandwidth(ref) }
    var defaultTargetRange: Double { ewp_doppler_target_range(ref) }
    var defaultBeamwidthAz: Double { ewp_doppler_beamwidth_az(ref) }
    var defaultBeamwidthEl: Double { ewp_doppler_beamwidth_el(ref) }
}
