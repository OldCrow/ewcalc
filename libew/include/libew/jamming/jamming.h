#pragma once

/// @file jamming.h
/// @brief Jamming effectiveness analysis: J/S, burnthrough range, partial-band jamming.

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
/// At burnthrough, the signal propagation loss (as a function of range) equals the
/// jammer advantage. Solved analytically for the LOS regime; iteratively for 2-ray.
///
/// @param signal_erp       ERP of desired signal transmitter (dBm)
/// @param jammer_erp       ERP of jammer (dBm)
/// @param jammer_to_rx_dist Distance from jammer to receiver (km)
/// @param jammer_height    Jammer antenna height (m)
/// @param rx_height        Receiver antenna height (m)
/// @param frequency        Carrier frequency (MHz)
/// @param js_threshold     J/S level at which jamming is considered effective (dB)
/// @return Burnthrough range (km) — signal range at which J/S = js_threshold
[[nodiscard]] Km burnthrough_range(
    Dbm    signal_erp,
    Dbm    jammer_erp,
    Km     jammer_to_rx_dist,
    Meters jammer_height,
    Meters rx_height,
    Mhz    frequency,
    Db     js_threshold) noexcept;

// ---------------------------------------------------------------------------
// Partial-band / spot jamming
// ---------------------------------------------------------------------------

struct PartialBandResult {
    Mhz    optimum_jamming_bandwidth; ///< Optimum jamming bandwidth (MHz)
    double duty_cycle;                ///< Fraction of hop range covered
};

/// Compute optimum partial-band jamming parameters.
///
/// For a frequency-hopping target, jamming all hops with a narrow band increases
/// instantaneous J/S at the expense of hit probability. The optimum bandwidth
/// maximises the effective J/S averaged over the hopping band.
///
/// Optimum BW = signal_bandwidth (when single-channel J/S >= 0 dB, all hops covered)
/// Otherwise:  BW_opt = signal_bw * 10^(single_channel_js / 10)
///
/// @param signal_bandwidth     Target signal occupied bandwidth (kHz → stored as MHz)
/// @param hop_range_bandwidth  Total frequency hopping range (MHz)
/// @param single_channel_js    J/S achievable against a single non-hopping channel (dB)
/// @return PartialBandResult with optimum bandwidth and duty cycle
[[nodiscard]] PartialBandResult partial_band_jamming(
    Mhz signal_bandwidth,
    Mhz hop_range_bandwidth,
    Db  single_channel_js) noexcept;

} // namespace libew::jamming
