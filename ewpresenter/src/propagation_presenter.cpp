#include "ewpresenter/propagation_presenter.h"
#include "ewpresenter/formatter.h"
#include "libew/propagation/propagation.h"

namespace ewpresenter {

PropagationPresenter::PropagationPresenter() noexcept {
    recompute();
}

void PropagationPresenter::set_distance(double km) noexcept {
    distance_km_  = km;
    distance_err_ = validate_positive_bounded(km, 0.01, 10000.0);
    recompute(); fire();
}

void PropagationPresenter::set_frequency(double mhz) noexcept {
    frequency_mhz_  = mhz;
    frequency_err_  = validate_positive_bounded(mhz, 0.1, 100000.0);
    recompute(); fire();
}

void PropagationPresenter::set_tx_height(double meters) noexcept {
    tx_height_m_  = meters;
    tx_height_err_ = validate_positive_bounded(meters, 0.1, 100000.0);
    recompute(); fire();
}

void PropagationPresenter::set_rx_height(double meters) noexcept {
    rx_height_m_  = meters;
    rx_height_err_ = validate_positive_bounded(meters, 0.1, 100000.0);
    recompute(); fire();
}

void PropagationPresenter::set_obstruction_height(double meters) noexcept {
    obstruction_height_m_   = meters;
    obstruction_height_err_ = validate_non_negative(meters);
    recompute(); fire();
}

void PropagationPresenter::recompute() noexcept {
    output_.valid = (distance_err_          == FieldError::none &&
                     frequency_err_         == FieldError::none &&
                     tx_height_err_         == FieldError::none &&
                     rx_height_err_         == FieldError::none &&
                     obstruction_height_err_== FieldError::none);

    if (!output_.valid) {
        output_.fspl_str          = "\xe2\x80\x94"; // em dash
        output_.two_ray_loss_str  = "\xe2\x80\x94";
        output_.fresnel_zone_str  = "\xe2\x80\x94";
        output_.path_loss_str     = "\xe2\x80\x94";
        output_.regime_str        = "\xe2\x80\x94";
        output_.earth_bulge_str      = "\xe2\x80\x94";
        output_.horizon_range_str    = "\xe2\x80\x94";
        output_.diffraction_loss_str = "\xe2\x80\x94";
        return;
    }

    using namespace libew::units;
    const Km     d{distance_km_};
    const Mhz    f{frequency_mhz_};
    const Meters ht{tx_height_m_};
    const Meters hr{rx_height_m_};

    output_.fspl               = libew::propagation::free_space_path_loss(d, f);
    output_.two_ray_loss       = libew::propagation::two_ray_path_loss(d, ht, hr);
    output_.fresnel_zone_distance = libew::propagation::fresnel_zone_distance(ht, hr, f);
    output_.two_ray_regime     = libew::propagation::is_two_ray_regime(d, ht, hr, f);
    output_.path_loss          = output_.two_ray_regime ? output_.two_ray_loss : output_.fspl;

    output_.fspl_str          = format_db(output_.fspl);
    output_.two_ray_loss_str  = format_db(output_.two_ray_loss);
    output_.fresnel_zone_str  = format_km(output_.fresnel_zone_distance);
    output_.path_loss_str     = format_db(output_.path_loss);
    output_.regime_str        = format_regime(output_.two_ray_regime);

    // Earth geometry — earth bulge at path midpoint; horizon from antenna heights
    const Km half_d{distance_km_ / 2.0};
    output_.earth_bulge   = libew::propagation::earth_bulge(half_d, half_d);
    output_.horizon_range = libew::propagation::radar_horizon_range(ht, hr);

    output_.earth_bulge_str   = format_m(output_.earth_bulge, 1);
    output_.horizon_range_str = format_km(output_.horizon_range);

    // Knife-edge diffraction — obstacle at path midpoint
    const double los_mid  = (tx_height_m_ + rx_height_m_) / 2.0;
    const double clearance = los_mid - output_.earth_bulge.value - obstruction_height_m_;
    output_.diffraction_loss = libew::propagation::knife_edge_diffraction_loss(
        half_d, half_d, Meters{clearance}, f);
    output_.diffraction_loss_str = format_db(output_.diffraction_loss);
}

void PropagationPresenter::fire() noexcept {
    if (on_change_) on_change_(output_);
}

} // namespace ewpresenter
