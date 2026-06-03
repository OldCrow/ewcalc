#pragma once

/// @file location.h
/// @brief Emitter location accuracy calculations: CEP from AOA, TDOA, and EEP.

#include "libew/core/units.h"

namespace libew::location {

using namespace libew::units;

/// CEP from RMS bearing error assuming ideal geometry
/// (two receivers 90° apart, equidistant from emitter).
/// CEP ≈ 1.2 * range * tan(rms_error_rad)
///
/// @param rms_bearing_error  RMS bearing error (degrees)
/// @param range_to_emitter   Range from each receiver to emitter (km)
/// @return CEP (km)
[[nodiscard]] Km cep_from_rms_bearing_error(Degrees rms_bearing_error, Km range_to_emitter) noexcept;

/// CEP from TDOA RMS timing error.
///
/// For two receivers with baseline separation B and an emitter at range R,
/// the position uncertainty in the cross-baseline direction (ideal perpendicular
/// geometry, θ = 90° from baseline) is:
///
///   CEP_TDOA ≈ c·σ_t·R / (2·B)
///
/// The factor c·σ_t gives the range-difference uncertainty; R/B is the
/// geometric dilution from range-difference to position.
/// Source: Adamy EW101 emitter location chapter.
///
/// @param rms_time_error_ns  RMS TDOA timing error (nanoseconds)
/// @param range_to_emitter   Slant range from receiver midpoint to emitter (km)
/// @param baseline           Receiver separation distance (km)
/// @return CEP (km)
[[nodiscard]] Km cep_from_tdoa_rms_error(double rms_time_error_ns,
                                          Km range_to_emitter,
                                          Km baseline) noexcept;

/// CEP from an Elliptical Error Probable (EEP) — exact for the symmetric case.
/// For an ellipse with semi-axes a ≥ b:
///   CEP ≈ 0.59 * (a + b)
/// This approximation holds to within ~1% for axis ratios up to 4:1.
///
/// @param semi_major  Semi-major axis of the EEP ellipse (km)
/// @param semi_minor  Semi-minor axis of the EEP ellipse (km)
/// @return CEP (km)
[[nodiscard]] Km cep_from_eep(Km semi_major, Km semi_minor) noexcept;

} // namespace libew::location
