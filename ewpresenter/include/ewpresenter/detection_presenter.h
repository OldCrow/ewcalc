#pragma once

/// @file detection_presenter.h
/// @brief Presenter for radar detection statistics: required SNR (Albersheim /
///        Shnidman), Swerling fluctuation loss, scan timing, false-alarm rate.

#include "ewpresenter/presenter_base.h"
#include "ewpresenter/validation.h"
#include "libew/core/units.h"
#include <functional>
#include <string>

namespace ewpresenter {

using namespace libew::units;

class DetectionPresenter : public PresenterBase<DetectionPresenter> {
    friend PresenterBase<DetectionPresenter>;
public:
    struct Output {
        // Required SNR
        Db required_snr{};          ///< Shnidman, selected Swerling case
        Db required_snr_albersheim{}; ///< Albersheim, nonfluctuating reference
        Db fluctuation_loss{};
        std::string required_snr_str;
        std::string required_snr_albersheim_str;
        std::string fluctuation_loss_str;

        // Scan timing
        Seconds dwell_time{};
        double hits_per_scan{};
        std::string dwell_time_str;
        std::string hits_per_scan_str;

        // False-alarm rate
        double far_hz{};
        std::string far_str;

        bool valid{false};
    };

    DetectionPresenter() noexcept;

    // -----------------------------------------------------------------------
    // Detection inputs
    // -----------------------------------------------------------------------
    /// Probability of detection. Range: 0.1 – 0.99 (Shnidman validity range).
    void set_pd(double pd) noexcept;

    /// False-alarm probability exponent x, Pfa = 10^x. Range: -9 – -3
    /// (Shnidman validity range).
    void set_pfa_exponent(double x) noexcept;

    /// Pulses noncoherently integrated. Range: 1 – 100 (Shnidman validity range).
    void set_num_pulses(int n) noexcept;

    /// Swerling case: 0 (nonfluctuating) – 4. Range: 0 – 4.
    void set_swerling_case(int c) noexcept;

    // -----------------------------------------------------------------------
    // Scan timing inputs
    // -----------------------------------------------------------------------
    /// Azimuth 3 dB beamwidth (degrees). Range: 0.1 – 45.
    void set_beamwidth(double deg) noexcept;

    /// Antenna scan rate (degrees/second). Range: 1 – 720.
    void set_scan_rate(double deg_s) noexcept;

    /// Pulse repetition frequency (Hz). Range: 10 – 1 000 000.
    void set_prf(double hz) noexcept;

    // -----------------------------------------------------------------------
    // False-alarm-rate inputs
    // -----------------------------------------------------------------------
    /// Receiver noise bandwidth (MHz). Range: 0.001 – 10 000.
    void set_bandwidth(double mhz) noexcept;

    // -----------------------------------------------------------------------
    // Accessors
    // -----------------------------------------------------------------------
    [[nodiscard]] const Output& output() const noexcept { return output_; }
    [[nodiscard]] bool is_valid() const noexcept { return output_.valid; }

    [[nodiscard]] double pd()            const noexcept { return pd_; }
    [[nodiscard]] double pfa_exponent()  const noexcept { return pfa_exponent_; }
    [[nodiscard]] int    num_pulses()    const noexcept { return num_pulses_; }
    [[nodiscard]] int    swerling_case() const noexcept { return swerling_case_; }
    [[nodiscard]] double beamwidth_deg() const noexcept { return beamwidth_deg_; }
    [[nodiscard]] double scan_rate_deg_s() const noexcept { return scan_rate_deg_s_; }
    [[nodiscard]] double prf_hz()        const noexcept { return prf_hz_; }
    [[nodiscard]] double bandwidth_mhz() const noexcept { return bandwidth_mhz_; }
    [[nodiscard]] FieldError pd_error()            const noexcept { return pd_err_; }
    [[nodiscard]] FieldError pfa_exponent_error()  const noexcept { return pfa_exponent_err_; }
    [[nodiscard]] FieldError num_pulses_error()    const noexcept { return num_pulses_err_; }
    [[nodiscard]] FieldError swerling_case_error() const noexcept { return swerling_case_err_; }
    [[nodiscard]] FieldError beamwidth_error()     const noexcept { return beamwidth_err_; }
    [[nodiscard]] FieldError scan_rate_error()     const noexcept { return scan_rate_err_; }
    [[nodiscard]] FieldError prf_error()           const noexcept { return prf_err_; }
    [[nodiscard]] FieldError bandwidth_error()     const noexcept { return bandwidth_err_; }

    void set_on_change(std::function<void(const Output&)> cb) noexcept {
        on_change_ = std::move(cb);
    }

private:
    double pd_             {0.9};
    double pfa_exponent_   {-6.0};
    int    num_pulses_     {10};
    int    swerling_case_  {1};
    double beamwidth_deg_  {2.0};
    double scan_rate_deg_s_{36.0};  // 10 s rotation
    double prf_hz_         {1000.0};
    double bandwidth_mhz_  {1.0};

    FieldError pd_err_            {FieldError::none};
    FieldError pfa_exponent_err_  {FieldError::none};
    FieldError num_pulses_err_    {FieldError::none};
    FieldError swerling_case_err_ {FieldError::none};
    FieldError beamwidth_err_     {FieldError::none};
    FieldError scan_rate_err_     {FieldError::none};
    FieldError prf_err_           {FieldError::none};
    FieldError bandwidth_err_     {FieldError::none};

    Output output_;
    std::function<void(const Output&)> on_change_;

    void recompute() noexcept;
    void fire() noexcept;
};

} // namespace ewpresenter
