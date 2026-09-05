// DetectionAdapter.swift
import Foundation

final class DetectionAdapter: ObservableObject {
    private let ref: EwpDetectionRef
    @Published private(set) var output: EwpDetectionOutput

    // Per-field validation state (#16).
    @Published private(set) var pdError:           EwpFieldError
    @Published private(set) var pfaExponentError:  EwpFieldError
    @Published private(set) var numPulsesError:    EwpFieldError
    @Published private(set) var swerlingCaseError: EwpFieldError
    @Published private(set) var beamwidthError:    EwpFieldError
    @Published private(set) var scanRateError:     EwpFieldError
    @Published private(set) var prfError:          EwpFieldError
    @Published private(set) var bandwidthError:    EwpFieldError

    private static let _cb: EwpDetectionCallback = { out, ctx in
        guard let ctx else { return }
        Unmanaged<DetectionAdapter>.fromOpaque(ctx).takeUnretainedValue().output = out
    }

    init() {
        ref    = ewp_detection_create()
        output = ewp_detection_output(ref)
        pdError           = ewp_detection_pd_error(ref)
        pfaExponentError  = ewp_detection_pfa_exponent_error(ref)
        numPulsesError    = ewp_detection_num_pulses_error(ref)
        swerlingCaseError = ewp_detection_swerling_case_error(ref)
        beamwidthError    = ewp_detection_beamwidth_error(ref)
        scanRateError     = ewp_detection_scan_rate_error(ref)
        prfError          = ewp_detection_prf_error(ref)
        bandwidthError    = ewp_detection_bandwidth_error(ref)
        // passUnretained: safe only because all adapters are `let` properties of
        // EwCalcStore (@StateObject owned by ewcalcApp) and live until process exit.
        // Do not move adapters to shorter-lived objects without switching to
        // passRetained/takeRetainedValue + clearing the callback in deinit.
        ewp_detection_set_callback(ref, Self._cb, Unmanaged.passUnretained(self).toOpaque())
    }

    deinit { ewp_detection_destroy(ref) }

    func setPd(_ pd: Double) {
        ewp_detection_set_pd(ref, pd)
        pdError = ewp_detection_pd_error(ref)
    }
    func setPfaExponent(_ exponent: Double) {
        ewp_detection_set_pfa_exponent(ref, exponent)
        pfaExponentError = ewp_detection_pfa_exponent_error(ref)
    }
    func setNumPulses(_ n: Int) {
        ewp_detection_set_num_pulses(ref, Int32(n))
        numPulsesError = ewp_detection_num_pulses_error(ref)
    }
    func setSwerlingCase(_ swerling: Int) {
        ewp_detection_set_swerling_case(ref, Int32(swerling))
        swerlingCaseError = ewp_detection_swerling_case_error(ref)
    }
    func setBeamwidth(_ deg: Double) {
        ewp_detection_set_beamwidth(ref, deg)
        beamwidthError = ewp_detection_beamwidth_error(ref)
    }
    func setScanRate(_ degS: Double) {
        ewp_detection_set_scan_rate(ref, degS)
        scanRateError = ewp_detection_scan_rate_error(ref)
    }
    func setPrf(_ hz: Double) {
        ewp_detection_set_prf(ref, hz)
        prfError = ewp_detection_prf_error(ref)
    }
    func setBandwidth(_ mhz: Double) {
        ewp_detection_set_bandwidth(ref, mhz)
        bandwidthError = ewp_detection_bandwidth_error(ref)
    }

    var defaultPd:           Double { ewp_detection_pd(ref) }
    var defaultPfaExponent:  Double { ewp_detection_pfa_exponent(ref) }
    var defaultNumPulses:    Int { Int(ewp_detection_num_pulses(ref)) }
    var defaultSwerlingCase: Int { Int(ewp_detection_swerling_case(ref)) }
    var defaultBeamwidth:    Double { ewp_detection_beamwidth(ref) }
    var defaultScanRate:     Double { ewp_detection_scan_rate(ref) }
    var defaultPrf:          Double { ewp_detection_prf(ref) }
    var defaultBandwidth:    Double { ewp_detection_bandwidth(ref) }
}
