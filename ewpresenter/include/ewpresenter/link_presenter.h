#pragma once

/// @file link_presenter.h
/// @brief Presenter for one-way link budget and effective range.

#include "ewpresenter/presenter_base.h"
#include "ewpresenter/validation.h"
#include "libew/core/units.h"
#include <functional>
#include <string>

namespace ewpresenter {

using namespace libew::units;

class LinkPresenter : public PresenterBase<LinkPresenter> {
    friend PresenterBase<LinkPresenter>;
public:
    struct Output {
        // One-way link
        Dbm  received_power{};
        Db   path_loss{};
        Km   fresnel_zone_distance{};
        bool two_ray_regime{false};

        // Effective range
        Km   effective_range{};
        bool range_two_ray_regime{false};

        std::string received_power_str;
        std::string path_loss_str;
        std::string fresnel_zone_str;
        std::string regime_str;
        std::string effective_range_str;
        std::string range_regime_str;
        std::string link_margin_str;   ///< received_power - rx_sensitivity

        bool valid{false};
    };

    LinkPresenter() noexcept;

    /// Transmitter output power (dBm). Range: -50 to 200 dBm.
    void set_tx_power(double dbm) noexcept;

    /// Transmit antenna gain toward receiver (dB). Range: -30 to 60 dB.
    void set_tx_gain(double db) noexcept;

    /// Receive antenna gain toward transmitter (dB). Range: -30 to 60 dB.
    void set_rx_gain(double db) noexcept;

    /// Link distance (km). Range: 0.01 – 10 000 km.
    void set_distance(double km) noexcept;

    /// Transmit antenna height (m). Range: 0.1 – 100 000 m.
    void set_tx_height(double meters) noexcept;

    /// Receive antenna height (m). Range: 0.1 – 100 000 m.
    void set_rx_height(double meters) noexcept;

    /// Carrier frequency (MHz). Range: 0.1 – 100 000 MHz.
    void set_frequency(double mhz) noexcept;

    /// Receiver system sensitivity (dBm). Range: -200 to 0 dBm.
    void set_rx_sensitivity(double dbm) noexcept;

    [[nodiscard]] const Output& output() const noexcept { return output_; }
    [[nodiscard]] bool is_valid() const noexcept { return output_.valid; }

    [[nodiscard]] double tx_power_dbm()     const noexcept { return tx_power_dbm_; }
    [[nodiscard]] double tx_gain_db()       const noexcept { return tx_gain_db_; }
    [[nodiscard]] double rx_gain_db()       const noexcept { return rx_gain_db_; }
    [[nodiscard]] double distance_km()      const noexcept { return distance_km_; }
    [[nodiscard]] double tx_height_m()      const noexcept { return tx_height_m_; }
    [[nodiscard]] double rx_height_m()      const noexcept { return rx_height_m_; }
    [[nodiscard]] double frequency_mhz()    const noexcept { return frequency_mhz_; }
    [[nodiscard]] double rx_sensitivity_dbm() const noexcept { return rx_sensitivity_dbm_; }

    [[nodiscard]] FieldError tx_power_error()      const noexcept { return tx_power_err_; }
    [[nodiscard]] FieldError tx_gain_error()        const noexcept { return tx_gain_err_; }
    [[nodiscard]] FieldError rx_gain_error()        const noexcept { return rx_gain_err_; }
    [[nodiscard]] FieldError distance_error()       const noexcept { return distance_err_; }
    [[nodiscard]] FieldError tx_height_error()      const noexcept { return tx_height_err_; }
    [[nodiscard]] FieldError rx_height_error()      const noexcept { return rx_height_err_; }
    [[nodiscard]] FieldError frequency_error()      const noexcept { return frequency_err_; }
    [[nodiscard]] FieldError rx_sensitivity_error() const noexcept { return rx_sensitivity_err_; }

    void set_on_change(std::function<void(const Output&)> cb) noexcept {
        on_change_ = std::move(cb);
    }

private:
    double tx_power_dbm_       {20.0};
    double tx_gain_db_          {0.0};
    double rx_gain_db_          {0.0};
    double distance_km_        {32.6};
    double tx_height_m_        {10.0};
    double rx_height_m_        {10.0};
    double frequency_mhz_     {100.0};
    double rx_sensitivity_dbm_ {-120.5};

    FieldError tx_power_err_       {FieldError::none};
    FieldError tx_gain_err_        {FieldError::none};
    FieldError rx_gain_err_        {FieldError::none};
    FieldError distance_err_       {FieldError::none};
    FieldError tx_height_err_      {FieldError::none};
    FieldError rx_height_err_      {FieldError::none};
    FieldError frequency_err_      {FieldError::none};
    FieldError rx_sensitivity_err_ {FieldError::none};

    Output output_;
    std::function<void(const Output&)> on_change_;

    void recompute() noexcept;
    void fire() noexcept;
};

} // namespace ewpresenter
