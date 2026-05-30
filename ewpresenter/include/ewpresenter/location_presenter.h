#pragma once

/// @file location_presenter.h
/// @brief Presenter for emitter location accuracy (CEP from AOA bearing error and EEP).

#include "ewpresenter/validation.h"
#include "libew/core/units.h"
#include <functional>
#include <string>

namespace ewpresenter {

using namespace libew::units;

class LocationPresenter {
public:
    struct Output {
        // AOA section
        Km  cep_aoa{};
        std::string cep_aoa_str;

        // EEP section
        Km  cep_eep{};
        std::string cep_eep_str;

        bool valid{false};
    };

    LocationPresenter() noexcept;

    // -----------------------------------------------------------------------
    // AOA inputs
    // -----------------------------------------------------------------------
    /// RMS bearing error (degrees). Range: 0.01 – 45 degrees.
    void set_rms_bearing_error(double degrees) noexcept;

    /// Range from each receiver to emitter (km). Range: 0.1 – 10 000 km.
    void set_aoa_range(double km) noexcept;

    // -----------------------------------------------------------------------
    // EEP inputs
    // -----------------------------------------------------------------------
    /// Semi-major axis of the error ellipse (km, 1-sigma). Range: 0.001 – 1000 km.
    void set_semi_major(double km) noexcept;

    /// Semi-minor axis of the error ellipse (km, 1-sigma). Range: 0.001 – 1000 km.
    void set_semi_minor(double km) noexcept;

    // -----------------------------------------------------------------------
    // Accessors
    // -----------------------------------------------------------------------
    [[nodiscard]] const Output& output() const noexcept { return output_; }
    [[nodiscard]] bool is_valid() const noexcept { return output_.valid; }

    [[nodiscard]] double rms_bearing_error_deg() const noexcept { return rms_bearing_error_deg_; }
    [[nodiscard]] double aoa_range_km()          const noexcept { return aoa_range_km_; }
    [[nodiscard]] double semi_major_km()         const noexcept { return semi_major_km_; }
    [[nodiscard]] double semi_minor_km()         const noexcept { return semi_minor_km_; }

    [[nodiscard]] FieldError rms_bearing_error()  const noexcept { return rms_bearing_err_; }
    [[nodiscard]] FieldError aoa_range_error()    const noexcept { return aoa_range_err_; }
    [[nodiscard]] FieldError semi_major_error()   const noexcept { return semi_major_err_; }
    [[nodiscard]] FieldError semi_minor_error()   const noexcept { return semi_minor_err_; }

    void set_on_change(std::function<void(const Output&)> cb) noexcept {
        on_change_ = std::move(cb);
    }

private:
    double rms_bearing_error_deg_ {1.0};
    double aoa_range_km_          {100.0};
    double semi_major_km_         {2.0};
    double semi_minor_km_         {1.0};

    FieldError rms_bearing_err_ {FieldError::none};
    FieldError aoa_range_err_   {FieldError::none};
    FieldError semi_major_err_  {FieldError::none};
    FieldError semi_minor_err_  {FieldError::none};

    Output output_;
    std::function<void(const Output&)> on_change_;

    void recompute() noexcept;
    void fire() noexcept;
};

} // namespace ewpresenter
