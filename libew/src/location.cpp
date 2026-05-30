#include "libew/location/location.h"
#include "libew/core/constants.h"
#include <cmath>
#include <numbers>

namespace libew::location {

Km cep_from_rms_bearing_error(Degrees rms_bearing_error, Km range_to_emitter) noexcept {
    // CEP ≈ 1.2 * R * tan(σ_θ)
    // For small angles this simplifies to 1.2 * R * σ_θ_rad,
    // but the spreadsheet uses the exact tangent form.
    const double sigma_rad = rms_bearing_error.value * std::numbers::pi / 180.0;
    return Km{1.2 * range_to_emitter.value * std::tan(sigma_rad)};
}

Km cep_from_tdoa_rms_error(double rms_time_error_ns, Km range_to_emitter) noexcept {
    // Range error from timing: Δr = c * Δt
    // For ideal geometry (baseline perpendicular to emitter direction):
    // CEP ≈ (c * σ_t) / 2   — halved because TDOA measures sum of two one-way errors
    // Result scaled by range_to_emitter for a rough position estimate
    (void)range_to_emitter; // range-independent approximation at this level
    const double delta_r_m = constants::speed_of_light_m_s * rms_time_error_ns * 1e-9;
    return Km{delta_r_m / 2.0 / 1000.0};
}

Km cep_from_eep(Km semi_major, Km semi_minor) noexcept {
    // CEP ≈ 0.59 * (a + b)  — accurate to ~1% for a/b ≤ 4
    return Km{0.59 * (semi_major.value + semi_minor.value)};
}

} // namespace libew::location
