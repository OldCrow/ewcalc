#pragma once

/// @file jamming.h
/// @brief Jamming effectiveness analysis: J/S, burnthrough range, partial-band jamming.
///
/// Sources: comms J/S — Adamy EW102 Sec 5.8.1 p.138 / EW103 Sec 9.1 p.252.
/// Burnthrough — Adamy EW101 Sec 9.3 pp.187-191 (fullest treatment) and
/// EW102 Sec 5.8 pp.137-140. Partial-band — Adamy EW102 Sec 5.9.1.2
/// pp.144-147 / EW103 Sec 9.3.1 pp.259-262. Full pins: docs/formulas.md.

#include "libew/core/units.h"

namespace libew::jamming {

using namespace libew::units;

// ---------------------------------------------------------------------------
// Communications jamming — J/S ratio
// ---------------------------------------------------------------------------

struct JammingResult {
    Db  js_ratio;                    ///< Jamming-to-signal ratio (dB)
    Dbm jammer_power_at_receiver;    ///< Jammer power at receiver input (dBm)
    Dbm signal_power_at_receiver;    ///< Desired signal power at receiver input (dBm)
};

/// Compute J/S ratio for communications jamming.
///
/// Each path is evaluated independently using the appropriate propagation model
/// (LOS or 2-ray based on height/distance geometry at the given frequency).
///
/// J/S = J_rx - S_rx   (all in dBm, result in dB)
///
/// @param signal_erp            ERP of desired signal transmitter toward receiver (dBm)
/// @param jammer_erp            ERP of jammer toward receiver (dBm)
/// @param signal_to_rx_dist     Distance from signal tx to receiver (km)
/// @param jammer_to_rx_dist     Distance from jammer to receiver (km)
/// @param signal_tx_height      Signal transmitter antenna height (m)
/// @param jammer_height         Jammer antenna height (m)
/// @param rx_height             Receiver antenna height (m)
/// @param frequency             Carrier frequency (MHz)
/// @param rx_gain_toward_signal Receiver antenna gain toward signal transmitter (dB)
/// @param rx_gain_toward_jammer Receiver antenna gain toward jammer (dB)
/// @return JammingResult with J/S, jammer power, and signal power at receiver
[[nodiscard]] JammingResult comms_jamming_js(
    Dbm    signal_erp,
    Dbm    jammer_erp,
    Km     signal_to_rx_dist,
    Km     jammer_to_rx_dist,
    Meters signal_tx_height,
    Meters jammer_height,
    Meters rx_height,
    Mhz    frequency,
    Db     rx_gain_toward_signal,
    Db     rx_gain_toward_jammer) noexcept;

// ---------------------------------------------------------------------------
// Burnthrough range
// ---------------------------------------------------------------------------

/// Compute burnthrough range — the range at which J/S drops to a specified threshold.
///
/// Solves analytically for the LOS regime; selects the 2-ray closed-form inversion
/// when the LOS solution exceeds the Fresnel zone crossover distance.
///
/// LOS  inversion: d = 10^((margin − 32.44 − 20·log₁₀(f)) / 20)
/// 2-ray inversion: d = 10^((margin − 120 + 20·log₁₀(h_tx) + 20·log₁₀(h_rx)) / 40)
/// Both expressions are from the propagation loss model in libew/propagation.
///
/// @param signal_erp        ERP of desired signal transmitter (dBm)
/// @param jammer_erp        ERP of jammer (dBm)
/// @param jammer_to_rx_dist Distance from jammer to receiver (km)
/// @param signal_tx_height  Signal transmitter antenna height (m)
/// @param jammer_height     Jammer antenna height (m)
/// @param rx_height         Receiver antenna height (m)
/// @param frequency         Carrier frequency (MHz)
/// @param js_threshold      J/S level at which jamming is considered effective (dB)
/// @return Burnthrough range (km) — signal range at which J/S = js_threshold
[[nodiscard]] Km burnthrough_range(
    Dbm    signal_erp,
    Dbm    jammer_erp,
    Km     jammer_to_rx_dist,
    Meters signal_tx_height,
    Meters jammer_height,
    Meters rx_height,
    Mhz    frequency,
    Db     js_threshold) noexcept;

// ---------------------------------------------------------------------------
// Partial-band / spot jamming
// ---------------------------------------------------------------------------

struct PartialBandResult {
    Mhz    optimum_jamming_bandwidth; ///< Optimum jamming bandwidth (MHz)
    double duty_cycle{0.0};           ///< Fraction of hop range covered
};

/// Compute optimum partial-band jamming parameters.
///
/// For a frequency-hopping target, jamming all hops with a narrow band increases
/// instantaneous J/S at the expense of hit probability. The optimum bandwidth
/// maximises the effective J/S averaged over the hopping band.
///
/// BW_opt = signal_bw * 10^(single_channel_js / 10)
/// The result is capped only by hop_range_bandwidth; for single-channel J/S
/// >= 0 dB, surplus jammer power widens the jammed bandwidth beyond the
/// signal bandwidth to cover more hops.
///
/// @param signal_bandwidth     Target signal occupied bandwidth (kHz → stored as MHz)
/// @param hop_range_bandwidth  Total frequency hopping range (MHz)
/// @param single_channel_js    J/S achievable against a single non-hopping channel (dB)
/// @return PartialBandResult with optimum bandwidth and duty cycle
[[nodiscard]] PartialBandResult partial_band_jamming(
    Mhz signal_bandwidth,
    Mhz hop_range_bandwidth,
    Db  single_channel_js) noexcept;

// ---------------------------------------------------------------------------
// Radar jamming — self-protection (SPJ) and stand-off (SOJ)
// ---------------------------------------------------------------------------
//
// The radar's skin return travels two ways (R⁴) while the jamming travels one
// way (R²), so J/S at the radar receiver grows 20 dB per range decade for a
// self-protection jammer. Both forms assume the jammer's ERP falls entirely
// within the radar's receiver bandwidth; frequency cancels between the two
// paths. The constant is 10·log₁₀(4π·10⁶) = 70.99 dB for R in km and σ in m²
// (constants::radar_js_constant_db; Adamy rounds it to 71).
//
// Source: Adamy EW101 Sec 9.2.3 (J/S ratio). Burnthrough: stand-off at
// Sec 9.3.4 (constant as −71), self-protection at Sec 9.3.5.

/// Self-protection jamming J/S at the radar receiver.
///
/// J/S = ERP_J − ERP_S + 70.99 + 20·log₁₀ R(km) − σ(dBsm)
///
/// @param radar_erp  Radar ERP toward the target (dBm)
/// @param jammer_erp Jammer ERP toward the radar (dBm)
/// @param range      Radar-to-target range; the jammer rides on the target (km)
/// @param rcs        Target radar cross-section (dBsm)
/// @return J/S at the radar receiver (dB)
[[nodiscard]] Db self_protection_js(
    Dbm  radar_erp,
    Dbm  jammer_erp,
    Km   range,
    Dbsm rcs) noexcept;

/// Self-protection burnthrough range — the range at which J/S falls to the
/// threshold. Inside it the radar sees the target through the jamming.
///
/// R = 10^((J/S_thr − (ERP_J − ERP_S + 70.99 − σ)) / 20)  km
[[nodiscard]] Km self_protection_burnthrough(
    Dbm  radar_erp,
    Dbm  jammer_erp,
    Dbsm rcs,
    Db   js_threshold) noexcept;

/// Stand-off jamming J/S at the radar receiver.
///
/// J/S = ERP_J − ERP_S + G_S − G_M + 70.99 + 40·log₁₀ R_T − 20·log₁₀ R_J − σ
///
/// @param radar_erp     Radar ERP toward the target, mainbeam (dBm)
/// @param jammer_erp    Jammer ERP toward the radar (dBm)
/// @param target_range  Radar-to-target range R_T (km)
/// @param jammer_range  Radar-to-jammer range R_J (km)
/// @param mainbeam_gain Radar receive gain toward the target, G_M (dB)
/// @param sidelobe_gain Radar receive gain toward the jammer, G_S (dB)
/// @param rcs           Target radar cross-section (dBsm)
/// @return J/S at the radar receiver (dB)
[[nodiscard]] Db stand_off_js(
    Dbm  radar_erp,
    Dbm  jammer_erp,
    Km   target_range,
    Km   jammer_range,
    Db   mainbeam_gain,
    Db   sidelobe_gain,
    Dbsm rcs) noexcept;

/// Stand-off burnthrough range — the radar-to-target range at which J/S falls
/// to the threshold, with the jammer held at jammer_range.
///
/// R_T = 10^((J/S_thr − (ERP_J − ERP_S + G_S − G_M + 70.99 − 20·log₁₀ R_J − σ)) / 40)  km
[[nodiscard]] Km stand_off_burnthrough(
    Dbm  radar_erp,
    Dbm  jammer_erp,
    Km   jammer_range,
    Db   mainbeam_gain,
    Db   sidelobe_gain,
    Dbsm rcs,
    Db   js_threshold) noexcept;

} // namespace libew::jamming
