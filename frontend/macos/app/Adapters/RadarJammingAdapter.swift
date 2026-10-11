// RadarJammingAdapter.swift
import Foundation

final class RadarJammingAdapter: ObservableObject {
    private let ref: EwpRadarJammingRef
    @Published private(set) var output: EwpRadarJammingOutput

    // Per-field validation state (#16).
    @Published private(set) var geometryError:     EwpFieldError
    @Published private(set) var radarErpError:     EwpFieldError
    @Published private(set) var jammerErpError:    EwpFieldError
    @Published private(set) var targetRcsError:    EwpFieldError
    @Published private(set) var targetRangeError:  EwpFieldError
    @Published private(set) var jammerRangeError:  EwpFieldError
    @Published private(set) var mainbeamGainError: EwpFieldError
    @Published private(set) var sidelobeGainError: EwpFieldError
    @Published private(set) var jsThresholdError:  EwpFieldError

    private static let _cb: EwpRadarJammingCallback = { out, ctx in
        guard let ctx else { return }
        Unmanaged<RadarJammingAdapter>.fromOpaque(ctx).takeUnretainedValue().output = out
    }

    init() {
        ref    = ewp_radar_jamming_create()
        output = ewp_radar_jamming_output(ref)
        geometryError     = ewp_radar_jamming_geometry_error(ref)
        radarErpError     = ewp_radar_jamming_radar_erp_error(ref)
        jammerErpError    = ewp_radar_jamming_jammer_erp_error(ref)
        targetRcsError    = ewp_radar_jamming_target_rcs_error(ref)
        targetRangeError  = ewp_radar_jamming_target_range_error(ref)
        jammerRangeError  = ewp_radar_jamming_jammer_range_error(ref)
        mainbeamGainError = ewp_radar_jamming_mainbeam_gain_error(ref)
        sidelobeGainError = ewp_radar_jamming_sidelobe_gain_error(ref)
        jsThresholdError  = ewp_radar_jamming_js_threshold_error(ref)
        // passUnretained: safe only because all adapters are `let` properties of
        // EwCalcStore (@StateObject owned by ewcalcApp) and live until process exit.
        // Do not move adapters to shorter-lived objects without switching to
        // passRetained/takeRetainedValue + clearing the callback in deinit.
        ewp_radar_jamming_set_callback(ref, Self._cb, Unmanaged.passUnretained(self).toOpaque())
    }

    deinit { ewp_radar_jamming_destroy(ref) }

    func setGeometry(_ geometry: Int) {
        ewp_radar_jamming_set_geometry(ref, Int32(geometry))
        geometryError = ewp_radar_jamming_geometry_error(ref)
    }
    func setRadarErp(_ dbm: Double) {
        ewp_radar_jamming_set_radar_erp(ref, dbm)
        radarErpError = ewp_radar_jamming_radar_erp_error(ref)
    }
    func setJammerErp(_ dbm: Double) {
        ewp_radar_jamming_set_jammer_erp(ref, dbm)
        jammerErpError = ewp_radar_jamming_jammer_erp_error(ref)
    }
    func setTargetRcs(_ dbsm: Double) {
        ewp_radar_jamming_set_target_rcs(ref, dbsm)
        targetRcsError = ewp_radar_jamming_target_rcs_error(ref)
    }
    func setTargetRange(_ km: Double) {
        ewp_radar_jamming_set_target_range(ref, km)
        targetRangeError = ewp_radar_jamming_target_range_error(ref)
    }
    func setJammerRange(_ km: Double) {
        ewp_radar_jamming_set_jammer_range(ref, km)
        jammerRangeError = ewp_radar_jamming_jammer_range_error(ref)
    }
    func setMainbeamGain(_ db: Double) {
        ewp_radar_jamming_set_mainbeam_gain(ref, db)
        mainbeamGainError = ewp_radar_jamming_mainbeam_gain_error(ref)
    }
    func setSidelobeGain(_ db: Double) {
        ewp_radar_jamming_set_sidelobe_gain(ref, db)
        sidelobeGainError = ewp_radar_jamming_sidelobe_gain_error(ref)
    }
    func setJsThreshold(_ db: Double) {
        ewp_radar_jamming_set_js_threshold(ref, db)
        jsThresholdError = ewp_radar_jamming_js_threshold_error(ref)
    }

    var defaultGeometry: Int { Int(ewp_radar_jamming_geometry(ref)) }
    var defaultRadarErp: Double { ewp_radar_jamming_radar_erp(ref) }
    var defaultJammerErp: Double { ewp_radar_jamming_jammer_erp(ref) }
    var defaultTargetRcs: Double { ewp_radar_jamming_target_rcs(ref) }
    var defaultTargetRange: Double { ewp_radar_jamming_target_range(ref) }
    var defaultJammerRange: Double { ewp_radar_jamming_jammer_range(ref) }
    var defaultMainbeamGain: Double { ewp_radar_jamming_mainbeam_gain(ref) }
    var defaultSidelobeGain: Double { ewp_radar_jamming_sidelobe_gain(ref) }
    var defaultJsThreshold: Double { ewp_radar_jamming_js_threshold(ref) }
}
