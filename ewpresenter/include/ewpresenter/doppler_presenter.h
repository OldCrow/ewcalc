#pragma once

/// @file doppler_presenter.h
/// @brief Presenter for Doppler shift, PRF ambiguity (unambiguous range /
///        velocity, blind speed), and range / cross-range resolution.

#include "ewpresenter/presenter_base.h"
#include "ewpresenter/validation.h"
#include "libew/core/units.h"
#include <functional>
#include <string>

namespace ewpresenter {

using namespace libew::units;

class DopplerPresenter : public PresenterBase<DopplerPresenter> {
    friend PresenterBase<DopplerPresenter>;
public:
    struct Output {
        // Doppler
        double doppler_shift_hz{};
        std::string doppler_shift_str;

        // PRF ambiguity
        Km unambiguous_range{};
        double blind_speed_m_s{};
        double unambiguous_velocity_m_s{};
        std::string unambiguous_range_str;
        std::string blind_speed_str;
        std::string unambiguous_velocity_str;

        // Resolution
        Meters range_resolution{};
        Meters cross_range_az{};
        Meters cross_range_el{};
        std::string range_resolution_str;
        std::string cross_range_az_str;
        std::string cross_range_el_str;

        bool valid{false};
    };

    DopplerPresenter() noexcept;

    // -----------------------------------------------------------------------
    // Doppler / ambiguity inputs
    // -----------------------------------------------------------------------
    /// Radar carrier frequency (MHz). Range: 1 – 100 000 MHz.
    void set_frequency(double mhz) noexcept;

    /// Target radial speed (m/s, closing positive). Range: -3000 – 3000.
    void set_radial_speed(double m_s) noexcept;

    /// Pulse repetition frequency (Hz). Range: 10 – 1 000 000.
    void set_prf(double hz) noexcept;

    // -----------------------------------------------------------------------
    // Resolution inputs
    // -----------------------------------------------------------------------
    /// Waveform (compressed) bandwidth (MHz). Range: 0.001 – 10 000.
    void set_bandwidth(double mhz) noexcept;

    /// Target range (km). Range: 0.1 – 5000.
    void set_target_range(double km) noexcept;

    /// Azimuth 3 dB beamwidth (degrees). Range: 0.1 – 45.
    void set_beamwidth_az(double deg) noexcept;

    /// Elevation 3 dB beamwidth (degrees). Range: 0.1 – 45.
    void set_beamwidth_el(double deg) noexcept;

    // -----------------------------------------------------------------------
    // Accessors
    // -----------------------------------------------------------------------
    [[nodiscard]] const Output& output() const noexcept { return output_; }
    [[nodiscard]] bool is_valid() const noexcept { return output_.valid; }

    [[nodiscard]] double frequency_mhz()   const noexcept { return frequency_mhz_; }
    [[nodiscard]] double radial_speed_m_s() const noexcept { return radial_speed_m_s_; }
    [[nodiscard]] double prf_hz()          const noexcept { return prf_hz_; }
    [[nodiscard]] double bandwidth_mhz()   const noexcept { return bandwidth_mhz_; }
    [[nodiscard]] double target_range_km() const noexcept { return target_range_km_; }
    [[nodiscard]] double beamwidth_az_deg() const noexcept { return beamwidth_az_deg_; }
    [[nodiscard]] double beamwidth_el_deg() const noexcept { return beamwidth_el_deg_; }
    [[nodiscard]] FieldError frequency_error()    const noexcept { return frequency_err_; }
    [[nodiscard]] FieldError radial_speed_error() const noexcept { return radial_speed_err_; }
    [[nodiscard]] FieldError prf_error()          const noexcept { return prf_err_; }
    [[nodiscard]] FieldError bandwidth_error()    const noexcept { return bandwidth_err_; }
    [[nodiscard]] FieldError target_range_error() const noexcept { return target_range_err_; }
    [[nodiscard]] FieldError beamwidth_az_error() const noexcept { return beamwidth_az_err_; }
    [[nodiscard]] FieldError beamwidth_el_error() const noexcept { return beamwidth_el_err_; }

    void set_on_change(std::function<void(const Output&)> cb) noexcept {
        on_change_ = std::move(cb);
    }

private:
    double frequency_mhz_    {10000.0}; // X-band
    double radial_speed_m_s_ {300.0};
    double prf_hz_           {1000.0};
    double bandwidth_mhz_    {1.0};
    double target_range_km_  {100.0};
    double beamwidth_az_deg_ {2.0};
    double beamwidth_el_deg_ {2.0};

    FieldError frequency_err_    {FieldError::none};
    FieldError radial_speed_err_ {FieldError::none};
    FieldError prf_err_          {FieldError::none};
    FieldError bandwidth_err_    {FieldError::none};
    FieldError target_range_err_ {FieldError::none};
    FieldError beamwidth_az_err_ {FieldError::none};
    FieldError beamwidth_el_err_ {FieldError::none};

    Output output_;
    std::function<void(const Output&)> on_change_;

    void recompute() noexcept;
    void fire() noexcept;
};

} // namespace ewpresenter
