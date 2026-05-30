#pragma once

/// @file units.h
/// @brief Strong-type wrappers for RF and EW quantities.
///
/// Prevents silent mixing of incompatible log-domain quantities.
/// Only arithmetic operations that are physically meaningful are defined.
///
/// Valid power-domain algebra:
///   Dbm + Db  -> Dbm    (apply gain/loss to power)
///   Dbm - Db  -> Dbm
///   Dbm - Dbm -> Db     (ratio of two power levels)
///   Db  + Db  -> Db     (combine gains)
///   Db  - Db  -> Db
///   -Db       -> Db     (negate gain = loss)
///   (same rules apply for Dbw)
///
/// Invalid operations (compile errors):
///   Dbm + Dbm           (sum of two absolute power levels is meaningless)
///   Dbm + Dbw           (incompatible references)
///   Km  + Meters        (incompatible distance units — convert first)

#include <compare>

namespace libew::units {

/// Primary strong-type template. Each quantity type is a distinct tag instantiation.
template<typename Tag>
struct Quantity {
    double value{};

    constexpr Quantity() noexcept = default;
    explicit constexpr Quantity(double v) noexcept : value{v} {}

    [[nodiscard]] constexpr bool operator==(const Quantity&) const noexcept = default;
    [[nodiscard]] constexpr auto operator<=>(const Quantity&) const noexcept = default;
};

// ---------------------------------------------------------------------------
// Tag types — one per distinct quantity kind
// ---------------------------------------------------------------------------
struct DbmTag  {};   ///< Power relative to 1 mW (dBm)
struct DbwTag  {};   ///< Power relative to 1 W  (dBW)
struct DbTag   {};   ///< Dimensionless ratio: gain, loss, SNR, NF (dB)
struct DbsmTag {};   ///< Radar cross-section relative to 1 m² (dBsm)
struct WattsTag{};   ///< Linear power (W)
struct MhzTag  {};   ///< Frequency (MHz)
struct GhzTag  {};   ///< Frequency (GHz)
struct KmTag   {};   ///< Distance (km)
struct MetersTag{};  ///< Distance (m)
struct DegreesTag{}; ///< Angle (degrees)
struct RadiansTag{}; ///< Angle (radians)
struct SquareMetersTag{}; ///< Area — linear RCS (m²)

// ---------------------------------------------------------------------------
// Named quantity types
// ---------------------------------------------------------------------------
using Dbm         = Quantity<DbmTag>;
using Dbw         = Quantity<DbwTag>;
using Db          = Quantity<DbTag>;
using Dbsm        = Quantity<DbsmTag>;
using Watts       = Quantity<WattsTag>;
using Mhz         = Quantity<MhzTag>;
using Ghz         = Quantity<GhzTag>;
using Km          = Quantity<KmTag>;
using Meters      = Quantity<MetersTag>;
using Degrees     = Quantity<DegreesTag>;
using Radians     = Quantity<RadiansTag>;
using SquareMeters = Quantity<SquareMetersTag>;

// ---------------------------------------------------------------------------
// Power-domain arithmetic
// ---------------------------------------------------------------------------

// Dbm ± Db → Dbm
[[nodiscard]] constexpr Dbm operator+(Dbm p, Db g) noexcept { return Dbm{p.value + g.value}; }
[[nodiscard]] constexpr Dbm operator+(Db g, Dbm p) noexcept { return Dbm{p.value + g.value}; }
[[nodiscard]] constexpr Dbm operator-(Dbm p, Db l) noexcept { return Dbm{p.value - l.value}; }
[[nodiscard]] constexpr Db  operator-(Dbm a, Dbm b) noexcept { return Db{a.value - b.value}; }

// Dbw ± Db → Dbw
[[nodiscard]] constexpr Dbw operator+(Dbw p, Db g) noexcept { return Dbw{p.value + g.value}; }
[[nodiscard]] constexpr Dbw operator+(Db g, Dbw p) noexcept { return Dbw{p.value + g.value}; }
[[nodiscard]] constexpr Dbw operator-(Dbw p, Db l) noexcept { return Dbw{p.value - l.value}; }
[[nodiscard]] constexpr Db  operator-(Dbw a, Dbw b) noexcept { return Db{a.value - b.value}; }

// Dbsm ± Db → Dbsm  (e.g. fluctuation loss applied to RCS)
[[nodiscard]] constexpr Dbsm operator+(Dbsm r, Db d) noexcept { return Dbsm{r.value + d.value}; }
[[nodiscard]] constexpr Dbsm operator-(Dbsm r, Db d) noexcept { return Dbsm{r.value - d.value}; }

// Gain arithmetic: Db ± Db → Db
[[nodiscard]] constexpr Db operator+(Db a, Db b) noexcept { return Db{a.value + b.value}; }
[[nodiscard]] constexpr Db operator-(Db a, Db b) noexcept { return Db{a.value - b.value}; }
[[nodiscard]] constexpr Db operator-(Db g) noexcept       { return Db{-g.value}; }

// ---------------------------------------------------------------------------
// Unit conversions
// ---------------------------------------------------------------------------

// Power: dBm ↔ dBW (differ by 30 dB)
[[nodiscard]] constexpr Dbw dbm_to_dbw(Dbm p) noexcept { return Dbw{p.value - 30.0}; }
[[nodiscard]] constexpr Dbm dbw_to_dbm(Dbw p) noexcept { return Dbm{p.value + 30.0}; }

// Power: dBm/dBW ↔ linear Watts
[[nodiscard]] inline Watts dbm_to_watts(Dbm p) noexcept;
[[nodiscard]] inline Dbm   watts_to_dbm(Watts w) noexcept;
[[nodiscard]] inline Watts dbw_to_watts(Dbw p) noexcept;
[[nodiscard]] inline Dbw   watts_to_dbw(Watts w) noexcept;

// Gain: dB ↔ linear ratio
[[nodiscard]] inline double db_to_linear(Db g) noexcept;
[[nodiscard]] inline Db     linear_to_db(double ratio) noexcept;

// RCS: dBsm ↔ m²
[[nodiscard]] inline SquareMeters dbsm_to_sqm(Dbsm r) noexcept;
[[nodiscard]] inline Dbsm         sqm_to_dbsm(SquareMeters r) noexcept;

// Frequency: MHz ↔ GHz
[[nodiscard]] constexpr Ghz mhz_to_ghz(Mhz f) noexcept { return Ghz{f.value / 1000.0}; }
[[nodiscard]] constexpr Mhz ghz_to_mhz(Ghz f) noexcept { return Mhz{f.value * 1000.0}; }

// Distance: km ↔ m
[[nodiscard]] constexpr Meters km_to_m(Km d)   noexcept { return Meters{d.value * 1000.0}; }
[[nodiscard]] constexpr Km     m_to_km(Meters d) noexcept { return Km{d.value / 1000.0}; }

// Angle: degrees ↔ radians
[[nodiscard]] inline Radians deg_to_rad(Degrees a) noexcept;
[[nodiscard]] inline Degrees rad_to_deg(Radians a) noexcept;

// ---------------------------------------------------------------------------
// User-defined literals
// ---------------------------------------------------------------------------
namespace literals {
    [[nodiscard]] constexpr Dbm  operator""_dBm (long double v) noexcept { return Dbm {static_cast<double>(v)}; }
    [[nodiscard]] constexpr Dbm  operator""_dBm (unsigned long long v) noexcept { return Dbm {static_cast<double>(v)}; }
    [[nodiscard]] constexpr Dbw  operator""_dBw (long double v) noexcept { return Dbw {static_cast<double>(v)}; }
    [[nodiscard]] constexpr Dbw  operator""_dBw (unsigned long long v) noexcept { return Dbw {static_cast<double>(v)}; }
    [[nodiscard]] constexpr Db   operator""_dB  (long double v) noexcept { return Db  {static_cast<double>(v)}; }
    [[nodiscard]] constexpr Db   operator""_dB  (unsigned long long v) noexcept { return Db  {static_cast<double>(v)}; }
    [[nodiscard]] constexpr Dbsm operator""_dBsm(long double v) noexcept { return Dbsm{static_cast<double>(v)}; }
    [[nodiscard]] constexpr Dbsm operator""_dBsm(unsigned long long v) noexcept { return Dbsm{static_cast<double>(v)}; }
    [[nodiscard]] constexpr Mhz  operator""_MHz (long double v) noexcept { return Mhz {static_cast<double>(v)}; }
    [[nodiscard]] constexpr Mhz  operator""_MHz (unsigned long long v) noexcept { return Mhz {static_cast<double>(v)}; }
    [[nodiscard]] constexpr Ghz  operator""_GHz (long double v) noexcept { return Ghz {static_cast<double>(v)}; }
    [[nodiscard]] constexpr Ghz  operator""_GHz (unsigned long long v) noexcept { return Ghz {static_cast<double>(v)}; }
    [[nodiscard]] constexpr Km   operator""_km  (long double v) noexcept { return Km  {static_cast<double>(v)}; }
    [[nodiscard]] constexpr Km   operator""_km  (unsigned long long v) noexcept { return Km  {static_cast<double>(v)}; }
    [[nodiscard]] constexpr Meters operator""_m (long double v) noexcept { return Meters{static_cast<double>(v)}; }
    [[nodiscard]] constexpr Meters operator""_m (unsigned long long v) noexcept { return Meters{static_cast<double>(v)}; }
    [[nodiscard]] constexpr Degrees operator""_deg(long double v) noexcept { return Degrees{static_cast<double>(v)}; }
    [[nodiscard]] constexpr Degrees operator""_deg(unsigned long long v) noexcept { return Degrees{static_cast<double>(v)}; }
    [[nodiscard]] constexpr Watts operator""_W  (long double v) noexcept { return Watts{static_cast<double>(v)}; }
    [[nodiscard]] constexpr Watts operator""_W  (unsigned long long v) noexcept { return Watts{static_cast<double>(v)}; }
} // namespace literals

} // namespace libew::units

