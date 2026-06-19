#include "ewpresenter/radar_presenter.h"
#include "ewpresenter/formatter.h"
#include "libew/radar/radar.h"

namespace ewpresenter {

RadarPresenter::RadarPresenter() noexcept { recompute(); }

void RadarPresenter::set_tx_power(double dbm) noexcept {
    update_field(dbm, tx_power_dbm_, tx_power_err_, validate_bounds(dbm, -50.0, 200.0));
}
void RadarPresenter::set_antenna_gain(double dbi) noexcept {
    update_field(dbi, antenna_gain_dbi_, antenna_gain_err_, validate_bounds(dbi, -30.0, 60.0));
}
void RadarPresenter::set_target_rcs(double dbsm) noexcept {
    update_field(dbsm, target_rcs_dbsm_, target_rcs_err_, validate_bounds(dbsm, -40.0, 60.0));
}
void RadarPresenter::set_frequency(double mhz) noexcept {
    update_field(mhz, frequency_mhz_, frequency_err_, validate_positive_bounded(mhz, 1.0, 100000.0));
}
void RadarPresenter::set_system_losses(double db) noexcept {
    update_field(db, system_losses_db_, system_losses_err_, validate_bounds(db, 0.0, 30.0));
}
void RadarPresenter::set_noise_figure(double db) noexcept {
    update_field(db, noise_figure_db_, noise_figure_err_, validate_bounds(db, 0.0, 30.0));
}
void RadarPresenter::set_bandwidth(double mhz) noexcept {
    update_field(mhz, bandwidth_mhz_, bandwidth_err_, validate_positive_bounded(mhz, 0.001, 10000.0));
}
void RadarPresenter::set_required_snr(double db) noexcept {
    update_field(db, required_snr_db_, required_snr_err_, validate_bounds(db, -10.0, 50.0));
}
void RadarPresenter::set_time_bandwidth_product(double tb) noexcept {
    update_field(tb, time_bandwidth_product_, tb_product_err_, validate_positive_bounded(tb, 1.0, 1.0e6));
}
void RadarPresenter::set_num_pulses(int n) noexcept {
    num_pulses_ = n;
    num_pulses_err_ = validate_bounds(static_cast<double>(n), 1.0, 100000.0);
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
                     num_pulses_err_   == FieldError::none);

    if (!output_.valid) {
        output_.max_range_str                 = kDash;
        output_.two_way_loss_str              = kDash;
        output_.pulse_compression_gain_str    = kDash;
        output_.coherent_integration_gain_str = kDash;
        output_.lpi_advantage_str             = kDash;
        output_.target_rcs_str                = kDash;
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

    output_.target_rcs                    = Dbsm{target_rcs_dbsm_};
    output_.max_range_str                 = format_km(output_.max_range);
    output_.two_way_loss_str              = format_db(output_.two_way_loss);
    output_.pulse_compression_gain_str    = format_db(output_.pulse_compression_gain);
    output_.coherent_integration_gain_str = format_db(output_.coherent_integration_gain);
    output_.lpi_advantage_str             = format_db(output_.lpi_advantage);
    output_.target_rcs_str                = format_dbsm(output_.target_rcs);
}

void RadarPresenter::fire() noexcept {
    if (on_change_) on_change_(output_);
}

} // namespace ewpresenter
