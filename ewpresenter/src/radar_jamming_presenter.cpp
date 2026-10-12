#include "ewpresenter/radar_jamming_presenter.h"
#include "ewpresenter/formatter.h"
#include "libew/jamming/jamming.h"

namespace ewpresenter {

RadarJammingPresenter::RadarJammingPresenter() noexcept { recompute(); }

void RadarJammingPresenter::set_geometry(int g) noexcept {
    geometry_ = g;
    geometry_err_ = validate_bounds(static_cast<double>(g),
                                    static_cast<double>(kSelfProtection),
                                    static_cast<double>(kStandOff));
    recompute(); fire();
}
void RadarJammingPresenter::set_radar_erp(double dbm) noexcept {
    update_field(dbm, radar_erp_dbm_, radar_erp_err_, validate_bounds(dbm, -100.0, 200.0));
}
void RadarJammingPresenter::set_jammer_erp(double dbm) noexcept {
    update_field(dbm, jammer_erp_dbm_, jammer_erp_err_, validate_bounds(dbm, -100.0, 200.0));
}
void RadarJammingPresenter::set_target_rcs(double dbsm) noexcept {
    update_field(dbsm, target_rcs_dbsm_, target_rcs_err_, validate_bounds(dbsm, -40.0, 60.0));
}
void RadarJammingPresenter::set_target_range(double km) noexcept {
    update_field(km, target_range_km_, target_range_err_, validate_positive_bounded(km, 0.01, 10000.0));
}
void RadarJammingPresenter::set_jammer_range(double km) noexcept {
    update_field(km, jammer_range_km_, jammer_range_err_, validate_positive_bounded(km, 0.01, 10000.0));
}
void RadarJammingPresenter::set_mainbeam_gain(double db) noexcept {
    update_field(db, mainbeam_gain_db_, mainbeam_gain_err_, validate_bounds(db, -30.0, 60.0));
}
void RadarJammingPresenter::set_sidelobe_gain(double db) noexcept {
    update_field(db, sidelobe_gain_db_, sidelobe_gain_err_, validate_bounds(db, -30.0, 60.0));
}
void RadarJammingPresenter::set_js_threshold(double db) noexcept {
    update_field(db, js_threshold_db_, js_threshold_err_, validate_bounds(db, -30.0, 30.0));
}

void RadarJammingPresenter::recompute() noexcept {
    const bool stand_off = geometry_ == kStandOff;

    // Stand-off-only fields do not block a self-protection result.
    const bool common_ok = geometry_err_     == FieldError::none &&
                           radar_erp_err_    == FieldError::none &&
                           jammer_erp_err_   == FieldError::none &&
                           target_rcs_err_   == FieldError::none &&
                           target_range_err_ == FieldError::none &&
                           js_threshold_err_ == FieldError::none;
    const bool soj_ok    = jammer_range_err_  == FieldError::none &&
                           mainbeam_gain_err_ == FieldError::none &&
                           sidelobe_gain_err_ == FieldError::none;

    output_.valid = common_ok && (!stand_off || soj_ok);

    if (!output_.valid) {
        output_.jammer_effective      = false;
        output_.js_ratio_str          = kDash;
        output_.js_margin_str         = kDash;
        output_.burnthrough_range_str = kDash;
        output_.status_str            = kDash;
        return;
    }

    namespace jam = libew::jamming;
    const Dbm  radar_erp{radar_erp_dbm_};
    const Dbm  jammer_erp{jammer_erp_dbm_};
    const Dbsm rcs{target_rcs_dbsm_};
    const Db   threshold{js_threshold_db_};

    if (stand_off) {
        const Km rj{jammer_range_km_};
        const Db gm{mainbeam_gain_db_};
        const Db gs{sidelobe_gain_db_};
        output_.js_ratio = jam::stand_off_js(radar_erp, jammer_erp, Km{target_range_km_},
                                             rj, gm, gs, rcs);
        output_.burnthrough_range = jam::stand_off_burnthrough(radar_erp, jammer_erp,
                                                               rj, gm, gs, rcs, threshold);
    } else {
        output_.js_ratio = jam::self_protection_js(radar_erp, jammer_erp,
                                                   Km{target_range_km_}, rcs);
        output_.burnthrough_range = jam::self_protection_burnthrough(radar_erp, jammer_erp,
                                                                     rcs, threshold);
    }

    output_.js_margin        = Db{output_.js_ratio.value - js_threshold_db_};
    output_.jammer_effective = output_.js_margin.value >= 0.0;

    output_.js_ratio_str          = format_db(output_.js_ratio);
    output_.js_margin_str         = format_db(output_.js_margin);
    output_.burnthrough_range_str = format_km(output_.burnthrough_range);
    output_.status_str            = output_.jammer_effective ? "Jammer effective"
                                                             : "Radar burns through";
}

void RadarJammingPresenter::fire() noexcept {
    if (on_change_) on_change_(output_);
}

} // namespace ewpresenter
