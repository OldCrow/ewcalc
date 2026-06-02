#include "ewpresenter/radar_presenter.h"
#include "ewpresenter/formatter.h"
#include "libew/radar/radar.h"

namespace ewpresenter {

static constexpr const char* DASH = "\xe2\x80\x94";

RadarPresenter::RadarPresenter() noexcept { recompute(); }

void RadarPresenter::set_tx_power(double dbm) noexcept {
    tx_power_dbm_ = dbm;
    tx_power_err_ = validate_bounds(dbm, -50.0, 200.0);
    recompute(); fire();
}
void RadarPresenter::set_antenna_gain(double dbi) noexcept {
    antenna_gain_dbi_ = dbi;
    antenna_gain_err_ = validate_bounds(dbi, -30.0, 60.0);
    recompute(); fire();
}
void RadarPresenter::set_target_rcs(double dbsm) noexcept {
    target_rcs_dbsm_ = dbsm;
    target_rcs_err_ = validate_bounds(dbsm, -40.0, 60.0);
    recompute(); fire();
}
void RadarPresenter::set_frequency(double mhz) noexcept {
    frequency_mhz_ = mhz;
    frequency_err_ = validate_positive_bounded(mhz, 1.0, 100000.0);
    recompute(); fire();
}
void RadarPresenter::set_system_losses(double db) noexcept {
    system_losses_db_ = db;
    system_losses_err_ = validate_bounds(db, 0.0, 30.0);
    recompute(); fire();
}
void RadarPresenter::set_noise_figure(double db) noexcept {
    noise_figure_db_ = db;
    noise_figure_err_ = validate_bounds(db, 0.0, 30.0);
    recompute(); fire();
}
void RadarPresenter::set_bandwidth(double mhz) noexcept {
    bandwidth_mhz_ = mhz;
    bandwidth_err_ = validate_positive_bounded(mhz, 0.001, 10000.0);
    recompute(); fire();
}
void RadarPresenter::set_required_snr(double db) noexcept {
    required_snr_db_ = db;
    required_snr_err_ = validate_bounds(db, -10.0, 50.0);
    recompute(); fire();
}
void RadarPresenter::set_time_bandwidth_product(double tb) noexcept {
    time_bandwidth_product_ = tb;
    tb_product_err_ = validate_positive_bounded(tb, 1.0, 1.0e6);
    recompute(); fire();
}
void RadarPresenter::set_num_pulses(int n) noexcept {
    num_pulses_ = n;
    recompute(); fire();
}

void RadarPresenter::recompute() noexcept {
    output_.valid = (tx_power_err_     == FieldError::none &&
                     antenna_gain_err_ == FieldError::none &&
                     target_rcs_err_   == FieldError::none &&
                     frequency_err_    == FieldError::none &&
                     system_losses_err_== FieldError::none &&
                     noise_figure_err_ == FieldError::none &&
                     bandwidth_err_    == FieldError::none &&
                     required_snr_err_ == FieldError::none &&
                     tb_product_err_   == FieldError::none &&
                     num_pulses_ >= 1 && num_pulses_ <= 100000);

    if (!output_.valid) {
        output_.max_range_str              = DASH;
        output_.two_way_loss_str           = DASH;
        output_.pulse_compression_gain_str = DASH;
        output_.coherent_integration_gain_str = DASH;
        return;
    }

    using namespace libew::units;

    const auto res = libew::radar::radar_range(
        Dbm{tx_power_dbm_}, Db{antenna_gain_dbi_},
        Dbsm{target_rcs_dbsm_}, Mhz{frequency_mhz_},
        Db{system_losses_db_}, Db{noise_figure_db_},
        Mhz{bandwidth_mhz_}, Db{required_snr_db_});

    output_.max_range    = res.max_range;
    output_.two_way_loss = res.two_way_loss;

    output_.pulse_compression_gain   = libew::radar::pulse_compression_gain(time_bandwidth_product_);
    output_.coherent_integration_gain = libew::radar::coherent_integration_gain(num_pulses_);
    output_.lpi_advantage             = libew::radar::lpi_advantage(time_bandwidth_product_);

    output_.max_range_str              = format_km(output_.max_range);
    output_.two_way_loss_str           = format_db(output_.two_way_loss);
    output_.pulse_compression_gain_str = format_db(output_.pulse_compression_gain);
    output_.coherent_integration_gain_str = format_db(output_.coherent_integration_gain);
    output_.lpi_advantage_str          = format_db(output_.lpi_advantage);
}

void RadarPresenter::fire() noexcept {
    if (on_change_) on_change_(output_);
}

} // namespace ewpresenter
