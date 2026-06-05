#pragma once

/// @file radar_presenter.h
/// @brief Presenter for radar range equation and signal processing gain.

#include "ewpresenter/validation.h"
#include "libew/core/units.h"
#include <functional>
#include <string>

namespace ewpresenter {

using namespace libew::units;

class RadarPresenter {
public:
    struct Output {
        // Radar range equation
        Km max_range{};
        Db two_way_loss{};
        std::string max_range_str;
        std::string two_way_loss_str;

        // Pulse compression
        Db pulse_compression_gain{};
        std::string pulse_compression_gain_str;

        // Coherent integration
        Db coherent_integration_gain{};
        std::string coherent_integration_gain_str;

        // LPI advantage (from time-bandwidth product)
        Db lpi_advantage{};
        std::string lpi_advantage_str;

        // Target RCS (formatted with unit suffix for self-contained result panels)
        Dbsm target_rcs{};
        std::string target_rcs_str;

        bool valid{false};
    };

    RadarPresenter() noexcept;

    // -----------------------------------------------------------------------
    // Radar range inputs
    // -----------------------------------------------------------------------
    /// Transmitter output power (dBm). Range: -50 – 200 dBm.
    void set_tx_power(double dbm) noexcept;

    /// Antenna gain (dBi, same for Tx and Rx). Range: -30 – 60 dBi.
    void set_antenna_gain(double dbi) noexcept;

    /// Target radar cross-section (dBsm). Range: -40 – 60 dBsm.
    void set_target_rcs(double dbsm) noexcept;

    /// Radar carrier frequency (MHz). Range: 1 – 100 000 MHz.
    void set_frequency(double mhz) noexcept;

    /// Combined system losses (dB). Range: 0 – 30 dB.
    void set_system_losses(double db) noexcept;

    /// Receiver noise figure (dB). Range: 0 – 30 dB.
    void set_noise_figure(double db) noexcept;

    /// Receiver noise bandwidth (MHz). Range: 0.001 – 10 000 MHz.
    void set_bandwidth(double mhz) noexcept;

    /// Minimum required SNR for detection (dB). Range: -10 – 50 dB.
    void set_required_snr(double db) noexcept;

    // -----------------------------------------------------------------------
    // Signal processing inputs
    // -----------------------------------------------------------------------
    /// Pulse compression time-bandwidth product (dimensionless). Range: 1 – 1 000 000.
    void set_time_bandwidth_product(double tb) noexcept;

    /// Number of coherently integrated pulses. Range: 1 – 100 000.
    void set_num_pulses(int n) noexcept;

    // -----------------------------------------------------------------------
    // Accessors
    // -----------------------------------------------------------------------
    [[nodiscard]] const Output& output() const noexcept { return output_; }
    [[nodiscard]] bool is_valid() const noexcept { return output_.valid; }

    [[nodiscard]] double tx_power_dbm()            const noexcept { return tx_power_dbm_; }
    [[nodiscard]] double antenna_gain_dbi()        const noexcept { return antenna_gain_dbi_; }
    [[nodiscard]] double target_rcs_dbsm()         const noexcept { return target_rcs_dbsm_; }
    [[nodiscard]] double frequency_mhz()           const noexcept { return frequency_mhz_; }
    [[nodiscard]] double system_losses_db()        const noexcept { return system_losses_db_; }
    [[nodiscard]] double noise_figure_db()         const noexcept { return noise_figure_db_; }
    [[nodiscard]] double bandwidth_mhz()           const noexcept { return bandwidth_mhz_; }
    [[nodiscard]] double required_snr_db()         const noexcept { return required_snr_db_; }
    [[nodiscard]] double time_bandwidth_product()  const noexcept { return time_bandwidth_product_; }
    [[nodiscard]] int    num_pulses()              const noexcept { return num_pulses_; }

    void set_on_change(std::function<void(const Output&)> cb) noexcept {
        on_change_ = std::move(cb);
    }

private:
    double tx_power_dbm_           {60.0};   // 1 kW
    double antenna_gain_dbi_       {30.0};
    double target_rcs_dbsm_        {0.0};    // 1 m²
    double frequency_mhz_          {3000.0}; // X-band
    double system_losses_db_       {3.0};
    double noise_figure_db_        {5.0};
    double bandwidth_mhz_          {1.0};
    double required_snr_db_        {13.0};
    double time_bandwidth_product_ {100.0};
    int    num_pulses_             {16};

    FieldError tx_power_err_    {FieldError::none};
    FieldError antenna_gain_err_{FieldError::none};
    FieldError target_rcs_err_  {FieldError::none};
    FieldError frequency_err_   {FieldError::none};
    FieldError system_losses_err_{FieldError::none};
    FieldError noise_figure_err_{FieldError::none};
    FieldError bandwidth_err_   {FieldError::none};
    FieldError required_snr_err_{FieldError::none};
    FieldError tb_product_err_  {FieldError::none};

    Output output_;
    std::function<void(const Output&)> on_change_;

    void recompute() noexcept;
    void fire() noexcept;
};

} // namespace ewpresenter
