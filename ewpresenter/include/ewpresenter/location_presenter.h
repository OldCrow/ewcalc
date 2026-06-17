#pragma once

/// @file location_presenter.h
/// @brief Presenter for emitter location accuracy (CEP from AOA bearing error and EEP).

#include "ewpresenter/presenter_base.h"
#include "ewpresenter/validation.h"
#include "libew/core/units.h"
#include <functional>
#include <string>

namespace ewpresenter {

using namespace libew::units;

class LocationPresenter : public PresenterBase<LocationPresenter> {
    friend PresenterBase<LocationPresenter>;
public:
    struct Output {
        // AOA section
        Km  cep_aoa{};
        std::string cep_aoa_str;

        // TDOA section
        Km  cep_tdoa{};
        std::string cep_tdoa_str;

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

    // -----------------------------------------------------------------------
    // TDOA inputs
    // -----------------------------------------------------------------------
    /// RMS TDOA timing error (nanoseconds). Range: 0.001 – 100 000 ns.
    void set_rms_time_error(double ns) noexcept;

    /// Receiver baseline separation (km). Range: 0.1 – 10 000 km.
    /// Wider baseline improves TDOA position accuracy.
    void set_baseline(double km) noexcept;

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
    [[nodiscard]] double rms_time_error_ns()     const noexcept { return rms_time_error_ns_; }
    [[nodiscard]] double baseline_km()           const noexcept { return baseline_km_; }
    [[nodiscard]] double semi_major_km()         const noexcept { return semi_major_km_; }
    [[nodiscard]] double semi_minor_km()         const noexcept { return semi_minor_km_; }

    [[nodiscard]] FieldError rms_bearing_error()  const noexcept { return rms_bearing_err_; }
    [[nodiscard]] FieldError aoa_range_error()    const noexcept { return aoa_range_err_; }
    [[nodiscard]] FieldError rms_time_error()     const noexcept { return rms_time_err_; }
    [[nodiscard]] FieldError baseline_error()     const noexcept { return baseline_err_; }
    [[nodiscard]] FieldError semi_major_error()   const noexcept { return semi_major_err_; }
    [[nodiscard]] FieldError semi_minor_error()   const noexcept { return semi_minor_err_; }

    void set_on_change(std::function<void(const Output&)> cb) noexcept {
        on_change_ = std::move(cb);
    }

private:
    double rms_bearing_error_deg_ {1.0};
    double aoa_range_km_          {100.0};
    double rms_time_error_ns_     {10.0};
    double baseline_km_           {10.0};  ///< Typical tactical baseline
    double semi_major_km_         {2.0};
    double semi_minor_km_         {1.0};

    FieldError rms_bearing_err_ {FieldError::none};
    FieldError aoa_range_err_   {FieldError::none};
    FieldError rms_time_err_    {FieldError::none};
    FieldError baseline_err_    {FieldError::none};
    FieldError semi_major_err_  {FieldError::none};
    FieldError semi_minor_err_  {FieldError::none};

    Output output_;
    std::function<void(const Output&)> on_change_;

    void recompute() noexcept;
    void fire() noexcept;
};

} // namespace ewpresenter
