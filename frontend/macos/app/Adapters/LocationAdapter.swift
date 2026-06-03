// LocationAdapter.swift
import Foundation

final class LocationAdapter: ObservableObject {
    private let ref: EwpLocationRef
    @Published private(set) var output: EwpLocationOutput

    private static let _cb: EwpLocationCallback = { out, ctx in
        guard let ctx else { return }
        Unmanaged<LocationAdapter>.fromOpaque(ctx).takeUnretainedValue().output = out
    }

    init() {
        ref    = ewp_location_create()
        output = ewp_location_output(ref)
        ewp_location_set_callback(ref, Self._cb, Unmanaged.passUnretained(self).toOpaque())
    }

    deinit { ewp_location_destroy(ref) }

    func setRmsBearingError(_ deg: Double) { ewp_location_set_rms_bearing_error(ref, deg) }
    func setAoaRange(_ km: Double)          { ewp_location_set_aoa_range(ref, km) }
    func setRmsTimeError(_ ns: Double)      { ewp_location_set_rms_time_error(ref, ns) }
    func setBaseline(_ km: Double)          { ewp_location_set_baseline(ref, km) }
    func setSemiMajor(_ km: Double)         { ewp_location_set_semi_major(ref, km) }
    func setSemiMinor(_ km: Double)         { ewp_location_set_semi_minor(ref, km) }

    var defaultRmsBearingError: Double { ewp_location_rms_bearing_error(ref) }
    var defaultAoaRange:        Double { ewp_location_aoa_range(ref) }
    var defaultRmsTimeError:    Double { ewp_location_rms_time_error(ref) }
    var defaultBaseline:        Double { ewp_location_baseline(ref) }
    var defaultSemiMajor:       Double { ewp_location_semi_major(ref) }
    var defaultSemiMinor:       Double { ewp_location_semi_minor(ref) }
}
