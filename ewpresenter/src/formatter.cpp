#include "ewpresenter/formatter.h"
#include <cstdio>
#include <cmath>

namespace ewpresenter {

namespace {
    // Format a double with a given number of decimal places and a unit label.
    std::string fmt(double value, int decimals, const char* unit) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.*f %s",
                      decimals, value, unit);
        return buf;
    }
} // namespace

std::string format_dbm(Dbm value, int decimals) {
    return fmt(value.value, decimals, "dBm");
}

std::string format_dbw(Dbw value, int decimals) {
    return fmt(value.value, decimals, "dBW");
}

static constexpr double kWattsDisplayThreshold = 1.0;    ///< Display in mW below this.
static constexpr double kMilliwattsPerWatt      = 1000.0; ///< Scale factor mW → W.
static constexpr double kPercentMultiplier      = 100.0;  ///< Fraction → percent.

std::string format_watts(Watts value) {
    // Auto-scale: display in mW if < 1 W
    if (std::abs(value.value) < kWattsDisplayThreshold && value.value != 0.0) {
        return fmt(value.value * kMilliwattsPerWatt, 3, "mW");
    }
    return fmt(value.value, 3, "W");
}

std::string format_db(Db value, int decimals) {
    return fmt(value.value, decimals, "dB");
}

std::string format_dbsm(Dbsm value, int decimals) {
    return fmt(value.value, decimals, "dBsm");
}

std::string format_mhz(Mhz value, int decimals) {
    return fmt(value.value, decimals, "MHz");
}

std::string format_ghz(Ghz value, int decimals) {
    return fmt(value.value, decimals, "GHz");
}

std::string format_km(Km value, int decimals) {
    return fmt(value.value, decimals, "km");
}

std::string format_m(Meters value, int decimals) {
    return fmt(value.value, decimals, "m");
}

std::string format_degrees(Degrees value, int decimals) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.*f\xc2\xb0",   // UTF-8 degree sign
                  decimals, value.value);
    return buf;
}

std::string format_percent(double fraction, int decimals) {
    return fmt(fraction * kPercentMultiplier, decimals, "%");
}

std::string format_regime(bool two_ray) noexcept {
    return two_ray ? "2-ray" : "LOS";
}

std::string format_kelvin(Kelvin value, int decimals) {
    return fmt(value.value, decimals, "K");
}

} // namespace ewpresenter
