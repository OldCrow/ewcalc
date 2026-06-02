#pragma once

/// @file receiver_presenter.h
/// @brief Presenter for receiver sensitivity, cascaded noise figure, and dynamic range.

#include "ewpresenter/validation.h"
#include "libew/core/units.h"
#include "libew/receiver/receiver.h"
#include <functional>
#include <string>
#include <vector>

namespace ewpresenter {

using namespace libew::units;

class ReceiverPresenter {
public:
    struct Output {
        // Sensitivity section
        Dbm sensitivity{};
        std::string sensitivity_str;

        // Cascaded noise figure section
        Db  cascaded_nf{};
        std::string cascaded_nf_str;

        // Analog dynamic range
        Db  sfdr_second_order{};
        Db  sfdr_third_order{};
        std::string sfdr2_str;
        std::string sfdr3_str;

        // Digital dynamic range
        Db  digital_dr{};
        std::string digital_dr_str;

        // System noise temperature (from system NF input)
        Kelvin system_noise_temp{};
        std::string system_noise_temp_str;

        bool valid{false};
    };

    /// A single stage in the Friis noise chain.
    struct StageInput {
        double noise_figure_db{3.0};
        double gain_db{20.0};
    };

    ReceiverPresenter() noexcept;

    // -----------------------------------------------------------------------
    // Sensitivity inputs
    // -----------------------------------------------------------------------
    /// IF bandwidth (MHz). Range: 0.001 – 10 000 MHz.
    void set_bandwidth(double mhz) noexcept;

    /// System noise figure (dB). Range: 0 – 30 dB.
    void set_noise_figure(double db) noexcept;

    /// Required pre-detection SNR (dB). Range: -20 – 50 dB.
    void set_required_snr(double db) noexcept;

    // -----------------------------------------------------------------------
    // Noise chain inputs
    // -----------------------------------------------------------------------
    /// Replace the entire stage chain (Friis cascade order, front to back).
    void set_stages(std::vector<StageInput> stages) noexcept;

    /// Replace a single stage; index 0 = front-end.
    void set_stage(std::size_t index, StageInput stage) noexcept;

    // -----------------------------------------------------------------------
    // Dynamic range inputs
    // -----------------------------------------------------------------------
    /// Second-order input intercept point (dBm). Range: -50 – 100 dBm.
    void set_second_order_ip(double dbm) noexcept;

    /// Third-order input intercept point (dBm). Range: -50 – 100 dBm.
    void set_third_order_ip(double dbm) noexcept;

    /// ADC quantizer word length (bits). Range: 1 – 64.
    void set_adc_bits(int bits) noexcept;

    // -----------------------------------------------------------------------
    // Accessors
    // -----------------------------------------------------------------------
    [[nodiscard]] const Output& output() const noexcept { return output_; }
    [[nodiscard]] bool is_valid() const noexcept { return output_.valid; }

    [[nodiscard]] double bandwidth_mhz()      const noexcept { return bandwidth_mhz_; }
    [[nodiscard]] double noise_figure_db()     const noexcept { return noise_figure_db_; }
    [[nodiscard]] double required_snr_db()     const noexcept { return required_snr_db_; }
    [[nodiscard]] double second_order_ip_dbm() const noexcept { return second_order_ip_dbm_; }
    [[nodiscard]] double third_order_ip_dbm()  const noexcept { return third_order_ip_dbm_; }
    [[nodiscard]] int    adc_bits()            const noexcept { return adc_bits_; }
    [[nodiscard]] const std::vector<StageInput>& stages() const noexcept { return stages_; }

    [[nodiscard]] FieldError bandwidth_error()       const noexcept { return bandwidth_err_; }
    [[nodiscard]] FieldError noise_figure_error()    const noexcept { return noise_figure_err_; }
    [[nodiscard]] FieldError required_snr_error()    const noexcept { return required_snr_err_; }
    [[nodiscard]] FieldError second_order_ip_error() const noexcept { return second_order_ip_err_; }
    [[nodiscard]] FieldError third_order_ip_error()  const noexcept { return third_order_ip_err_; }

    void set_on_change(std::function<void(const Output&)> cb) noexcept {
        on_change_ = std::move(cb);
    }

private:
    double bandwidth_mhz_      {0.1};
    double noise_figure_db_    {6.5};
    double required_snr_db_    {15.0};
    double second_order_ip_dbm_{50.0};
    double third_order_ip_dbm_ {20.0};
    int    adc_bits_           {12};

    std::vector<StageInput> stages_{
        {1.0,  -1.0},   // feed loss
        {3.0,  20.0},   // preamp
        {10.0, -10.0},  // cable
        {11.0,  0.0}    // receiver
    };

    FieldError bandwidth_err_       {FieldError::none};
    FieldError noise_figure_err_    {FieldError::none};
    FieldError required_snr_err_    {FieldError::none};
    FieldError second_order_ip_err_ {FieldError::none};
    FieldError third_order_ip_err_  {FieldError::none};

    Output output_;
    std::function<void(const Output&)> on_change_;

    void recompute() noexcept;
    void fire() noexcept;
};

} // namespace ewpresenter
