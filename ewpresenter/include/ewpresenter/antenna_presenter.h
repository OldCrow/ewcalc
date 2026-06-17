#pragma once

/// @file antenna_presenter.h
/// @brief Presenter for antenna gain, ERP, beamwidth, and wavelength.

#include "ewpresenter/presenter_base.h"
#include "ewpresenter/validation.h"
#include "libew/core/units.h"
#include <functional>
#include <string>

namespace ewpresenter {

using namespace libew::units;

class AntennaPresenter : public PresenterBase<AntennaPresenter> {
    friend PresenterBase<AntennaPresenter>;  ///< Allows base to call recompute/fire.
public:
    struct Output {
        // ERP
        Dbm erp{};
        std::string erp_str;

        // Beamwidth from gain
        Degrees beamwidth_from_gain{};
        std::string beamwidth_from_gain_str;

        // Gain from beamwidth
        Db gain_from_beamwidth{};
        std::string gain_from_beamwidth_str;

        // Wavelength
        Meters wavelength{};
        std::string wavelength_str;

        bool valid{false};
    };

    AntennaPresenter() noexcept;

    // -----------------------------------------------------------------------
    // Inputs
    // -----------------------------------------------------------------------
    /// Antenna gain (dBi). Range: -10 – 60 dBi.
    void set_gain(double dbi) noexcept;

    /// Azimuth 3 dB beamwidth (degrees). Range: 0.1 – 360 degrees.
    void set_az_beamwidth(double deg) noexcept;

    /// Elevation 3 dB beamwidth (degrees). Range: 0.1 – 360 degrees.
    void set_el_beamwidth(double deg) noexcept;

    /// Transmitter output power (dBm). Range: -30 – 100 dBm.
    void set_tx_power(double dbm) noexcept;

    /// Carrier frequency (MHz). Range: 0.1 – 100 000 MHz.
    void set_frequency(double mhz) noexcept;

    // -----------------------------------------------------------------------
    // Accessors
    // -----------------------------------------------------------------------
    [[nodiscard]] const Output& output() const noexcept { return output_; }
    [[nodiscard]] bool is_valid() const noexcept { return output_.valid; }

    [[nodiscard]] double gain_dbi()        const noexcept { return gain_dbi_; }
    [[nodiscard]] double az_beamwidth_deg() const noexcept { return az_beamwidth_deg_; }
    [[nodiscard]] double el_beamwidth_deg() const noexcept { return el_beamwidth_deg_; }
    [[nodiscard]] double tx_power_dbm()    const noexcept { return tx_power_dbm_; }
    [[nodiscard]] double frequency_mhz()   const noexcept { return frequency_mhz_; }

    [[nodiscard]] FieldError gain_error()          const noexcept { return gain_err_; }
    [[nodiscard]] FieldError az_beamwidth_error()   const noexcept { return az_bw_err_; }
    [[nodiscard]] FieldError el_beamwidth_error()   const noexcept { return el_bw_err_; }
    [[nodiscard]] FieldError tx_power_error()       const noexcept { return tx_power_err_; }
    [[nodiscard]] FieldError frequency_error()      const noexcept { return frequency_err_; }

    void set_on_change(std::function<void(const Output&)> cb) noexcept {
        on_change_ = std::move(cb);
    }

private:
    double gain_dbi_          { 0.0};
    double az_beamwidth_deg_  {60.0};
    double el_beamwidth_deg_  {60.0};
    double tx_power_dbm_      {30.0};
    double frequency_mhz_     {1000.0};

    FieldError gain_err_      {FieldError::none};
    FieldError az_bw_err_     {FieldError::none};
    FieldError el_bw_err_     {FieldError::none};
    FieldError tx_power_err_  {FieldError::none};
    FieldError frequency_err_ {FieldError::none};

    Output output_;
    std::function<void(const Output&)> on_change_;

    void recompute() noexcept; ///< CRTP hook — called by PresenterBase::update_field.
    void fire() noexcept;      ///< CRTP hook — called by PresenterBase::update_field.
};

} // namespace ewpresenter
