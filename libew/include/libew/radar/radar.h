#pragma once

/// @file radar.h
/// @brief Radar range equation, pulse compression, and coherent integration gain.

#include "libew/core/units.h"

namespace libew::radar {

using namespace libew::units;

// ---------------------------------------------------------------------------
// Radar range equation
// ---------------------------------------------------------------------------

struct RadarRangeResult {
    Km max_range;    ///< Maximum detection range (km)
    Db two_way_loss; ///< Total two-way propagation loss at max range (dB)
};

/// Radar range equation (one-way link budget form, applied twice).
///
/// The radar range equation in log domain:
///   R_max^4 = P_t * G^2 * λ^2 * σ / ((4π)^3 * k*T*B*NF*SNR_min * L_sys)
///
/// Rearranged as a link budget:
///   20*log10(R_max) = (P_t_dBm + 2*G_dBi + 20*log10(λ) + σ_dBsm
///                      - 30*log10(4π) - noise_power_dBm - SNR_dB - L_sys_dB) / 4
/// where λ = c / f_MHz / 1e6 (meters) and noise_power = sensitivity - SNR
///
/// @param tx_power     Transmitter power (dBm)
/// @param antenna_gain Antenna gain (dBi, same antenna used for Tx and Rx)
/// @param target_rcs   Target radar cross-section (dBsm)
/// @param frequency    Radar carrier frequency (MHz)
/// @param system_losses Combined system losses (dB): feed, atmospheric, etc.
/// @param noise_figure Receiver noise figure (dB)
/// @param bandwidth    Receiver noise bandwidth (MHz)
/// @param required_snr Minimum required SNR for detection (dB)
/// @return RadarRangeResult with max range and two-way path loss
[[nodiscard]] RadarRangeResult radar_range(
    Dbm  tx_power,
    Db   antenna_gain,
    Dbsm target_rcs,
    Mhz  frequency,
    Db   system_losses,
    Db   noise_figure,
    Mhz  bandwidth,
    Db   required_snr) noexcept;

// ---------------------------------------------------------------------------
// Signal processing gain
// ---------------------------------------------------------------------------

/// Pulse compression gain.
/// G_pc = 10*log10(time_bandwidth_product)
/// TB product = pulse width (s) × bandwidth (Hz) — must be > 1 for gain.
/// @param time_bandwidth_product  Pulse width × bandwidth (dimensionless)
/// @return Pulse compression gain (dB)
[[nodiscard]] Db pulse_compression_gain(double time_bandwidth_product) noexcept;

/// Coherent integration gain.
/// G_int = 10*log10(num_pulses)
/// @param num_pulses  Number of coherently integrated pulses
/// @return Coherent integration gain (dB)
[[nodiscard]] Db coherent_integration_gain(int num_pulses) noexcept;

/// Wavelength in meters from frequency in MHz (convenience function).
[[nodiscard]] Meters wavelength_m(Mhz frequency) noexcept;

} // namespace libew::radar
