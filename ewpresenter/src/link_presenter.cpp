#include "ewpresenter/link_presenter.h"
#include "ewpresenter/formatter.h"
#include "libew/link/link.h"

namespace ewpresenter {

static constexpr const char* DASH = "\xe2\x80\x94";

LinkPresenter::LinkPresenter() noexcept { recompute(); }

void LinkPresenter::set_tx_power(double dbm) noexcept {
    update_field(dbm, tx_power_dbm_, tx_power_err_, validate_bounds(dbm, -50.0, 200.0));
}
void LinkPresenter::set_tx_gain(double db) noexcept {
    update_field(db, tx_gain_db_, tx_gain_err_, validate_bounds(db, -30.0, 60.0));
}
void LinkPresenter::set_rx_gain(double db) noexcept {
    update_field(db, rx_gain_db_, rx_gain_err_, validate_bounds(db, -30.0, 60.0));
}
void LinkPresenter::set_distance(double km) noexcept {
    update_field(km, distance_km_, distance_err_, validate_positive_bounded(km, 0.01, 10000.0));
}
void LinkPresenter::set_tx_height(double meters) noexcept {
    update_field(meters, tx_height_m_, tx_height_err_, validate_positive_bounded(meters, 0.1, 100000.0));
}
void LinkPresenter::set_rx_height(double meters) noexcept {
    update_field(meters, rx_height_m_, rx_height_err_, validate_positive_bounded(meters, 0.1, 100000.0));
}
void LinkPresenter::set_frequency(double mhz) noexcept {
    update_field(mhz, frequency_mhz_, frequency_err_, validate_positive_bounded(mhz, 0.1, 100000.0));
}
void LinkPresenter::set_rx_sensitivity(double dbm) noexcept {
    update_field(dbm, rx_sensitivity_dbm_, rx_sensitivity_err_, validate_bounds(dbm, -200.0, 0.0));
}

void LinkPresenter::recompute() noexcept {
    output_.valid = (tx_power_err_       == FieldError::none &&
                     tx_gain_err_        == FieldError::none &&
                     rx_gain_err_        == FieldError::none &&
                     distance_err_       == FieldError::none &&
                     tx_height_err_      == FieldError::none &&
                     rx_height_err_      == FieldError::none &&
                     frequency_err_      == FieldError::none &&
                     rx_sensitivity_err_ == FieldError::none);

    if (!output_.valid) {
        output_.received_power_str = DASH;
        output_.path_loss_str      = DASH;
        output_.fresnel_zone_str   = DASH;
        output_.regime_str         = DASH;
        output_.effective_range_str= DASH;
        output_.range_regime_str   = DASH;
        output_.link_margin_str    = DASH;
        return;
    }

    using namespace libew::units;
    const auto res = libew::link::one_way_link(
        Dbm{tx_power_dbm_}, Db{tx_gain_db_}, Db{rx_gain_db_},
        Km{distance_km_}, Meters{tx_height_m_}, Meters{rx_height_m_},
        Mhz{frequency_mhz_});

    output_.received_power       = res.received_power;
    output_.path_loss            = res.path_loss;
    output_.fresnel_zone_distance= res.fresnel_zone_distance;
    output_.two_ray_regime       = res.two_ray_regime;

    const auto range_res = libew::link::effective_range(
        Dbm{tx_power_dbm_}, Db{tx_gain_db_}, Db{rx_gain_db_},
        Meters{tx_height_m_}, Meters{rx_height_m_},
        Mhz{frequency_mhz_}, Dbm{rx_sensitivity_dbm_});

    output_.effective_range          = range_res.range;
    output_.range_two_ray_regime     = range_res.two_ray_regime;

    const Db margin = res.received_power - Dbm{rx_sensitivity_dbm_};

    output_.received_power_str = format_dbm(output_.received_power);
    output_.path_loss_str      = format_db(output_.path_loss);
    output_.fresnel_zone_str   = format_km(output_.fresnel_zone_distance);
    output_.regime_str         = format_regime(output_.two_ray_regime);
    output_.effective_range_str= format_km(output_.effective_range);
    output_.range_regime_str   = format_regime(output_.range_two_ray_regime);
    output_.link_margin_str    = format_db(margin);
}

void LinkPresenter::fire() noexcept {
    if (on_change_) on_change_(output_);
}

} // namespace ewpresenter
