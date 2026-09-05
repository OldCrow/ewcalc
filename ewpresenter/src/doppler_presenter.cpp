#include "ewpresenter/doppler_presenter.h"
#include "ewpresenter/formatter.h"
#include "libew/radar/radar.h"

namespace ewpresenter {

DopplerPresenter::DopplerPresenter() noexcept { recompute(); }

void DopplerPresenter::set_frequency(double mhz) noexcept {
    update_field(mhz, frequency_mhz_, frequency_err_, validate_positive_bounded(mhz, 1.0, 100000.0));
}
void DopplerPresenter::set_radial_speed(double m_s) noexcept {
    update_field(m_s, radial_speed_m_s_, radial_speed_err_, validate_bounds(m_s, -3000.0, 3000.0));
}
void DopplerPresenter::set_prf(double hz) noexcept {
    update_field(hz, prf_hz_, prf_err_, validate_positive_bounded(hz, 10.0, 1.0e6));
}
void DopplerPresenter::set_bandwidth(double mhz) noexcept {
    update_field(mhz, bandwidth_mhz_, bandwidth_err_, validate_positive_bounded(mhz, 0.001, 10000.0));
}
void DopplerPresenter::set_target_range(double km) noexcept {
    update_field(km, target_range_km_, target_range_err_, validate_positive_bounded(km, 0.1, 5000.0));
}
void DopplerPresenter::set_beamwidth_az(double deg) noexcept {
    update_field(deg, beamwidth_az_deg_, beamwidth_az_err_, validate_positive_bounded(deg, 0.1, 45.0));
}
void DopplerPresenter::set_beamwidth_el(double deg) noexcept {
    update_field(deg, beamwidth_el_deg_, beamwidth_el_err_, validate_positive_bounded(deg, 0.1, 45.0));
}

void DopplerPresenter::recompute() noexcept {
    output_.valid = (frequency_err_    == FieldError::none &&
                     radial_speed_err_ == FieldError::none &&
                     prf_err_          == FieldError::none &&
                     bandwidth_err_    == FieldError::none &&
                     target_range_err_ == FieldError::none &&
                     beamwidth_az_err_ == FieldError::none &&
                     beamwidth_el_err_ == FieldError::none);

    if (!output_.valid) {
        output_.doppler_shift_str        = kDash;
        output_.unambiguous_range_str    = kDash;
        output_.blind_speed_str          = kDash;
        output_.unambiguous_velocity_str = kDash;
        output_.range_resolution_str     = kDash;
        output_.cross_range_az_str       = kDash;
        output_.cross_range_el_str       = kDash;
        return;
    }

    using namespace libew::units;
    namespace rad = libew::radar;

    const Mhz f{frequency_mhz_};

    output_.doppler_shift_hz = rad::doppler_shift_hz(f, radial_speed_m_s_);
    output_.unambiguous_range        = rad::unambiguous_range(prf_hz_);
    output_.blind_speed_m_s          = rad::blind_speed_m_s(f, prf_hz_);
    output_.unambiguous_velocity_m_s = rad::unambiguous_velocity_m_s(f, prf_hz_);

    output_.range_resolution = rad::range_resolution(Mhz{bandwidth_mhz_});
    output_.cross_range_az   = rad::cross_range_resolution(Km{target_range_km_},
                                                           Degrees{beamwidth_az_deg_});
    output_.cross_range_el   = rad::cross_range_resolution(Km{target_range_km_},
                                                           Degrees{beamwidth_el_deg_});

    output_.doppler_shift_str        = format_hz(output_.doppler_shift_hz);
    output_.unambiguous_range_str    = format_km(output_.unambiguous_range);
    output_.blind_speed_str          = format_mps(output_.blind_speed_m_s);
    output_.unambiguous_velocity_str = format_mps(output_.unambiguous_velocity_m_s);
    output_.range_resolution_str     = format_m(output_.range_resolution);
    output_.cross_range_az_str       = format_m(output_.cross_range_az);
    output_.cross_range_el_str       = format_m(output_.cross_range_el);
}

void DopplerPresenter::fire() noexcept {
    if (on_change_) on_change_(output_);
}

} // namespace ewpresenter
