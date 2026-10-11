#pragma once

/// @file radar_jamming_presenter.h
/// @brief Presenter for radar jamming: self-protection (SPJ) and stand-off
///        (SOJ) J/S at the radar receiver, and burnthrough range.

#include "ewpresenter/presenter_base.h"
#include "ewpresenter/validation.h"
#include "libew/core/units.h"
#include <functional>
#include <string>

namespace ewpresenter {

using namespace libew::units;

class RadarJammingPresenter : public PresenterBase<RadarJammingPresenter> {
    friend PresenterBase<RadarJammingPresenter>;
public:
    /// Jamming geometry, selected by set_geometry().
    static constexpr int kSelfProtection = 0;
    static constexpr int kStandOff       = 1;

    struct Output {
        Db  js_ratio{};
        Db  js_margin{};             ///< J/S minus the threshold
        Km  burnthrough_range{};
        bool jammer_effective{false}; ///< J/S >= threshold at the current range
        std::string js_ratio_str;
        std::string js_margin_str;
        std::string burnthrough_range_str;
        std::string status_str;       ///< "Jammer effective" / "Radar burns through"

        bool valid{false};
    };

    RadarJammingPresenter() noexcept;

    // -----------------------------------------------------------------------
    // Inputs
    // -----------------------------------------------------------------------
    /// Geometry: kSelfProtection (0) or kStandOff (1).
    void set_geometry(int g) noexcept;

    /// Radar ERP toward the target (dBm). Range: -100 – 200 dBm.
    void set_radar_erp(double dbm) noexcept;

    /// Jammer ERP toward the radar, within the radar bandwidth (dBm).
    /// Range: -100 – 200 dBm.
    void set_jammer_erp(double dbm) noexcept;

    /// Target radar cross-section (dBsm). Range: -40 – 60 dBsm.
    void set_target_rcs(double dbsm) noexcept;

    /// Radar-to-target range (km). Range: 0.01 – 10 000 km.
    void set_target_range(double km) noexcept;

    /// Radar-to-jammer range, stand-off only (km). Range: 0.01 – 10 000 km.
    void set_jammer_range(double km) noexcept;

    /// Radar receive gain toward the target, stand-off only (dB). Range: -30 – 60 dB.
    void set_mainbeam_gain(double db) noexcept;

    /// Radar receive gain toward the jammer, stand-off only (dB). Range: -30 – 60 dB.
    void set_sidelobe_gain(double db) noexcept;

    /// J/S at which jamming is considered effective (dB). Range: -30 – 30 dB.
    void set_js_threshold(double db) noexcept;

    // -----------------------------------------------------------------------
    // Accessors
    // -----------------------------------------------------------------------
    [[nodiscard]] const Output& output() const noexcept { return output_; }
    [[nodiscard]] bool is_valid() const noexcept { return output_.valid; }

    [[nodiscard]] int    geometry()           const noexcept { return geometry_; }
    [[nodiscard]] double radar_erp_dbm()      const noexcept { return radar_erp_dbm_; }
    [[nodiscard]] double jammer_erp_dbm()     const noexcept { return jammer_erp_dbm_; }
    [[nodiscard]] double target_rcs_dbsm()    const noexcept { return target_rcs_dbsm_; }
    [[nodiscard]] double target_range_km()    const noexcept { return target_range_km_; }
    [[nodiscard]] double jammer_range_km()    const noexcept { return jammer_range_km_; }
    [[nodiscard]] double mainbeam_gain_db()   const noexcept { return mainbeam_gain_db_; }
    [[nodiscard]] double sidelobe_gain_db()   const noexcept { return sidelobe_gain_db_; }
    [[nodiscard]] double js_threshold_db()    const noexcept { return js_threshold_db_; }

    [[nodiscard]] FieldError geometry_error()      const noexcept { return geometry_err_; }
    [[nodiscard]] FieldError radar_erp_error()     const noexcept { return radar_erp_err_; }
    [[nodiscard]] FieldError jammer_erp_error()    const noexcept { return jammer_erp_err_; }
    [[nodiscard]] FieldError target_rcs_error()    const noexcept { return target_rcs_err_; }
    [[nodiscard]] FieldError target_range_error()  const noexcept { return target_range_err_; }
    [[nodiscard]] FieldError jammer_range_error()  const noexcept { return jammer_range_err_; }
    [[nodiscard]] FieldError mainbeam_gain_error() const noexcept { return mainbeam_gain_err_; }
    [[nodiscard]] FieldError sidelobe_gain_error() const noexcept { return sidelobe_gain_err_; }
    [[nodiscard]] FieldError js_threshold_error()  const noexcept { return js_threshold_err_; }

    void set_on_change(std::function<void(const Output&)> cb) noexcept {
        on_change_ = std::move(cb);
    }

private:
    int    geometry_         {kSelfProtection};
    double radar_erp_dbm_    {110.0};   // 100 kW peak + 30 dBi
    double jammer_erp_dbm_   {40.0};    // 10 W + 10 dBi
    double target_rcs_dbsm_  {10.0};
    double target_range_km_  {20.0};
    double jammer_range_km_  {50.0};
    double mainbeam_gain_db_ {30.0};
    double sidelobe_gain_db_ {0.0};     // sidelobes 30 dB down
    double js_threshold_db_  {0.0};

    FieldError geometry_err_      {FieldError::none};
    FieldError radar_erp_err_     {FieldError::none};
    FieldError jammer_erp_err_    {FieldError::none};
    FieldError target_rcs_err_    {FieldError::none};
    FieldError target_range_err_  {FieldError::none};
    FieldError jammer_range_err_  {FieldError::none};
    FieldError mainbeam_gain_err_ {FieldError::none};
    FieldError sidelobe_gain_err_ {FieldError::none};
    FieldError js_threshold_err_  {FieldError::none};

    Output output_;
    std::function<void(const Output&)> on_change_;

    void recompute() noexcept;
    void fire() noexcept;
};

} // namespace ewpresenter
