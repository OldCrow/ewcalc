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

/// CEP from TDOA RMS timing error assuming ideal geometry.
/// TDOA error in time converts to a range error via speed of light,
/// then treated as RMS range-difference error in a hyperbolic system.
/// CEP ≈ (c * rms_time_error) / 2
/// where the result is a position error (m), then converted to km.
///
/// @param rms_time_error_ns  RMS TDOA timing error (nanoseconds)
/// @param range_to_emitter   Range from receivers to emitter (km)
/// @return CEP (km)
[[nodiscard]] Km cep_from_tdoa_rms_error(double rms_time_error_ns, Km range_to_emitter) noexcept;

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
