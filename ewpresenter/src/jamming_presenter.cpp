#include "ewpresenter/jamming_presenter.h"
#include "ewpresenter/formatter.h"
#include "libew/jamming/jamming.h"

namespace ewpresenter {

static constexpr const char* DASH = "\xe2\x80\x94";

JammingPresenter::JammingPresenter() noexcept { recompute(); }

void JammingPresenter::set_signal_erp(double dbm) noexcept {
    signal_erp_dbm_ = dbm;
    signal_erp_err_ = validate_bounds(dbm, -100.0, 200.0);
    recompute(); fire();
}
void JammingPresenter::set_jammer_erp(double dbm) noexcept {
    jammer_erp_dbm_ = dbm;
    jammer_erp_err_ = validate_bounds(dbm, -100.0, 200.0);
    recompute(); fire();
}
void JammingPresenter::set_signal_to_rx_dist(double km) noexcept {
    signal_to_rx_dist_km_ = km;
    signal_to_rx_err_ = validate_positive_bounded(km, 0.01, 10000.0);
    recompute(); fire();
}
void JammingPresenter::set_jammer_to_rx_dist(double km) noexcept {
    jammer_to_rx_dist_km_ = km;
    jammer_to_rx_err_ = validate_positive_bounded(km, 0.01, 10000.0);
    recompute(); fire();
}
void JammingPresenter::set_signal_tx_height(double meters) noexcept {
    signal_tx_height_m_ = meters;
    signal_height_err_ = validate_positive_bounded(meters, 0.1, 100000.0);
    recompute(); fire();
}
void JammingPresenter::set_jammer_height(double meters) noexcept {
    jammer_height_m_ = meters;
    jammer_height_err_ = validate_positive_bounded(meters, 0.1, 100000.0);
    recompute(); fire();
}
void JammingPresenter::set_rx_height(double meters) noexcept {
    rx_height_m_ = meters;
    rx_height_err_ = validate_positive_bounded(meters, 0.1, 100000.0);
    recompute(); fire();
}
void JammingPresenter::set_frequency(double mhz) noexcept {
    frequency_mhz_ = mhz;
    frequency_err_ = validate_positive_bounded(mhz, 0.1, 100000.0);
    recompute(); fire();
}
void JammingPresenter::set_rx_gain_signal(double db) noexcept {
    rx_gain_signal_db_ = db;
    recompute(); fire();
}
void JammingPresenter::set_rx_gain_jammer(double db) noexcept {
    rx_gain_jammer_db_ = db;
    recompute(); fire();
}
void JammingPresenter::set_signal_bandwidth(double mhz) noexcept {
    signal_bandwidth_mhz_ = mhz;
    signal_bandwidth_err_ = validate_positive_bounded(mhz, 0.001, 1000.0);
    recompute(); fire();
}
void JammingPresenter::set_hop_range(double mhz) noexcept {
    hop_range_mhz_ = mhz;
    hop_range_err_ = validate_positive_bounded(mhz, 0.001, 10000.0);
    recompute(); fire();
}
void JammingPresenter::set_single_channel_js(double db) noexcept {
    single_channel_js_db_ = db;
    recompute(); fire();
}

void JammingPresenter::recompute() noexcept {
    output_.valid = (signal_erp_err_       == FieldError::none &&
                     jammer_erp_err_       == FieldError::none &&
                     signal_to_rx_err_     == FieldError::none &&
                     jammer_to_rx_err_     == FieldError::none &&
                     signal_height_err_    == FieldError::none &&
                     jammer_height_err_    == FieldError::none &&
                     rx_height_err_        == FieldError::none &&
                     frequency_err_        == FieldError::none &&
                     signal_bandwidth_err_ == FieldError::none &&
                     hop_range_err_        == FieldError::none);

    if (!output_.valid) {
        output_.js_ratio_str      = DASH;
        output_.signal_at_rx_str  = DASH;
        output_.jammer_at_rx_str  = DASH;
        output_.optimum_bw_str    = DASH;
        output_.duty_cycle_str    = DASH;
        return;
    }

    using namespace libew::units;

    const auto js = libew::jamming::comms_jamming_js(
        Dbm{signal_erp_dbm_}, Dbm{jammer_erp_dbm_},
        Km{signal_to_rx_dist_km_}, Km{jammer_to_rx_dist_km_},
        Meters{signal_tx_height_m_}, Meters{jammer_height_m_}, Meters{rx_height_m_},
        Mhz{frequency_mhz_},
        Db{rx_gain_signal_db_}, Db{rx_gain_jammer_db_});

    output_.js_ratio    = js.js_ratio;
    output_.signal_at_rx= js.signal_power_at_receiver;
    output_.jammer_at_rx= js.jammer_power_at_receiver;

    // Feed computed J/S into partial-band calculation
    single_channel_js_db_ = js.js_ratio.value;

    const auto pb = libew::jamming::partial_band_jamming(
        Mhz{signal_bandwidth_mhz_}, Mhz{hop_range_mhz_}, Db{single_channel_js_db_});

    output_.optimum_bw = pb.optimum_jamming_bandwidth;
    output_.duty_cycle = pb.duty_cycle;

    output_.js_ratio_str     = format_db(output_.js_ratio);
    output_.signal_at_rx_str = format_dbm(output_.signal_at_rx);
    output_.jammer_at_rx_str = format_dbm(output_.jammer_at_rx);
    output_.optimum_bw_str   = format_mhz(output_.optimum_bw, 3);
    output_.duty_cycle_str   = format_percent(output_.duty_cycle);
}

void JammingPresenter::fire() noexcept {
    if (on_change_) on_change_(output_);
}

} // namespace ewpresenter
