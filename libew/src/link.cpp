#include "libew/link/link.h"
#include "libew/propagation/propagation.h"
#include "libew/core/constants.h"
#include <cmath>

namespace libew::link {

OnewayLinkResult one_way_link(
    Dbm    tx_power,
    Db     tx_gain,
    Db     rx_gain,
    Km     distance,
    Meters tx_height,
    Meters rx_height,
    Mhz    frequency) noexcept
{
    const Km  fz   = propagation::fresnel_zone_distance(tx_height, rx_height, frequency);
    const bool two_ray = distance.value >= fz.value;
    const Db   loss = two_ray
        ? propagation::two_ray_path_loss(distance, tx_height, rx_height)
        : propagation::free_space_path_loss(distance, frequency);

    // Prx = Ptx + Gtx + Grx - loss
    const Dbm received = tx_power + tx_gain + rx_gain - loss;

    return {received, loss, fz, two_ray};
}

EffectiveRangeResult effective_range(
    Dbm    tx_power,
    Db     tx_gain,
    Db     rx_gain,
    Meters tx_height,
    Meters rx_height,
    Mhz    frequency,
    Dbm    rx_sensitivity) noexcept
{
    // Available margin above sensitivity
    const double margin_db = (tx_power + tx_gain + rx_gain - rx_sensitivity).value;

    // LOS range: margin = 32.44 + 20*log10(d) + 20*log10(f)
    // => d = 10^((margin - 32.44 - 20*log10(f)) / 20)
    const double los_exp = (margin_db - constants::fspl_constant_km_mhz
                            - 20.0 * std::log10(frequency.value)) / 20.0;
    const Km los_range{std::pow(10.0, los_exp)};

    const Km fz = propagation::fresnel_zone_distance(tx_height, rx_height, frequency);

    if (los_range.value <= fz.value) {
        // Effective range is in the LOS regime
        return {los_range, false};
    }

    // 2-ray range: margin = 120 + 40*log10(d) - 20*log10(h_t) - 20*log10(h_r)
    // => d = 10^((margin - 120 + 20*log10(h_t) + 20*log10(h_r)) / 40)
    const double two_ray_exp = (margin_db - constants::two_ray_constant_km
                                + 20.0 * std::log10(tx_height.value)
                                + 20.0 * std::log10(rx_height.value)) / 40.0;
    const Km two_ray_range{std::pow(10.0, two_ray_exp)};

    return {two_ray_range, true};
}

} // namespace libew::link
