#pragma once

/// @file link.h
/// @brief One-way link budget and effective range calculations.

#include "libew/core/units.h"

namespace libew::link {

using namespace libew::units;

// ---------------------------------------------------------------------------
// One-way link budget result
// ---------------------------------------------------------------------------

struct OnewayLinkResult {
    Dbm received_power;        ///< Received signal power at receiver input (dBm)
    Db  path_loss;             ///< Total propagation path loss (dB, positive)
    Km  fresnel_zone_distance; ///< Fresnel zone crossover distance (km)
    bool two_ray_regime;       ///< True if 2-ray propagation applies at this range
};

/// Compute received power for a one-way RF link.
///
/// Received power = tx_power + tx_gain + rx_gain - path_loss
/// Path loss model is selected automatically (LOS or 2-ray) based on geometry.
///
/// @param tx_power       Transmitter output power (dBm)
/// @param tx_gain        Transmit antenna gain toward receiver (dB)
/// @param rx_gain        Receive antenna gain toward transmitter (dB)
/// @param distance       Link distance (km)
/// @param tx_height      Transmit antenna height above ground (m)
/// @param rx_height      Receive antenna height above ground (m)
/// @param frequency      Carrier frequency (MHz)
/// @return OnewayLinkResult with received power, loss, Fresnel distance, and regime flag
[[nodiscard]] OnewayLinkResult one_way_link(
    Dbm    tx_power,
    Db     tx_gain,
    Db     rx_gain,
    Km     distance,
    Meters tx_height,
    Meters rx_height,
    Mhz    frequency) noexcept;

// ---------------------------------------------------------------------------
// Effective range
// ---------------------------------------------------------------------------

struct EffectiveRangeResult {
    Km   range;          ///< Maximum range at which received power ≥ sensitivity (km)
    bool two_ray_regime; ///< True if the effective range falls in the 2-ray regime
};

/// Compute effective communication range — the maximum distance at which the
/// received power equals the receiver sensitivity.
///
/// If LOS range exceeds the Fresnel zone crossover, 2-ray propagation limits the
/// effective range (which will be shorter than the LOS result).
///
/// @param tx_power      Transmitter output power (dBm)
/// @param tx_gain       Transmit antenna gain (dB)
/// @param rx_gain       Receive antenna gain (dB)
/// @param tx_height     Transmit antenna height (m)
/// @param rx_height     Receive antenna height (m)
/// @param frequency     Carrier frequency (MHz)
/// @param rx_sensitivity Receiver system sensitivity (dBm)
/// @return EffectiveRangeResult with range and regime flag
[[nodiscard]] EffectiveRangeResult effective_range(
    Dbm    tx_power,
    Db     tx_gain,
    Db     rx_gain,
    Meters tx_height,
    Meters rx_height,
    Mhz    frequency,
    Dbm    rx_sensitivity) noexcept;

} // namespace libew::link
