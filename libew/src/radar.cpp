#include "libew/radar/radar.h"
#include "libew/receiver/receiver.h"
#include "libew/core/constants.h"
#include <cmath>
#include <numbers>

namespace libew::radar {

Meters wavelength_m(Mhz frequency) noexcept {
    return Meters{constants::speed_of_light_m_s / (frequency.value * 1.0e6)};
}

RadarRangeResult radar_range(
    Dbm  tx_power,
    Db   antenna_gain,
    Dbsm target_rcs,
    Mhz  frequency,
    Db   system_losses,
    Db   noise_figure,
    Mhz  bandwidth,
    Db   required_snr) noexcept
{
    // Radar range equation in dB form:
    //
    // R_max^4 = Pt * G^2 * λ^2 * σ / ((4π)^3 * kTBNF * SNR * L)
    //
    // Taking 10*log10 of both sides:
    //   40*log10(R_m) = Pt_dBm + 2*G_dB + 20*log10(λ_m) + σ_dBsm
    //                   - 30*log10(4π) - noise_power_dBm - SNR_dB - L_dB
    //   where R is in metres (convert to km at the end)
    //   and noise_power_dBm = sensitivity - SNR (the kTBNF term)

    const double lambda_m = wavelength_m(frequency).value;

    // Noise power = kTBNF (dBm)
    const Dbm noise_power = receiver::system_sensitivity(bandwidth, noise_figure, Db{0.0});

    // Compute the numerator (all log terms that add to range)
    const double numerator_db =
          tx_power.value                        // Pt (dBm)
        + 2.0 * antenna_gain.value              // G^2 (dB)
        + 20.0 * std::log10(lambda_m)           // λ^2 term
        + target_rcs.value                       // σ (dBsm)
        - 30.0 * std::log10(constants::four_pi) // (4π)^3 term
        - noise_power.value                      // kTBNF (dBm)
        - required_snr.value                     // SNR threshold (dB)
        - system_losses.value;                   // L_sys (dB)

    // R_m = 10^(numerator / 40)
    const double range_m = std::pow(10.0, numerator_db / 40.0);
    const Km range_km{range_m / 1000.0};

    // Two-way path loss at this range (free-space approximation)
    // Two-way loss = 2 * FSPL, but expressed differently for radar:
    // L_2way = (4π * R / λ)^2  for each one-way path, squared for round-trip
    const double one_way_loss_db = 20.0 * std::log10(constants::four_pi * range_m / lambda_m);
    const Db two_way_loss{2.0 * one_way_loss_db};

    return {range_km, two_way_loss};
}

Db pulse_compression_gain(double time_bandwidth_product) noexcept {
    return Db{10.0 * std::log10(time_bandwidth_product)};
}

Db coherent_integration_gain(int num_pulses) noexcept {
    return Db{10.0 * std::log10(static_cast<double>(num_pulses))};
}

} // namespace libew::radar
