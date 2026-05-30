#pragma once

/// @file receiver.h
/// @brief Receiver sensitivity, noise figure (Friis), and dynamic range.

#include "libew/core/units.h"
#include <span>

namespace libew::receiver {

using namespace libew::units;

// ---------------------------------------------------------------------------
// System sensitivity
// ---------------------------------------------------------------------------

/// Compute receiver system sensitivity.
/// Formula: S = -114 + 10*log10(BW_MHz) + NF + SNR_min   (result in dBm)
/// The -114 dBm/MHz baseline derives from kT at 290 K = -174 dBm/Hz = -114 dBm/MHz.
///
/// @param bandwidth     Receiver IF bandwidth (MHz)
/// @param noise_figure  System noise figure (dB)
/// @param required_snr  Minimum required pre-detection SNR (dB)
/// @return Minimum detectable signal level (dBm)
[[nodiscard]] Dbm system_sensitivity(Mhz bandwidth, Db noise_figure, Db required_snr) noexcept;

// ---------------------------------------------------------------------------
// Cascaded noise figure (Friis formula)
// ---------------------------------------------------------------------------

/// A single stage in a receiver chain: pre-amplifier, feed loss, mixer, etc.
struct Stage {
    Db noise_figure; ///< Stage noise figure (dB)
    Db gain;         ///< Stage gain (dB); negative = loss
};

/// Cascaded noise figure of two stages (Friis formula).
/// NF_sys = NF1 + (NF2 - 1) / G1   (linear domain, converted to dB)
[[nodiscard]] Db cascaded_noise_figure(Stage s1, Stage s2) noexcept;

/// Cascaded noise figure of an arbitrary chain of stages.
/// Applies the Friis formula sequentially from front to back.
/// @param stages  Ordered span from first (front-end) to last stage
[[nodiscard]] Db cascaded_noise_figure(std::span<const Stage> stages) noexcept;

// ---------------------------------------------------------------------------
// Dynamic range
// ---------------------------------------------------------------------------

/// Analog receiver second-order spurious-free dynamic range.
/// SFDR2 = (2/3) * (IP2 - sensitivity)
/// @param sensitivity       Receiver sensitivity (dBm)
/// @param second_order_ip   Second-order input intercept point (dBm)
/// @return Second-order SFDR (dB)
[[nodiscard]] Db analog_sfdr_second_order(Dbm sensitivity, Dbm second_order_ip) noexcept;

/// Analog receiver third-order spurious-free dynamic range.
/// SFDR3 = (2/3) * (IP3 - sensitivity)
/// @param sensitivity       Receiver sensitivity (dBm)
/// @param third_order_ip    Third-order input intercept point (dBm)
/// @return Third-order SFDR (dB)
[[nodiscard]] Db analog_sfdr_third_order(Dbm sensitivity, Dbm third_order_ip) noexcept;

/// Digital receiver dynamic range from ADC quantizer word length.
/// DR ≈ 6.02 * num_bits + 1.76   (dB)
/// @param num_bits  ADC quantizer word length (bits)
/// @return Dynamic range (dB)
[[nodiscard]] Db digital_dynamic_range(int num_bits) noexcept;

} // namespace libew::receiver
