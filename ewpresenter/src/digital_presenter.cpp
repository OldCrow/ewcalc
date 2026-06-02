#include "ewpresenter/digital_presenter.h"
#include "ewpresenter/formatter.h"
#include "libew/digital/digital.h"

namespace ewpresenter {

static constexpr const char* DASH = "\xe2\x80\x94";

DigitalPresenter::DigitalPresenter() noexcept { recompute(); }

void DigitalPresenter::set_snr(double db) noexcept {
    snr_db_  = db;
    snr_err_ = validate_bounds(db, -30.0, 60.0);
    recompute(); fire();
}
void DigitalPresenter::set_bandwidth(double mhz) noexcept {
    bandwidth_mhz_ = mhz;
    bandwidth_err_ = validate_positive_bounded(mhz, 0.001, 10000.0);
    recompute(); fire();
}
void DigitalPresenter::set_data_rate(double mbps) noexcept {
    data_rate_mhz_ = mbps;
    data_rate_err_ = validate_positive_bounded(mbps, 0.0001, 10000.0);
    recompute(); fire();
}
void DigitalPresenter::set_chip_rate(double mcps) noexcept {
    chip_rate_mhz_ = mcps;
    chip_rate_err_ = validate_positive_bounded(mcps, 0.0001, 10000.0);
    recompute(); fire();
}
void DigitalPresenter::set_required_eb_no(double db) noexcept {
    required_eb_no_db_  = db;
    required_eb_no_err_ = validate_bounds(db, -10.0, 30.0);
    recompute(); fire();
}
void DigitalPresenter::set_implementation_loss(double db) noexcept {
    implementation_loss_db_ = db;
    impl_loss_err_ = validate_bounds(db, 0.0, 10.0);
    recompute(); fire();
}

void DigitalPresenter::recompute() noexcept {
    output_.valid = (snr_err_            == FieldError::none &&
                     bandwidth_err_      == FieldError::none &&
                     data_rate_err_      == FieldError::none &&
                     chip_rate_err_      == FieldError::none &&
                     required_eb_no_err_ == FieldError::none &&
                     impl_loss_err_      == FieldError::none);

    if (!output_.valid) {
        output_.eb_no_str          = DASH;
        output_.process_gain_str   = DASH;
        output_.jamming_margin_str = DASH;
        output_.required_js_str    = DASH;
        return;
    }

    using namespace libew::units;

    output_.eb_no = libew::digital::eb_no_from_snr(
        Db{snr_db_}, Mhz{bandwidth_mhz_}, Mhz{data_rate_mhz_});

    output_.process_gain = libew::digital::dsss_process_gain(
        Mhz{chip_rate_mhz_}, Mhz{data_rate_mhz_});

    output_.jamming_margin = libew::digital::dsss_jamming_margin(
        output_.process_gain,
        Db{required_eb_no_db_},
        Db{implementation_loss_db_});

    output_.required_js = libew::digital::dsss_required_js(
        output_.process_gain,
        Db{required_eb_no_db_},
        Db{implementation_loss_db_});

    output_.eb_no_str          = format_db(output_.eb_no);
    output_.process_gain_str   = format_db(output_.process_gain);
    output_.jamming_margin_str = format_db(output_.jamming_margin);
    output_.required_js_str    = format_db(output_.required_js);
}

void DigitalPresenter::fire() noexcept {
    if (on_change_) on_change_(output_);
}

} // namespace ewpresenter
