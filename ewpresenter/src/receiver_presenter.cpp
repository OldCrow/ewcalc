#include "ewpresenter/receiver_presenter.h"
#include "ewpresenter/formatter.h"
#include "libew/receiver/receiver.h"
#include "ewpresenter/validation.h"
#include <algorithm>

namespace ewpresenter {

static constexpr const char* DASH = "\xe2\x80\x94";

ReceiverPresenter::ReceiverPresenter() noexcept { recompute(); }

void ReceiverPresenter::set_bandwidth(double mhz) noexcept {
    update_field(mhz, bandwidth_mhz_, bandwidth_err_, validate_positive_bounded(mhz, 0.001, 10000.0));
}
void ReceiverPresenter::set_noise_figure(double db) noexcept {
    update_field(db, noise_figure_db_, noise_figure_err_, validate_bounds(db, 0.0, 30.0));
}
void ReceiverPresenter::set_required_snr(double db) noexcept {
    update_field(db, required_snr_db_, required_snr_err_, validate_bounds(db, -20.0, 50.0));
}
void ReceiverPresenter::set_stages(std::vector<StageInput> new_stages) noexcept {
    stages_ = std::move(new_stages);
    stage_nf_err_ = std::any_of(stages_.cbegin(), stages_.cend(),
        [](const StageInput& s) { return s.noise_figure_db < 0.0; })
        ? FieldError::invalid_negative : FieldError::none;
    recompute(); fire();
}
bool ReceiverPresenter::try_set_stage(std::size_t index, StageInput stage) noexcept {
    if (index >= stages_.size()) return false;
    stages_[index] = stage;
    stage_nf_err_ = std::any_of(stages_.cbegin(), stages_.cend(),
        [](const StageInput& s) { return s.noise_figure_db < 0.0; })
        ? FieldError::invalid_negative : FieldError::none;
    recompute(); fire();
    return true;
}
void ReceiverPresenter::set_stage(std::size_t index, StageInput stage) noexcept {
    [[maybe_unused]] bool ok = try_set_stage(index, stage);
}
void ReceiverPresenter::set_second_order_ip(double dbm) noexcept {
    update_field(dbm, second_order_ip_dbm_, second_order_ip_err_, validate_bounds(dbm, -50.0, 100.0));
}
void ReceiverPresenter::set_third_order_ip(double dbm) noexcept {
    update_field(dbm, third_order_ip_dbm_, third_order_ip_err_, validate_bounds(dbm, -50.0, 100.0));
}
void ReceiverPresenter::set_adc_bits(int bits) noexcept {
    adc_bits_ = bits;
    adc_bits_err_ = validate_bounds(static_cast<double>(bits), 1.0, 64.0);
    recompute(); fire();
}

void ReceiverPresenter::recompute() noexcept {
    output_.valid = (bandwidth_err_       == FieldError::none &&
                     noise_figure_err_    == FieldError::none &&
                     required_snr_err_    == FieldError::none &&
                     second_order_ip_err_ == FieldError::none &&
                     third_order_ip_err_  == FieldError::none &&
                     adc_bits_err_        == FieldError::none &&
                     stage_nf_err_        == FieldError::none);

    if (!output_.valid) {
        output_.sensitivity_str       = DASH;
        output_.cascaded_nf_str       = DASH;
        output_.sfdr2_str             = DASH;
        output_.sfdr3_str             = DASH;
        output_.digital_dr_str        = DASH;
        output_.system_noise_temp_str = DASH;
        output_.system_nf_str          = DASH;
        return;
    }

    using namespace libew::units;
    using namespace libew::receiver;

    // Sensitivity
    output_.sensitivity = system_sensitivity(
        Mhz{bandwidth_mhz_}, Db{noise_figure_db_}, Db{required_snr_db_});

    // Cascaded NF from stage chain
    if (!stages_.empty()) {
        std::vector<Stage> chain;
        chain.reserve(stages_.size());
        std::transform(stages_.cbegin(), stages_.cend(), std::back_inserter(chain),
            [](const StageInput& s) -> Stage { return {Db{s.noise_figure_db}, Db{s.gain_db}}; });
        output_.cascaded_nf = cascaded_noise_figure(std::span<const Stage>{chain});
    } else {
        output_.cascaded_nf = Db{noise_figure_db_};
    }

    // SFDR
    const Dbm sens = output_.sensitivity;
    output_.sfdr_second_order = analog_sfdr_second_order(sens, Dbm{second_order_ip_dbm_});
    output_.sfdr_third_order  = analog_sfdr_third_order(sens,  Dbm{third_order_ip_dbm_});

    // Digital DR
    output_.digital_dr = digital_dynamic_range(adc_bits_);

    // System noise temperature and equivalent NF (both derived from system NF input)
    output_.system_noise_temp = libew::receiver::noise_temp_from_nf(Db{noise_figure_db_});
    output_.system_nf         = libew::receiver::nf_from_noise_temp(output_.system_noise_temp);

    output_.sensitivity_str       = format_dbm(output_.sensitivity);
    output_.cascaded_nf_str       = format_db(output_.cascaded_nf);
    output_.sfdr2_str             = format_db(output_.sfdr_second_order);
    output_.sfdr3_str             = format_db(output_.sfdr_third_order);
    output_.digital_dr_str        = format_db(output_.digital_dr);
    output_.system_noise_temp_str = format_kelvin(output_.system_noise_temp);
    output_.system_nf_str         = format_db(output_.system_nf);
}

void ReceiverPresenter::fire() noexcept {
    if (on_change_) on_change_(output_);
}

} // namespace ewpresenter
