#pragma once

/// @file digital_presenter.h
/// @brief Presenter for digital link Eb/N₀ conversion and DSSS spread-spectrum
///        jamming analysis.

#include "ewpresenter/validation.h"
#include "libew/core/units.h"
#include <functional>
#include <string>

namespace ewpresenter {

using namespace libew::units;

class DigitalPresenter {
public:
    struct Output {
        // Eb/N₀ section
        Db  eb_no{};
        std::string eb_no_str;       ///< Eb/N₀ from received SNR

        // DSSS section
        Db  process_gain{};
        Db  jamming_margin{};
        Db  required_js{};
        std::string process_gain_str;
        std::string jamming_margin_str;
        std::string required_js_str;

        bool valid{false};
    };

    DigitalPresenter() noexcept;

    // -----------------------------------------------------------------------
    // Eb/N₀ inputs
    // -----------------------------------------------------------------------
    /// Carrier SNR in the receive noise bandwidth (dB). Range: -30 – 60 dB.
    void set_snr(double db) noexcept;

    /// Receiver noise bandwidth (MHz). Range: 0.001 – 10 000 MHz.
    void set_bandwidth(double mhz) noexcept;

    // -----------------------------------------------------------------------
    // Shared input
    // -----------------------------------------------------------------------
    /// Information bit rate (Mbps — stored as Mhz). Range: 0.0001 – 10 000 Mbps.
    /// Used by both the Eb/N₀ and DSSS calculations.
    void set_data_rate(double mbps) noexcept;

    // -----------------------------------------------------------------------
    // DSSS inputs
    // -----------------------------------------------------------------------
    /// Spread-spectrum chip rate (Mcps — stored as Mhz). Range: 0.0001 – 10 000 Mcps.
    void set_chip_rate(double mcps) noexcept;

    /// Required Eb/N₀ for acceptable BER (dB). Range: -10 – 30 dB.
    void set_required_eb_no(double db) noexcept;

    /// Implementation loss — practical degradation from non-ideal spreading (dB).
    /// Range: 0 – 10 dB. Default: 1 dB.
    void set_implementation_loss(double db) noexcept;

    // -----------------------------------------------------------------------
    // Accessors
    // -----------------------------------------------------------------------
    [[nodiscard]] const Output& output() const noexcept { return output_; }
    [[nodiscard]] bool is_valid() const noexcept { return output_.valid; }

    [[nodiscard]] double snr_db()               const noexcept { return snr_db_; }
    [[nodiscard]] double bandwidth_mhz()        const noexcept { return bandwidth_mhz_; }
    [[nodiscard]] double data_rate_mhz()        const noexcept { return data_rate_mhz_; }
    [[nodiscard]] double chip_rate_mhz()        const noexcept { return chip_rate_mhz_; }
    [[nodiscard]] double required_eb_no_db()    const noexcept { return required_eb_no_db_; }
    [[nodiscard]] double implementation_loss_db() const noexcept { return implementation_loss_db_; }

    [[nodiscard]] FieldError snr_error()               const noexcept { return snr_err_; }
    [[nodiscard]] FieldError bandwidth_error()         const noexcept { return bandwidth_err_; }
    [[nodiscard]] FieldError data_rate_error()         const noexcept { return data_rate_err_; }
    [[nodiscard]] FieldError chip_rate_error()         const noexcept { return chip_rate_err_; }
    [[nodiscard]] FieldError required_eb_no_error()    const noexcept { return required_eb_no_err_; }
    [[nodiscard]] FieldError implementation_loss_error() const noexcept { return impl_loss_err_; }

    void set_on_change(std::function<void(const Output&)> cb) noexcept {
        on_change_ = std::move(cb);
    }

private:
    double snr_db_                {10.0};
    double bandwidth_mhz_         {1.0};
    double data_rate_mhz_         {0.1};   // 100 kbps
    double chip_rate_mhz_         {10.0};  // 10 Mcps → PG = 20 dB
    double required_eb_no_db_     {10.0};
    double implementation_loss_db_{1.0};

    FieldError snr_err_            {FieldError::none};
    FieldError bandwidth_err_      {FieldError::none};
    FieldError data_rate_err_      {FieldError::none};
    FieldError chip_rate_err_      {FieldError::none};
    FieldError required_eb_no_err_ {FieldError::none};
    FieldError impl_loss_err_      {FieldError::none};

    Output output_;
    std::function<void(const Output&)> on_change_;

    void recompute() noexcept;
    void fire() noexcept;
};

} // namespace ewpresenter