// ---------------------------------------------------------------------------
// Inline definitions requiring <cmath>
// ---------------------------------------------------------------------------
#include <cmath>
#include <numbers>

namespace libew::units {

[[nodiscard]] inline Watts dbm_to_watts(Dbm p) noexcept {
    return Watts{std::pow(10.0, (p.value - 30.0) / 10.0)};
}
[[nodiscard]] inline Dbm watts_to_dbm(Watts w) noexcept {
    return Dbm{10.0 * std::log10(w.value) + 30.0};
}
[[nodiscard]] inline Watts dbw_to_watts(Dbw p) noexcept {
    return Watts{std::pow(10.0, p.value / 10.0)};
}
[[nodiscard]] inline Dbw watts_to_dbw(Watts w) noexcept {
    return Dbw{10.0 * std::log10(w.value)};
}
[[nodiscard]] inline double db_to_linear(Db g) noexcept {
    return std::pow(10.0, g.value / 10.0);
}
[[nodiscard]] inline Db linear_to_db(double ratio) noexcept {
    return Db{10.0 * std::log10(ratio)};
}
[[nodiscard]] inline SquareMeters dbsm_to_sqm(Dbsm r) noexcept {
    return SquareMeters{std::pow(10.0, r.value / 10.0)};
}
[[nodiscard]] inline Dbsm sqm_to_dbsm(SquareMeters r) noexcept {
    return Dbsm{10.0 * std::log10(r.value)};
}
[[nodiscard]] inline Radians deg_to_rad(Degrees a) noexcept {
    return Radians{a.value * std::numbers::pi / 180.0};
}
[[nodiscard]] inline Degrees rad_to_deg(Radians a) noexcept {
    return Degrees{a.value * 180.0 / std::numbers::pi};
}

} // namespace libew::units
