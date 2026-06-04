#include "ewpresenter/antenna_presenter.h"
#include "ewpresenter/formatter.h"
#include "libew/antenna/antenna.h"

namespace ewpresenter {

static constexpr const char* DASH = "\xe2\x80\x94";

AntennaPresenter::AntennaPresenter() noexcept { recompute(); }

void AntennaPresenter::set_gain(double dbi) noexcept {
    gain_dbi_  = dbi;
    gain_err_  = validate_bounds(dbi, -10.0, 60.0);
    recompute(); fire();
}
void AntennaPresenter::set_az_beamwidth(double deg) noexcept {
    az_beamwidth_deg_ = deg;
    az_bw_err_        = validate_positive_bounded(deg, 0.1, 360.0);
    recompute(); fire();
}
void AntennaPresenter::set_el_beamwidth(double deg) noexcept {
    el_beamwidth_deg_ = deg;
    el_bw_err_        = validate_positive_bounded(deg, 0.1, 360.0);
    recompute(); fire();
}
void AntennaPresenter::set_tx_power(double dbm) noexcept {
    tx_power_dbm_ = dbm;
    tx_power_err_ = validate_bounds(dbm, -30.0, 100.0);
    recompute(); fire();
}
void AntennaPresenter::set_frequency(double mhz) noexcept {
    frequency_mhz_ = mhz;
    frequency_err_ = validate_positive_bounded(mhz, 0.1, 100000.0);
    recompute(); fire();
}

void AntennaPresenter::recompute() noexcept {
    output_.valid = (gain_err_      == FieldError::none &&
                     az_bw_err_     == FieldError::none &&
                     el_bw_err_     == FieldError::none &&
                     tx_power_err_  == FieldError::none &&
                     frequency_err_ == FieldError::none);

    if (!output_.valid) {
        output_.erp_str                 = DASH;
        output_.beamwidth_from_gain_str = DASH;
        output_.gain_from_beamwidth_str = DASH;
        output_.wavelength_str          = DASH;
        return;
    }

    using namespace libew::units;

    output_.erp = libew::antenna::erp(Dbm{tx_power_dbm_}, Db{gain_dbi_});
    output_.beamwidth_from_gain = libew::antenna::beamwidth_from_gain(Db{gain_dbi_});
    output_.gain_from_beamwidth = libew::antenna::gain_from_beamwidth(
        Degrees{az_beamwidth_deg_}, Degrees{el_beamwidth_deg_});
    output_.wavelength = libew::antenna::wavelength(Mhz{frequency_mhz_});

    output_.erp_str                 = format_dbm(output_.erp);
    output_.beamwidth_from_gain_str = format_degrees(output_.beamwidth_from_gain);
    output_.gain_from_beamwidth_str = format_db(output_.gain_from_beamwidth);
    output_.wavelength_str          = format_m(output_.wavelength, 2);
}

void AntennaPresenter::fire() noexcept {
    if (on_change_) on_change_(output_);
}

} // namespace ewpresenter
