#pragma once

/// @file antenna.h
/// @brief Antenna gain and ERP calculations.

#include "libew/core/units.h"

namespace libew::antenna {

using namespace libew::units;

/// Effective Radiated Power from transmit power and antenna gain.
[[nodiscard]] constexpr Dbm erp(Dbm tx_power, Db antenna_gain) noexcept {
    return tx_power + antenna_gain;
}
[[nodiscard]] constexpr Dbw erp(Dbw tx_power, Db antenna_gain) noexcept {
    return tx_power + antenna_gain;
}

/// Convert isotropic gain (dBi) to dipole-referenced gain (dBd).
/// dBd = dBi - 2.15
[[nodiscard]] constexpr Db dbi_to_dbd(Db dbi) noexcept { return Db{dbi.value - 2.15}; }

/// Convert dipole-referenced gain (dBd) to isotropic gain (dBi).
/// dBi = dBd + 2.15
[[nodiscard]] constexpr Db dbd_to_dbi(Db dbd) noexcept { return Db{dbd.value + 2.15}; }

/// Approximate 3 dB beamwidth from antenna gain (Tai & Pereira approximation).
/// θ_3dB ≈ 10^((11.1 - G_dBi) / 20)  degrees (spherical cap approximation)
/// Note: valid for directional antennas with moderate to high gain.
[[nodiscard]] Degrees beamwidth_from_gain(Db gain_dbi) noexcept;

/// Approximate antenna gain from 3 dB beamwidth (azimuth × elevation).
/// G ≈ 10*log10(30000 / (θ_az_deg * θ_el_deg))
[[nodiscard]] Db gain_from_beamwidth(Degrees az_beamwidth, Degrees el_beamwidth) noexcept;

/// Wavelength in meters from frequency in MHz.
[[nodiscard]] Meters wavelength(Mhz frequency) noexcept;

} // namespace libew::antenna
