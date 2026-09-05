#pragma once

/// @file radar.h
/// @brief Radar range equation, pulse compression, coherent integration gain,
///        and detection statistics (Pd/Pfa/SNR, Swerling fluctuation, scan timing).

#include "libew/core/units.h"
#include <cstdint>

namespace libew::radar {

using namespace libew::units;

// ---------------------------------------------------------------------------
// Radar range equation
// ---------------------------------------------------------------------------

struct RadarRangeResult {
    Km max_range;    ///< Maximum detection range (km)
    Db two_way_loss; ///< Total two-way propagation loss at max range (dB)
};

/// Radar range equation (one-way link budget form, applied twice).
///
/// The radar range equation in log domain:
///   R_max^4 = P_t * G^2 * λ^2 * σ / ((4π)^3 * k*T*B*NF*SNR_min * L_sys)
///
/// Rearranged as a link budget:
///   20*log10(R_max) = (P_t_dBm + 2*G_dBi + 20*log10(λ) + σ_dBsm
///                      - 30*log10(4π) - noise_power_dBm - SNR_dB - L_sys_dB) / 4
/// where λ = c / f_MHz / 1e6 (meters) and noise_power = sensitivity - SNR
///
/// @param tx_power     Transmitter power (dBm)
/// @param antenna_gain Antenna gain (dBi, same antenna used for Tx and Rx)
/// @param target_rcs   Target radar cross-section (dBsm)
/// @param frequency    Radar carrier frequency (MHz)
/// @param system_losses Combined system losses (dB): feed, atmospheric, etc.
/// @param noise_figure Receiver noise figure (dB)
/// @param bandwidth    Receiver noise bandwidth (MHz)
/// @param required_snr Minimum required SNR for detection (dB)
/// @return RadarRangeResult with max range and two-way path loss
[[nodiscard]] RadarRangeResult radar_range(
    Dbm  tx_power,
    Db   antenna_gain,
    Dbsm target_rcs,
    Mhz  frequency,
    Db   system_losses,
    Db   noise_figure,
    Mhz  bandwidth,
    Db   required_snr) noexcept;

// ---------------------------------------------------------------------------
// Signal processing gain
// ---------------------------------------------------------------------------

/// Pulse compression gain.
/// G_pc = 10*log10(time_bandwidth_product)
/// TB product = pulse width (s) × bandwidth (Hz) — must be > 1 for gain.
/// @param time_bandwidth_product  Pulse width × bandwidth (dimensionless)
/// @return Pulse compression gain (dB)
[[nodiscard]] Db pulse_compression_gain(double time_bandwidth_product) noexcept;

/// Coherent integration gain.
/// G_int = 10*log10(num_pulses)
/// @param num_pulses  Number of coherently integrated pulses
/// @return Coherent integration gain (dB)
[[nodiscard]] Db coherent_integration_gain(int num_pulses) noexcept;

/// Wavelength in meters from frequency in MHz (convenience function).
[[nodiscard]] Meters wavelength_m(Mhz frequency) noexcept;

/// LPI radar advantage — the detection-range disadvantage a non-coherent
/// intercept receiver suffers compared to a matched-filter PC radar with the
/// same average power.
///
/// Comparison scenario (Adamy EW102, Low Probability of Intercept Radars):
///   - Radar uses pulse compression with time-bandwidth product TB.
///   - Intercept receiver: energy-detecting (non-coherent), cannot perform
///     matched filtering; integrates the uncompressed pulse at full bandwidth.
///   - Radar receiver: matched filter with full PC gain of 10·log10(TB) dB.
///
/// The intercept receiver’s detection range scales as (peak power)^(1/2),
/// the radar’s detection range scales as (average power)^(1/4) with PC gain.
/// The range advantage of the intercept receiver over the radar scales as
/// TB^(−1/4), so the LPI advantage (intercept range / radar range) in dB is:
///
///   LPI advantage = 10·log10(TB) / 4   (= pulse_compression_gain / 4)
///
/// Note: if the intercept receiver also uses a matched filter (worst case for
/// the radar), the advantage collapses to 0 dB.
///
/// @param time_bandwidth_product  Pulse width × bandwidth (dimensionless, ≥ 1)
/// @return LPI advantage (dB)
[[nodiscard]] Db lpi_advantage(double time_bandwidth_product) noexcept;

// ---------------------------------------------------------------------------
// Detection statistics
// ---------------------------------------------------------------------------

/// Swerling target-fluctuation model.
///
/// The case number encodes the two classic model axes: the RCS probability
/// density (exponential — many comparable scatterers — for cases 1/2; chi-4
/// — one dominant plus small scatterers — for cases 3/4) and the
/// decorrelation interval (scan-to-scan for 1/3, pulse-to-pulse for 2/4).
/// `nonfluctuating` is the constant-RCS Marcum / "Swerling 0/5" model.
enum class Swerling : std::uint8_t {
    nonfluctuating = 0, ///< Constant RCS (Marcum / Swerling 0 or 5)
    case1 = 1,          ///< Exponential RCS pdf, scan-to-scan decorrelation
    case2 = 2,          ///< Exponential RCS pdf, pulse-to-pulse decorrelation
    case3 = 3,          ///< Chi-4 RCS pdf, scan-to-scan decorrelation
    case4 = 4,          ///< Chi-4 RCS pdf, pulse-to-pulse decorrelation
};

/// Required single-pulse SNR — Albersheim's equation (nonfluctuating target,
/// linear detector, noncoherent integration of N pulses).
///
///   A = ln(0.62 / Pfa),   B = ln(Pd / (1 - Pd))
///   SNR_dB = -5·log10(N) + (6.2 + 4.545/sqrt(N + 0.44)) · log10(A + 0.12·A·B + 1.7·B)
///
/// Empirical fit to Robertson's detection curves; error < 0.2 dB for
/// 1e-7 <= Pfa <= 1e-3, 0.1 <= Pd <= 0.9, 1 <= N <= 8096. Also serviceable
/// for square-law detectors (linear vs square-law differs by ~0.2 dB).
///
/// @param pd          Probability of detection (0 < pd < 1)
/// @param pfa         Probability of false alarm (0 < pfa < 1)
/// @param num_pulses  Pulses noncoherently integrated (>= 1)
/// @return Required single-pulse SNR (dB)
[[nodiscard]] Db required_snr_albersheim(double pd, double pfa, int num_pulses) noexcept;

/// Required single-pulse SNR — Shnidman's equation (square-law detector,
/// noncoherent integration of N pulses, Swerling 0-4 fluctuation).
///
///   K    = inf (Sw0), 1 (Sw1), N (Sw2), 2 (Sw3), 2N (Sw4)
///   alpha= 0 for N < 40, 0.25 for N >= 40
///   eta  = sqrt(-0.8·ln(4·Pfa·(1-Pfa))) + sign(Pd-0.5)·sqrt(-0.8·ln(4·Pd·(1-Pd)))
///   X∞   = eta·(eta + 2·sqrt(N/2 + alpha - 0.25))
///   C1   = {[(17.7006·Pd - 18.4496)·Pd + 14.5339]·Pd - 3.525} / K
///   C2   = (1/K)·{exp(27.31·Pd - 25.14) + (Pd - 0.8)·[0.7·ln(1e-5/Pfa) + (2N-20)/80]}
///          (applied only for Pd > 0.872)
///   SNR_dB = 10·log10(X∞/N) + C1 + C2
///
/// Error < 0.5 dB over 0.1 <= Pd <= 0.99, 1e-9 <= Pfa <= 1e-3, 1 <= N <= 100
/// (validated in test_radar.cpp against an exact Marcum-Q / noncentral-χ²
/// oracle; see the test header for the oracle recipe).
///
/// @param pd          Probability of detection (0 < pd < 1)
/// @param pfa         Probability of false alarm (0 < pfa < 1)
/// @param num_pulses  Pulses noncoherently integrated (>= 1)
/// @param swerling    Target fluctuation model
/// @return Required single-pulse SNR (dB)
[[nodiscard]] Db required_snr_shnidman(double pd, double pfa, int num_pulses,
                                       Swerling swerling) noexcept;

/// Fluctuation loss — the extra SNR a fluctuating target demands over a
/// nonfluctuating one for the same Pd/Pfa/N (Shnidman-based):
///   L_f = SNR_required(swerling) - SNR_required(Swerling 0)
/// Positive for Pd > ~0.5 in the scan-to-scan cases (the usual regime);
/// can be negative at low Pd where fluctuation helps detection.
/// @return Fluctuation loss (dB)
[[nodiscard]] Db fluctuation_loss(double pd, double pfa, int num_pulses,
                                  Swerling swerling) noexcept;

// ---------------------------------------------------------------------------
// Scan timing
// ---------------------------------------------------------------------------

/// Dwell time — how long a scanning beam illuminates a point target.
///   T_D = theta_az / scan_rate
/// @param azimuth_beamwidth  3 dB azimuth beamwidth (degrees)
/// @param scan_rate_deg_s    Antenna scan rate (degrees/second)
/// @return Dwell time (seconds)
[[nodiscard]] Seconds dwell_time(Degrees azimuth_beamwidth,
                                 double scan_rate_deg_s) noexcept;

/// Hits per scan — pulses striking the target per beam pass.
///   n = T_D · PRF
/// @param dwell   Dwell time (seconds)
/// @param prf_hz  Pulse repetition frequency (Hz)
/// @return Number of hits (dimensionless, not rounded)
[[nodiscard]] double hits_per_scan(Seconds dwell, double prf_hz) noexcept;

/// False-alarm rate from false-alarm probability and receiver bandwidth.
///   FAR = Pfa · B
/// (one independent detection opportunity per resolution time 1/B)
/// @param pfa        Probability of false alarm per detection trial
/// @param bandwidth  Receiver noise bandwidth (MHz)
/// @return False alarms per second (Hz)
[[nodiscard]] double false_alarm_rate_hz(double pfa, Mhz bandwidth) noexcept;

} // namespace libew::radar
