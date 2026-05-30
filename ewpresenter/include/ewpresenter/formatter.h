#pragma once

/// @file formatter.h
/// @brief Formatted string output for EW quantities.
///
/// Each function returns a std::string with the value and appropriate unit label.
/// These strings are what the view layer displays directly — no formatting
/// logic should appear in the platform view code.

#include "libew/core/units.h"
#include <string>

namespace ewpresenter {

using namespace libew::units;

// ---------------------------------------------------------------------------
// Power
// ---------------------------------------------------------------------------
[[nodiscard]] std::string format_dbm(Dbm value, int decimals = 1);   // "-120.5 dBm"
[[nodiscard]] std::string format_dbw(Dbw value, int decimals = 1);   // "-150.5 dBW"
[[nodiscard]] std::string format_watts(Watts value);                  // "1.000 W" or "1.000 mW"

// ---------------------------------------------------------------------------
// Gain / loss / ratio
// ---------------------------------------------------------------------------
[[nodiscard]] std::string format_db(Db value, int decimals = 1);      // "32.4 dB"
[[nodiscard]] std::string format_dbsm(Dbsm value, int decimals = 1);  // "0.0 dBsm"

// ---------------------------------------------------------------------------
// Frequency
// ---------------------------------------------------------------------------
[[nodiscard]] std::string format_mhz(Mhz value, int decimals = 1);   // "42.5 MHz"
[[nodiscard]] std::string format_ghz(Ghz value, int decimals = 3);   // "3.000 GHz"

// ---------------------------------------------------------------------------
// Distance
// ---------------------------------------------------------------------------
[[nodiscard]] std::string format_km(Km value, int decimals = 3);      // "32.546 km"
[[nodiscard]] std::string format_m(Meters value, int decimals = 1);   // "10.0 m"

// ---------------------------------------------------------------------------
// Angle
// ---------------------------------------------------------------------------
[[nodiscard]] std::string format_degrees(Degrees value, int decimals = 2); // "1.00°"

// ---------------------------------------------------------------------------
// Dimensionless / ratio
// ---------------------------------------------------------------------------
[[nodiscard]] std::string format_percent(double fraction, int decimals = 3); // "0.043%"

// ---------------------------------------------------------------------------
// Boolean / status
// ---------------------------------------------------------------------------
[[nodiscard]] std::string format_regime(bool two_ray) noexcept; // "2-ray" or "LOS"

} // namespace ewpresenter
