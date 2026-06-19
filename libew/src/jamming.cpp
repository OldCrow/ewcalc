#include "libew/jamming/jamming.h"
#include "libew/propagation/propagation.h"
#include "libew/core/constants.h"
#include "libew/core/units.h"
#include <cmath>
#include <limits>
#include <algorithm>

namespace libew::jamming {

JammingResult comms_jamming_js(
    Dbm    signal_erp,
    Dbm    jammer_erp,
    Km     signal_to_rx_dist,
    Km     jammer_to_rx_dist,
    Meters signal_tx_height,
    Meters jammer_height,
    Meters rx_height,
    Mhz    frequency,
    Db     rx_gain_toward_signal,
    Db     rx_gain_toward_jammer) noexcept
{
    const Db signal_loss = propagation::path_loss(
        signal_to_rx_dist, signal_tx_height, rx_height, frequency);
    const Db jammer_loss = propagation::path_loss(
        jammer_to_rx_dist, jammer_height,    rx_height, frequency);

    const Dbm s_rx = signal_erp + rx_gain_toward_signal - signal_loss;
    const Dbm j_rx = jammer_erp + rx_gain_toward_jammer - jammer_loss;
    const Db  js   = j_rx - s_rx;

    return {js, j_rx, s_rx};
}

Km burnthrough_range(
    Dbm    signal_erp,
    Dbm    jammer_erp,
    Km     jammer_to_rx_dist,
    Meters signal_tx_height,
    Meters jammer_height,
    Meters rx_height,
    Mhz    frequency,
    Db     js_threshold) noexcept
{
    // Jammer power at receiver (fixed geometry, selects LOS or 2-ray automatically)
    const Db  j_loss = propagation::path_loss(jammer_to_rx_dist, jammer_height, rx_height, frequency);
    const Dbm j_rx   = jammer_erp - j_loss;

    // Signal power needed at receiver to achieve J/S == js_threshold
    // j_rx - s_rx = js_threshold  =>  s_rx = j_rx - js_threshold
    const Dbm s_rx_needed = j_rx - js_threshold;

    // Required link margin: ERP minus the needed received power
    const double margin_db = (signal_erp - s_rx_needed).value;

    // LOS analytical inversion:
    // margin = 32.44 + 20*log10(d) + 20*log10(f)
    // => d_los = 10^((margin - 32.44 - 20*log10(f)) / 20)
    const double los_exp  = (margin_db - constants::fspl_constant_km_mhz
                             - 20.0 * std::log10(frequency.value)) / 20.0;
    const Km los_range{std::pow(10.0, los_exp)};

    // Fresnel zone crossover for the signal path
    const Km fz = propagation::fresnel_zone_distance(signal_tx_height, rx_height, frequency);

    if (los_range.value <= fz.value) {
        return los_range;  // LOS regime applies
    }

    // 2-ray analytical inversion (signal path beyond Fresnel zone crossover):
    // margin = 120 + 40*log10(d) - 20*log10(h_tx) - 20*log10(h_rx)
    // => d_2ray = 10^((margin - 120 + 20*log10(h_tx) + 20*log10(h_rx)) / 40)
    const double two_ray_exp = (margin_db - constants::two_ray_constant_km
                                + 20.0 * std::log10(signal_tx_height.value)
                                + 20.0 * std::log10(rx_height.value)) / 40.0;
    return Km{std::pow(10.0, two_ray_exp)};
}

PartialBandResult partial_band_jamming(
    Mhz signal_bandwidth,
    Mhz hop_range_bandwidth,
    Db  single_channel_js) noexcept
{
    // Reject non-positive hop range unconditionally so release builds don't divide by zero.
    if (hop_range_bandwidth.value <= 0.0)
        return {Mhz{0.0}, 0.0};
    // If single-channel J/S >= 0 dB, jam full hop range for maximum coverage
    if (single_channel_js.value >= 0.0) {
        const double duty = std::min(
            signal_bandwidth.value / hop_range_bandwidth.value, 1.0);
        return {signal_bandwidth, duty};
    }

    // Optimum jamming bandwidth maximises effective J/S
    // BW_opt = signal_bw * 10^(js_single / 10)
    const double bw_opt = signal_bandwidth.value * std::pow(10.0, single_channel_js.value / 10.0);
    const double duty   = std::min(bw_opt / hop_range_bandwidth.value, 1.0);

    return {Mhz{bw_opt}, duty};
}

} // namespace libew::jamming
