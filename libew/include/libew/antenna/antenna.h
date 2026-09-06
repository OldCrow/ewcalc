#pragma once

/// @file antenna.h
/// @brief Antenna gain and ERP calculations.
///
/// Sources: ERP — Adamy EW103 Sec 5.2 p.120, Fig 5.4 p.121. dBi/dBd
/// (2.15 dB = half-wave dipole directivity) — IEEE Std 145; not treated in
/// Adamy EW101-EW103. Gain <-> beamwidth — Kraus/Tai-Pereira-family rule of
/// thumb, 30000 variant (Adamy EW103 Sec 3.7 p.70 uses 29000; 0.15 dB
/// documented delta). Full pins and deltas: docs/formulas.md.

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

/// Approximate circular 3 dB beamwidth from antenna gain.
/// Inverts the symmetric case of gain_from_beamwidth():
/// θ_3dB ≈ sqrt(30000 / 10^(G_dBi / 10)) degrees.
/// Note: valid for directional antennas where the derived beamwidth is <= 360°
/// (gain >= about -6.35 dBi with this approximation).
[[nodiscard]] Degrees beamwidth_from_gain(Db gain_dbi) noexcept;

/// Approximate antenna gain from 3 dB beamwidth (azimuth × elevation).
/// G ≈ 10*log10(30000 / (θ_az_deg * θ_el_deg))
[[nodiscard]] Db gain_from_beamwidth(Degrees az_beamwidth, Degrees el_beamwidth) noexcept;

/// Wavelength in meters from frequency in MHz.
[[nodiscard]] Meters wavelength(Mhz frequency) noexcept;

} // namespace libew::antenna
