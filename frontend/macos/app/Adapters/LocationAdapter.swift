// LocationAdapter.swift
import Foundation

final class LocationAdapter: ObservableObject {
    private let ref: EwpLocationRef
    @Published private(set) var output: EwpLocationOutput

    // Per-field validation state (#16). eepAxisError is a cross-field check
    // (semi_minor must not exceed semi_major) refreshed by both axis setters.
    @Published private(set) var rmsBearingFieldError: EwpFieldError
    @Published private(set) var aoaRangeError:        EwpFieldError
    @Published private(set) var rmsTimeFieldError:    EwpFieldError
    @Published private(set) var baselineError:        EwpFieldError
    @Published private(set) var semiMajorError:       EwpFieldError
    @Published private(set) var semiMinorError:       EwpFieldError
    @Published private(set) var eepAxisError:         EwpFieldError

    private static let _cb: EwpLocationCallback = { out, ctx in
        guard let ctx else { return }
        Unmanaged<LocationAdapter>.fromOpaque(ctx).takeUnretainedValue().output = out
    }

    init() {
        ref    = ewp_location_create()
        output = ewp_location_output(ref)
        rmsBearingFieldError = ewp_location_rms_bearing_field_error(ref)
        aoaRangeError        = ewp_location_aoa_range_error(ref)
        rmsTimeFieldError    = ewp_location_rms_time_field_error(ref)
        baselineError        = ewp_location_baseline_error(ref)
        semiMajorError       = ewp_location_semi_major_error(ref)
        semiMinorError       = ewp_location_semi_minor_error(ref)
        eepAxisError         = ewp_location_eep_axis_error(ref)
        // passUnretained: safe only because all adapters are `let` properties of
        // EwCalcStore (@StateObject owned by ewcalcApp) and live until process exit.
        // Do not move adapters to shorter-lived objects without switching to
        // passRetained/takeRetainedValue + clearing the callback in deinit.
        ewp_location_set_callback(ref, Self._cb, Unmanaged.passUnretained(self).toOpaque())
    }

    deinit { ewp_location_destroy(ref) }

    func setRmsBearingError(_ deg: Double) {
        ewp_location_set_rms_bearing_error(ref, deg)
        rmsBearingFieldError = ewp_location_rms_bearing_field_error(ref)
    }
    func setAoaRange(_ km: Double) {
        ewp_location_set_aoa_range(ref, km)
        aoaRangeError = ewp_location_aoa_range_error(ref)
    }
    func setRmsTimeError(_ ns: Double) {
        ewp_location_set_rms_time_error(ref, ns)
        rmsTimeFieldError = ewp_location_rms_time_field_error(ref)
    }
    func setBaseline(_ km: Double) {
        ewp_location_set_baseline(ref, km)
        baselineError = ewp_location_baseline_error(ref)
    }
    func setSemiMajor(_ km: Double) {
        ewp_location_set_semi_major(ref, km)
        semiMajorError = ewp_location_semi_major_error(ref)
        eepAxisError   = ewp_location_eep_axis_error(ref)
    }
    func setSemiMinor(_ km: Double) {
        ewp_location_set_semi_minor(ref, km)
        semiMinorError = ewp_location_semi_minor_error(ref)
        eepAxisError   = ewp_location_eep_axis_error(ref)
    }

    var defaultRmsBearingError: Double { ewp_location_rms_bearing_error(ref) }
    var defaultAoaRange:        Double { ewp_location_aoa_range(ref) }
    var defaultRmsTimeError:    Double { ewp_location_rms_time_error(ref) }
    var defaultBaseline:        Double { ewp_location_baseline(ref) }
    var defaultSemiMajor:       Double { ewp_location_semi_major(ref) }
    var defaultSemiMinor:       Double { ewp_location_semi_minor(ref) }
}
