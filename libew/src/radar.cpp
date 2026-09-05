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

Db lpi_advantage(double time_bandwidth_product) noexcept {
    // LPI advantage = pulse_compression_gain / 4 = 10·log10(TB) / 4
    return Db{10.0 * std::log10(time_bandwidth_product) / 4.0};
}

// ---------------------------------------------------------------------------
// Detection statistics
// ---------------------------------------------------------------------------

Db required_snr_albersheim(double pd, double pfa, int num_pulses) noexcept {
    // Albersheim (1981), as given in Richards, "Noncoherent Integration Gain,
    // and its Approximation" (2010), eq. 12. Empirical fit to Robertson's
    // curves; linear detector, nonfluctuating target.
    const double n = static_cast<double>(num_pulses);
    const double a = std::log(0.62 / pfa);
    const double b = std::log(pd / (1.0 - pd));
    const double snr_db =
        -5.0 * std::log10(n)
        + (6.2 + 4.545 / std::sqrt(n + 0.44))
              * std::log10(a + 0.12 * a * b + 1.7 * b);
    return Db{snr_db};
}

Db required_snr_shnidman(double pd, double pfa, int num_pulses,
                         Swerling swerling) noexcept {
    // Shnidman (2002), "Determination of Required SNR Values", IEEE Trans.
    // AES 38(3). Square-law detector, noncoherent integration of N pulses.
    //
    // The fluctuation correction divides by K, the number of independent
    // RCS samples the detector effectively averages: scan-to-scan models
    // give one (Sw1) or two (Sw3) degrees of freedom regardless of N;
    // pulse-to-pulse models scale with N (Sw2: N, Sw4: 2N). K = inf (no
    // correction) recovers the nonfluctuating case.
    const double n = static_cast<double>(num_pulses);

    // eta: two-sided Gaussian-tail surrogate for (Pfa, Pd). The 4·p·(1-p)
    // form is symmetric about p = 0.5, where the Pd term vanishes.
    const double eta =
        std::sqrt(-0.8 * std::log(4.0 * pfa * (1.0 - pfa)))
        + std::copysign(1.0, pd - 0.5)
              * std::sqrt(-0.8 * std::log(4.0 * pd * (1.0 - pd)));

    // X_inf: required *total* (N-pulse) SNR for the nonfluctuating target.
    const double alpha = (num_pulses >= 40) ? 0.25 : 0.0;
    const double x_inf = eta * (eta + 2.0 * std::sqrt(n / 2.0 + alpha - 0.25));

    double c_db = 0.0;
    if (swerling != Swerling::nonfluctuating) {
        double k = 1.0;
        switch (swerling) {
            case Swerling::case1: k = 1.0;     break;
            case Swerling::case2: k = n;       break;
            case Swerling::case3: k = 2.0;     break;
            case Swerling::case4: k = 2.0 * n; break;
            case Swerling::nonfluctuating:     break; // unreachable
        }
        // C1: cubic-in-Pd fluctuation penalty; C2: high-Pd correction that
        // switches in above Pd = 0.872 (Shnidman's stated breakpoint).
        const double c1 =
            (((17.7006 * pd - 18.4496) * pd + 14.5339) * pd - 3.525) / k;
        double c2 = 0.0;
        if (pd > 0.872) {
            c2 = (1.0 / k)
                 * (std::exp(27.31 * pd - 25.14)
                    + (pd - 0.8) * (0.7 * std::log(1.0e-5 / pfa)
                                    + (2.0 * n - 20.0) / 80.0));
        }
        c_db = c1 + c2;
    }

    // Per-pulse SNR: divide the total by N, then apply the fluctuation
    // correction in dB.
    return Db{10.0 * std::log10(x_inf / n) + c_db};
}

Db fluctuation_loss(double pd, double pfa, int num_pulses,
                    Swerling swerling) noexcept {
    return Db{required_snr_shnidman(pd, pfa, num_pulses, swerling).value
              - required_snr_shnidman(pd, pfa, num_pulses,
                                      Swerling::nonfluctuating).value};
}

// ---------------------------------------------------------------------------
// Scan timing
// ---------------------------------------------------------------------------

Seconds dwell_time(Degrees azimuth_beamwidth, double scan_rate_deg_s) noexcept {
    return Seconds{azimuth_beamwidth.value / scan_rate_deg_s};
}

double hits_per_scan(Seconds dwell, double prf_hz) noexcept {
    return dwell.value * prf_hz;
}

double false_alarm_rate_hz(double pfa, Mhz bandwidth) noexcept {
    return pfa * bandwidth.value * 1.0e6;
}

} // namespace libew::radar
