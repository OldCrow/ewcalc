#include "ewpresenter/detection_presenter.h"
#include "ewpresenter/formatter.h"
#include "libew/radar/radar.h"
#include <cmath>

namespace ewpresenter {

DetectionPresenter::DetectionPresenter() noexcept { recompute(); }

void DetectionPresenter::set_pd(double pd) noexcept {
    update_field(pd, pd_, pd_err_, validate_bounds(pd, 0.1, 0.99));
}
void DetectionPresenter::set_pfa_exponent(double x) noexcept {
    update_field(x, pfa_exponent_, pfa_exponent_err_, validate_bounds(x, -9.0, -3.0));
}
void DetectionPresenter::set_num_pulses(int n) noexcept {
    num_pulses_ = n;
    num_pulses_err_ = validate_bounds(static_cast<double>(n), 1.0, 100.0);
    recompute(); fire();
}
void DetectionPresenter::set_swerling_case(int c) noexcept {
    swerling_case_ = c;
    swerling_case_err_ = validate_bounds(static_cast<double>(c), 0.0, 4.0);
    recompute(); fire();
}
void DetectionPresenter::set_beamwidth(double deg) noexcept {
    update_field(deg, beamwidth_deg_, beamwidth_err_, validate_positive_bounded(deg, 0.1, 45.0));
}
void DetectionPresenter::set_scan_rate(double deg_s) noexcept {
    update_field(deg_s, scan_rate_deg_s_, scan_rate_err_, validate_positive_bounded(deg_s, 1.0, 720.0));
}
void DetectionPresenter::set_prf(double hz) noexcept {
    update_field(hz, prf_hz_, prf_err_, validate_positive_bounded(hz, 10.0, 1.0e6));
}
void DetectionPresenter::set_bandwidth(double mhz) noexcept {
    update_field(mhz, bandwidth_mhz_, bandwidth_err_, validate_positive_bounded(mhz, 0.001, 10000.0));
}

void DetectionPresenter::recompute() noexcept {
    output_.valid = (pd_err_            == FieldError::none &&
                     pfa_exponent_err_  == FieldError::none &&
                     num_pulses_err_    == FieldError::none &&
                     swerling_case_err_ == FieldError::none &&
                     beamwidth_err_     == FieldError::none &&
                     scan_rate_err_     == FieldError::none &&
                     prf_err_           == FieldError::none &&
                     bandwidth_err_     == FieldError::none);

    if (!output_.valid) {
        output_.required_snr_str            = kDash;
        output_.required_snr_albersheim_str = kDash;
        output_.fluctuation_loss_str        = kDash;
        output_.dwell_time_str              = kDash;
        output_.hits_per_scan_str           = kDash;
        output_.far_str                     = kDash;
        return;
    }

    using namespace libew::units;
    namespace rad = libew::radar;

    const double pfa = std::pow(10.0, pfa_exponent_);
    const auto swerling = static_cast<rad::Swerling>(swerling_case_);

    output_.required_snr =
        rad::required_snr_shnidman(pd_, pfa, num_pulses_, swerling);
    output_.required_snr_albersheim =
        rad::required_snr_albersheim(pd_, pfa, num_pulses_);
    output_.fluctuation_loss =
        rad::fluctuation_loss(pd_, pfa, num_pulses_, swerling);

    output_.dwell_time    = rad::dwell_time(Degrees{beamwidth_deg_}, scan_rate_deg_s_);
    output_.hits_per_scan = rad::hits_per_scan(output_.dwell_time, prf_hz_);
    output_.far_hz        = rad::false_alarm_rate_hz(pfa, Mhz{bandwidth_mhz_});

    output_.required_snr_str            = format_db(output_.required_snr);
    output_.required_snr_albersheim_str = format_db(output_.required_snr_albersheim);
    output_.fluctuation_loss_str        = format_db(output_.fluctuation_loss);
    output_.dwell_time_str              = format_seconds(output_.dwell_time);
    output_.hits_per_scan_str           = format_count(output_.hits_per_scan);
    output_.far_str                     = format_hz(output_.far_hz);
}

void DetectionPresenter::fire() noexcept {
    if (on_change_) on_change_(output_);
}

} // namespace ewpresenter
