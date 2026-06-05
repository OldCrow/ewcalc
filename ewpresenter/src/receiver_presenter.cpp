#include "ewpresenter/receiver_presenter.h"
#include "ewpresenter/formatter.h"
#include "libew/receiver/receiver.h"
#include "ewpresenter/validation.h"

namespace ewpresenter {

static constexpr const char* DASH = "\xe2\x80\x94";

ReceiverPresenter::ReceiverPresenter() noexcept { recompute(); }

void ReceiverPresenter::set_bandwidth(double mhz) noexcept {
    bandwidth_mhz_ = mhz;
    bandwidth_err_ = validate_positive_bounded(mhz, 0.001, 10000.0);
    recompute(); fire();
}
void ReceiverPresenter::set_noise_figure(double db) noexcept {
    noise_figure_db_ = db;
    noise_figure_err_ = validate_bounds(db, 0.0, 30.0);
    recompute(); fire();
}
void ReceiverPresenter::set_required_snr(double db) noexcept {
    required_snr_db_ = db;
    required_snr_err_ = validate_bounds(db, -20.0, 50.0);
    recompute(); fire();
}
void ReceiverPresenter::set_stages(std::vector<StageInput> stages) noexcept {
    stages_ = std::move(stages);
    recompute(); fire();
}
void ReceiverPresenter::set_stage(std::size_t index, StageInput stage) noexcept {
    if (index < stages_.size()) {
        stages_[index] = stage;
        recompute(); fire();
    }
}
void ReceiverPresenter::set_second_order_ip(double dbm) noexcept {
    second_order_ip_dbm_ = dbm;
    second_order_ip_err_ = validate_bounds(dbm, -50.0, 100.0);
    recompute(); fire();
}
void ReceiverPresenter::set_third_order_ip(double dbm) noexcept {
    third_order_ip_dbm_ = dbm;
    third_order_ip_err_ = validate_bounds(dbm, -50.0, 100.0);
    recompute(); fire();
}
void ReceiverPresenter::set_adc_bits(int bits) noexcept {
    adc_bits_ = bits;
    recompute(); fire();
}

void ReceiverPresenter::recompute() noexcept {
    output_.valid = (bandwidth_err_       == FieldError::none &&
                     noise_figure_err_    == FieldError::none &&
                     required_snr_err_    == FieldError::none &&
                     second_order_ip_err_ == FieldError::none &&
                     third_order_ip_err_  == FieldError::none &&
                     adc_bits_ >= 1 && adc_bits_ <= 64);

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
        for (const auto& s : stages_) {
            chain.push_back({Db{s.noise_figure_db}, Db{s.gain_db}});
        }
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
