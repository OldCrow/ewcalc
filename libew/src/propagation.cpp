#include "libew/propagation/propagation.h"
#include "libew/core/constants.h"
#include <cmath>

namespace libew::propagation {

Db free_space_path_loss(Km distance, Mhz frequency) noexcept {
    return Db{
        constants::fspl_constant_km_mhz
        + 20.0 * std::log10(distance.value)
        + 20.0 * std::log10(frequency.value)
    };
}

Db two_ray_path_loss(Km distance, Meters tx_height, Meters rx_height) noexcept {
    return Db{
        constants::two_ray_constant_km
        + 40.0 * std::log10(distance.value)
        - 20.0 * std::log10(tx_height.value)
        - 20.0 * std::log10(rx_height.value)
    };
}

Km fresnel_zone_distance(Meters tx_height, Meters rx_height, Mhz frequency) noexcept {
    return Km{
        tx_height.value * rx_height.value * frequency.value
        / constants::fresnel_zone_denominator
    };
}

bool is_two_ray_regime(Km distance, Meters tx_height, Meters rx_height, Mhz frequency) noexcept {
    return distance.value >= fresnel_zone_distance(tx_height, rx_height, frequency).value;
}

Db path_loss(Km distance, Meters tx_height, Meters rx_height, Mhz frequency) noexcept {
    if (is_two_ray_regime(distance, tx_height, rx_height, frequency)) {
        return two_ray_path_loss(distance, tx_height, rx_height);
    }
    return free_space_path_loss(distance, frequency);
}

Db knife_edge_diffraction_loss(
    Km     dist_tx_edge,
    Km     dist_edge_rx,
    Meters los_clearance,
    Mhz    frequency) noexcept
{
    // Wavelength in km: λ = c(m/s) / f(Hz) / 1000
    const double lambda_km = constants::speed_of_light_m_s
                             / (frequency.value * 1.0e6) / 1000.0;

    // Fresnel diffraction parameter v
    // v = h * sqrt(2 * (d1 + d2) / (lambda * d1 * d2))
    // h is negative when the edge is above the LOS (los_clearance < 0 means edge above LOS)
    const double d1 = dist_tx_edge.value;
    const double d2 = dist_edge_rx.value;
    const double h  = -los_clearance.value / 1000.0; // convert m to km, sign: positive = above LOS
    const double lambda = lambda_km;
    const double v = h * std::sqrt(2.0 * (d1 + d2) / (lambda * d1 * d2));

    // Compute gain Gd (dB) from approximation table, then loss = -Gd
    double gd_db = 0.0;
    if (v < -1.0) {
        gd_db = 0.0;
    } else if (v < 0.0) {
        gd_db = 20.0 * std::log10(0.5 - 0.62 * v);
    } else if (v < 1.0) {
        gd_db = 20.0 * std::log10(0.5 * std::exp(-0.95 * v));
    } else if (v < 2.4) {
        const double inner = 0.1184 - (0.38 - 0.1 * v) * (0.38 - 0.1 * v);
        gd_db = (inner > 0.0)
            ? 20.0 * std::log10(0.4 - std::sqrt(inner))
            : 20.0 * std::log10(0.225 / v);
    } else {
        gd_db = 20.0 * std::log10(0.225 / v);
    }

    return Db{-gd_db}; // positive = additional loss
}

} // namespace libew::propagation
