#pragma once

/// @file jamming_presenter.h
/// @brief Presenter for communications jamming J/S, burnthrough, and partial-band analysis.

#include "ewpresenter/validation.h"
#include "libew/core/units.h"
#include <functional>
#include <string>

namespace ewpresenter {

using namespace libew::units;

class JammingPresenter {
public:
    struct Output {
        // J/S ratio
        Db  js_ratio{};
        Dbm signal_at_rx{};
        Dbm jammer_at_rx{};
        std::string js_ratio_str;
        std::string signal_at_rx_str;
        std::string jammer_at_rx_str;

        // Partial-band jamming
        Mhz    optimum_bw{};
        double duty_cycle{};
        std::string optimum_bw_str;
        std::string duty_cycle_str;

        bool valid{false};
    };

    JammingPresenter() noexcept;

    // -----------------------------------------------------------------------
    // J/S inputs
    // -----------------------------------------------------------------------
    /// ERP of desired signal transmitter toward receiver (dBm). Range: -100 – 200 dBm.
    void set_signal_erp(double dbm) noexcept;

    /// ERP of jammer toward receiver (dBm). Range: -100 – 200 dBm.
    void set_jammer_erp(double dbm) noexcept;

    /// Distance from signal transmitter to receiver (km). Range: 0.01 – 10 000 km.
    void set_signal_to_rx_dist(double km) noexcept;

    /// Distance from jammer to receiver (km). Range: 0.01 – 10 000 km.
    void set_jammer_to_rx_dist(double km) noexcept;

    /// Signal transmitter antenna height (m). Range: 0.1 – 100 000 m.
    void set_signal_tx_height(double meters) noexcept;

    /// Jammer antenna height (m). Range: 0.1 – 100 000 m.
    void set_jammer_height(double meters) noexcept;

    /// Receiver antenna height (m). Range: 0.1 – 100 000 m.
    void set_rx_height(double meters) noexcept;

    /// Carrier frequency (MHz). Range: 0.1 – 100 000 MHz.
    void set_frequency(double mhz) noexcept;

    /// Receiver antenna gain toward signal (dB). Range: -30 – 60 dB.
    void set_rx_gain_signal(double db) noexcept;

    /// Receiver antenna gain toward jammer (dB). Range: -30 – 60 dB.
    void set_rx_gain_jammer(double db) noexcept;

    // -----------------------------------------------------------------------
    // Partial-band inputs
    // -----------------------------------------------------------------------
    /// Target signal bandwidth (MHz). Range: 0.001 – 1000 MHz.
    void set_signal_bandwidth(double mhz) noexcept;

    /// Frequency hopping range (MHz). Range: 0.001 – 10 000 MHz.
    void set_hop_range(double mhz) noexcept;

    /// Single-channel J/S (dB) — feeds the partial-band calculation.
    /// Automatically updated from the J/S computation; can also be set manually.
    void set_single_channel_js(double db) noexcept;

    // -----------------------------------------------------------------------
    // Accessors
    // -----------------------------------------------------------------------
    [[nodiscard]] const Output& output() const noexcept { return output_; }
    [[nodiscard]] bool is_valid() const noexcept { return output_.valid; }

    [[nodiscard]] double signal_erp_dbm()       const noexcept { return signal_erp_dbm_; }
    [[nodiscard]] double jammer_erp_dbm()       const noexcept { return jammer_erp_dbm_; }
    [[nodiscard]] double signal_to_rx_dist_km() const noexcept { return signal_to_rx_dist_km_; }
    [[nodiscard]] double jammer_to_rx_dist_km() const noexcept { return jammer_to_rx_dist_km_; }
    [[nodiscard]] double signal_tx_height_m()   const noexcept { return signal_tx_height_m_; }
    [[nodiscard]] double jammer_height_m()      const noexcept { return jammer_height_m_; }
    [[nodiscard]] double rx_height_m()          const noexcept { return rx_height_m_; }
    [[nodiscard]] double frequency_mhz()        const noexcept { return frequency_mhz_; }
    [[nodiscard]] double signal_bandwidth_mhz() const noexcept { return signal_bandwidth_mhz_; }
    [[nodiscard]] double hop_range_mhz()        const noexcept { return hop_range_mhz_; }

    void set_on_change(std::function<void(const Output&)> cb) noexcept {
        on_change_ = std::move(cb);
    }

private:
    double signal_erp_dbm_       {30.0};
    double jammer_erp_dbm_       {50.0};
    double signal_to_rx_dist_km_ {5.0};
    double jammer_to_rx_dist_km_ {50.0};
    double signal_tx_height_m_   {1.5};
    double jammer_height_m_      {1000.0};
    double rx_height_m_          {1.5};
    double frequency_mhz_        {100.0};
    double rx_gain_signal_db_    {0.0};
    double rx_gain_jammer_db_    {0.0};
    double signal_bandwidth_mhz_ {0.025};
    double hop_range_mhz_        {58.0};
    double single_channel_js_db_ {0.0};

    FieldError signal_erp_err_       {FieldError::none};
    FieldError jammer_erp_err_       {FieldError::none};
    FieldError signal_to_rx_err_     {FieldError::none};
    FieldError jammer_to_rx_err_     {FieldError::none};
    FieldError signal_height_err_    {FieldError::none};
    FieldError jammer_height_err_    {FieldError::none};
    FieldError rx_height_err_        {FieldError::none};
    FieldError frequency_err_        {FieldError::none};
    FieldError signal_bandwidth_err_ {FieldError::none};
    FieldError hop_range_err_        {FieldError::none};

    Output output_;
    std::function<void(const Output&)> on_change_;

    void recompute() noexcept;
    void fire() noexcept;
};

} // namespace ewpresenter
