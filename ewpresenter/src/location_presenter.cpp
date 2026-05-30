#include "ewpresenter/location_presenter.h"
#include "ewpresenter/formatter.h"
#include "libew/location/location.h"

namespace ewpresenter {

static constexpr const char* DASH = "\xe2\x80\x94";

LocationPresenter::LocationPresenter() noexcept { recompute(); }

void LocationPresenter::set_rms_bearing_error(double degrees) noexcept {
    rms_bearing_error_deg_ = degrees;
    rms_bearing_err_ = validate_positive_bounded(degrees, 0.01, 45.0);
    recompute(); fire();
}
void LocationPresenter::set_aoa_range(double km) noexcept {
    aoa_range_km_ = km;
    aoa_range_err_ = validate_positive_bounded(km, 0.1, 10000.0);
    recompute(); fire();
}
void LocationPresenter::set_semi_major(double km) noexcept {
    semi_major_km_ = km;
    semi_major_err_ = validate_positive_bounded(km, 0.001, 1000.0);
    recompute(); fire();
}
void LocationPresenter::set_semi_minor(double km) noexcept {
    semi_minor_km_ = km;
    semi_minor_err_ = validate_positive_bounded(km, 0.001, 1000.0);
    recompute(); fire();
}

void LocationPresenter::recompute() noexcept {
    const bool aoa_valid = (rms_bearing_err_ == FieldError::none &&
                            aoa_range_err_   == FieldError::none);
    const bool eep_valid = (semi_major_err_ == FieldError::none &&
                            semi_minor_err_ == FieldError::none &&
                            semi_major_km_ >= semi_minor_km_);

    output_.valid = aoa_valid && eep_valid;

    using namespace libew::units;

    if (aoa_valid) {
        output_.cep_aoa = libew::location::cep_from_rms_bearing_error(
            Degrees{rms_bearing_error_deg_}, Km{aoa_range_km_});
        output_.cep_aoa_str = format_km(output_.cep_aoa);
    } else {
        output_.cep_aoa_str = DASH;
    }

    if (eep_valid) {
        output_.cep_eep = libew::location::cep_from_eep(
            Km{semi_major_km_}, Km{semi_minor_km_});
        output_.cep_eep_str = format_km(output_.cep_eep);
    } else {
        output_.cep_eep_str = DASH;
    }
}

void LocationPresenter::fire() noexcept {
    if (on_change_) on_change_(output_);
}

} // namespace ewpresenter
