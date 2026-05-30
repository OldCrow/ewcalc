#pragma once

/// @file constants.h
/// @brief Physical constants used throughout libew.

namespace libew::constants {

/// Speed of light (m/s)
inline constexpr double speed_of_light_m_s = 2.99792458e8;

/// Boltzmann's constant (J/K)
inline constexpr double boltzmann_k = 1.380649e-23;

/// Standard reference temperature per IEEE (K)  — 290 K = 16.85°C
inline constexpr double standard_temperature_K = 290.0;

/// Thermal noise power density at standard temperature: kT = -174 dBm/Hz
/// Derivation: 10*log10(1.380649e-23 * 290) + 30 = -173.977 dBm/Hz ≈ -174 dBm/Hz
inline constexpr double thermal_noise_floor_dbm_per_hz = -173.977;

/// Convenient approximation used widely in EW textbooks
inline constexpr double thermal_noise_floor_dbm_per_hz_approx = -174.0;

/// 4π (used in radar range and link budget equations)
inline constexpr double four_pi = 12.566370614359172;

/// (4π)^2 in dB — appears in free-space path loss derivation
inline constexpr double four_pi_squared_db = 21.984;

/// dBi to dBd conversion: isotropic antenna is 2.15 dB above half-wave dipole
inline constexpr double dbi_to_dbd_offset_db = 2.15;

/// Constant in the standard FSPL formula: 32.44 + 20*log10(km) + 20*log10(MHz)
/// Derivation: 20*log10(4π/c * 1e6 * 1e3) = 20*log10(4π * 1e9 / 2.998e8) ≈ 32.44
inline constexpr double fspl_constant_km_mhz = 32.44;

/// Constant in the 2-ray path loss formula: 120 + 40*log10(km) - 20*log10(h_tx) - 20*log10(h_rx)
inline constexpr double two_ray_constant_km = 120.0;

/// Fresnel zone crossover denominator constant: h_t * h_r * f_MHz / 24000
inline constexpr double fresnel_zone_denominator = 24000.0;

/// Digital receiver dynamic range per bit: approximately 6.02 dB/bit
inline constexpr double dynamic_range_db_per_bit = 6.02;

} // namespace libew::constants
