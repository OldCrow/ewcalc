#pragma once

/// @file propagation.h
/// @brief RF propagation path loss models.
///
/// Implements FSPL, 2-ray ground reflection, and knife-edge diffraction.
/// Regime selection (LOS vs. 2-ray) is based on Fresnel zone crossover distance.

#include "libew/core/units.h"

namespace libew::propagation {

using namespace libew::units;

// ---------------------------------------------------------------------------
// Free-space path loss (LOS / Friis)
// ---------------------------------------------------------------------------

/// Free-space path loss.
/// Formula: L = 32.44 + 20*log10(d_km) + 20*log10(f_MHz)
/// @param distance  Link distance (km)
/// @param frequency Carrier frequency (MHz)
/// @return Path loss (dB, positive value)
[[nodiscard]] Db free_space_path_loss(Km distance, Mhz frequency) noexcept;

// ---------------------------------------------------------------------------
// 2-ray ground reflection model
// ---------------------------------------------------------------------------

/// 2-ray ground reflection path loss.
/// Formula: L = 120 + 40*log10(d_km) - 20*log10(h_tx_m) - 20*log10(h_rx_m)
/// Applies when the link distance exceeds the Fresnel zone crossover distance.
/// @param distance   Link distance (km)
/// @param tx_height  Transmit antenna height above ground (m)
/// @param rx_height  Receive antenna height above ground (m)
/// @return Path loss (dB, positive value)
[[nodiscard]] Db two_ray_path_loss(Km distance, Meters tx_height, Meters rx_height) noexcept;

// ---------------------------------------------------------------------------
// Fresnel zone crossover distance
// ---------------------------------------------------------------------------

/// Fresnel zone crossover distance — the range at which 2-ray propagation
/// begins to dominate over free-space LOS propagation.
/// Formula: d_FZ = h_tx * h_rx * f_MHz / 24000   (result in km)
/// @param tx_height  Transmit antenna height (m)
/// @param rx_height  Receive antenna height (m)
/// @param frequency  Carrier frequency (MHz)
/// @return Crossover distance (km)
[[nodiscard]] Km fresnel_zone_distance(Meters tx_height, Meters rx_height, Mhz frequency) noexcept;

// ---------------------------------------------------------------------------
// Regime selection
// ---------------------------------------------------------------------------

/// Returns true if 2-ray propagation applies (distance > Fresnel zone crossover).
[[nodiscard]] bool is_two_ray_regime(Km distance, Meters tx_height, Meters rx_height, Mhz frequency) noexcept;

/// Compute path loss using whichever regime applies at the given distance.
/// Uses free_space_path_loss for d < d_FZ, two_ray_path_loss otherwise.
[[nodiscard]] Db path_loss(Km distance, Meters tx_height, Meters rx_height, Mhz frequency) noexcept;

// ---------------------------------------------------------------------------
// Knife-edge diffraction
// ---------------------------------------------------------------------------

/// Knife-edge diffraction loss using the Fresnel diffraction parameter.
///
/// The diffraction parameter v is computed from geometry then mapped to gain Gd:
///   v < -1       → Gd = 0   (full illumination, no loss)
///   -1 ≤ v < 0  → Gd = 20*log10(0.5 - 0.62*v)
///   0  ≤ v < 1  → Gd = 20*log10(0.5 * exp(-0.95*v))
///   1  ≤ v < 2.4 → Gd = 20*log10(0.4 - sqrt(0.1184 - (0.38 - 0.1*v)^2))
///   v  ≥ 2.4    → Gd = 20*log10(0.225 / v)
///   loss = -Gd
///
/// @param dist_tx_edge  Distance from transmitter to knife edge (km)
/// @param dist_edge_rx  Distance from knife edge to receiver (km)
/// @param los_clearance Height of line-of-sight above knife edge (m); negative = edge above LOS
/// @param frequency     Carrier frequency (MHz)
/// @return Diffraction loss (dB, positive = additional loss)
[[nodiscard]] Db knife_edge_diffraction_loss(
    Km    dist_tx_edge,
    Km    dist_edge_rx,
    Meters los_clearance,
    Mhz   frequency) noexcept;

} // namespace libew::propagation
