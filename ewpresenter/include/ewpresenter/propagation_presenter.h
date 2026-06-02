#pragma once

/// @file propagation_presenter.h
/// @brief Presenter for RF propagation path loss calculations.

#include "ewpresenter/validation.h"
#include "libew/core/units.h"
#include <functional>
#include <string>

namespace ewpresenter {

using namespace libew::units;

class PropagationPresenter {
public:
    // -----------------------------------------------------------------------
    // Output — all fields updated atomically on each recompute()
    // -----------------------------------------------------------------------
    struct Output {
        // Raw values (valid only when valid == true)
        Db   fspl{};
        Db   two_ray_loss{};
        Km   fresnel_zone_distance{};
        Db   path_loss{};
        bool two_ray_regime{false};

        // Formatted strings (always set; show "—" when invalid)
        std::string fspl_str;
        std::string two_ray_loss_str;
        std::string fresnel_zone_str;
        std::string path_loss_str;
        std::string regime_str;

        // Earth geometry (from existing distance / height inputs)
        Meters earth_bulge{};      ///< Earth rise at path midpoint (m)
        Km     horizon_range{};    ///< Radar horizon range (km)
        std::string earth_bulge_str;
        std::string horizon_range_str;

        bool valid{false};
    };

    PropagationPresenter() noexcept;

    // -----------------------------------------------------------------------
    // Input setters — each validates, recomputes, and fires on_change
    // -----------------------------------------------------------------------
    /// Link distance (km). Valid range: 0.01 – 10 000 km.
    void set_distance(double km) noexcept;

    /// Carrier frequency (MHz). Valid range: 0.1 – 100 000 MHz.
    void set_frequency(double mhz) noexcept;

    /// Transmit antenna height above ground (m). Valid range: 0.1 – 100 000 m.
    void set_tx_height(double meters) noexcept;

    /// Receive antenna height above ground (m). Valid range: 0.1 – 100 000 m.
    void set_rx_height(double meters) noexcept;

    // -----------------------------------------------------------------------
    // Accessors
    // -----------------------------------------------------------------------
    [[nodiscard]] const Output& output() const noexcept { return output_; }
    [[nodiscard]] bool is_valid() const noexcept { return output_.valid; }

    [[nodiscard]] double distance_km()   const noexcept { return distance_km_; }
    [[nodiscard]] double frequency_mhz() const noexcept { return frequency_mhz_; }
    [[nodiscard]] double tx_height_m()   const noexcept { return tx_height_m_; }
    [[nodiscard]] double rx_height_m()   const noexcept { return rx_height_m_; }

    [[nodiscard]] FieldError distance_error()   const noexcept { return distance_err_; }
    [[nodiscard]] FieldError frequency_error()  const noexcept { return frequency_err_; }
    [[nodiscard]] FieldError tx_height_error()  const noexcept { return tx_height_err_; }
    [[nodiscard]] FieldError rx_height_error()  const noexcept { return rx_height_err_; }

    // -----------------------------------------------------------------------
    // Callback — fired after every recompute (valid or not)
    // -----------------------------------------------------------------------
    void set_on_change(std::function<void(const Output&)> cb) noexcept {
        on_change_ = std::move(cb);
    }

private:
    // Inputs (stored as raw doubles; views read/write doubles)
    double distance_km_   {32.6};
    double frequency_mhz_ {100.0};
    double tx_height_m_   {10.0};
    double rx_height_m_   {10.0};

    FieldError distance_err_  {FieldError::none};
    FieldError frequency_err_ {FieldError::none};
    FieldError tx_height_err_ {FieldError::none};
    FieldError rx_height_err_ {FieldError::none};

    Output output_;
    std::function<void(const Output&)> on_change_;

    void recompute() noexcept;
    void fire() noexcept;
};

} // namespace ewpresenter
