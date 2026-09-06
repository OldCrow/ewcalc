#include "test_main.h"
#include "libew/radar/radar.h"

using namespace libew::radar;
using namespace libew::units;
using namespace libew::units::literals;

// ---------------------------------------------------------------------------
// Pulse compression gain: G_pc = 10*log10(time_bandwidth_product).
// Source: Adamy EW102 (2004) Sec 3.5.2 "Pulse Compression", p.51.
// ---------------------------------------------------------------------------

void test_pulse_compression_gain() {
    // TB = 1000 → gain = 30 dB
    ASSERT_NEAR(pulse_compression_gain(1000.0).value, 30.0, 0.01);
    // TB = 1 → gain = 0 dB
    ASSERT_NEAR(pulse_compression_gain(1.0).value, 0.0, 0.01);
}

// ---------------------------------------------------------------------------
// Coherent integration gain: G_int = 10*log10(num_pulses). Standard
// non-fluctuating-target coherent pulse integration gain (general radar
// signal processing result, not Adamy-specific).
// Source: Richards, Fundamentals of Radar Signal Processing (coherent
// integration). Verified absent from Adamy (2026-09-06): EW102 Ch. 3
// discusses coherence contextually (pulse radar, detection, LPI) but
// states no formula.
// ---------------------------------------------------------------------------

void test_coherent_integration_gain() {
    // 100 pulses → 20 dB
    ASSERT_NEAR(coherent_integration_gain(100).value, 20.0, 0.01);
    // 10 pulses → 10 dB
    ASSERT_NEAR(coherent_integration_gain(10).value, 10.0, 0.01);
}

// ---------------------------------------------------------------------------
// Radar range equation (link-budget/log form of the classic radar range
// equation, applied one-way each direction plus RCS gain term).
// Source: Adamy EW102 Sec 3.2 "Radar Range Equation", p.36;
//         Sec 3.2.2 "Radar Detection Range", p.40.
// ---------------------------------------------------------------------------

void test_radar_range_computed() {
    // Correctness test with analytically-derived expected value.
    // Note (issue #14 item 2): the expected value below is hand-derived from
    // the documented radar range equation (see the derivation comment), not
    // computed by calling radar_range() itself, so this is a genuine
    // independent reference check rather than a self-referential round-trip.
    //
    // Inputs: Pt=60 dBm, G=30 dBi, σ=0 dBsm, f=3 GHz, L=3 dB, NF=5 dB, B=1 MHz, SNR=13 dB
    //
    // Derivation (from radar_range equation in radar.cpp):
    //   λ = c/f = 2.998e8/3e9 = 0.09993 m
    //   noise_power = sensitivity(1 MHz, 5 dB NF, 0 dB SNR) = -114+0+5 = -109 dBm
    //   numerator_dB = 60 + 2*30 + 20*log10(0.09993) + 0
    //                  - 30*log10(4π) - (-109) - 13 - 3
    //                ≈ 60 + 60 - 20.01 + 0 - 33.02 + 109 - 13 - 3 = 159.97 dB
    //   R_m = 10^(159.97/40) = 10^3.999 ≈ 9990 m ≈ 10.0 km
    // Harness output (default presenter inputs): 10.010 km
    const RadarRangeResult r = radar_range(
        Dbm{60.0},   // 1 kW transmitter
        Db{30.0},    // 30 dBi antenna gain
        Dbsm{0.0},   // 1 m² RCS
        Mhz{3000.0}, // X-band
        Db{3.0},     // system losses
        Db{5.0},     // noise figure
        Mhz{1.0},    // bandwidth
        Db{13.0}     // required SNR
    );
    ASSERT_NEAR(r.max_range.value, 10.01, 0.05);
    ASSERT_TRUE(r.two_way_loss.value > 0.0);
}

void test_wavelength() {
    // λ = c / f (basic physics; no domain-specific citation needed).
    // 3 GHz → λ = 0.1 m
    ASSERT_NEAR(wavelength_m(3000.0_MHz).value, 0.09993, 0.001);
    // 300 MHz → λ = 1 m
    ASSERT_NEAR(wavelength_m(300.0_MHz).value, 0.9993, 0.001);
}

// ---------------------------------------------------------------------------
// LPI advantage: ratio of intercept-receiver range disadvantage to radar
// detection range, expressed in dB as pulse_compression_gain / 4. This
// follows from the radar detection range scaling as (avg power * PC gain)^(1/4)
// versus a non-coherent intercept receiver's range scaling as (peak power)^(1/2)
// (see libew/include/libew/radar/radar.h derivation comment).
// Source: Adamy EW102 Sec 3.9 "Low Probability of Intercept Radars", p.67-72
//         (Sec 3.9.5 "LPI Figure of Merit", p.71).
// (An earlier audit note here flagged radar.h as citing EW103 for this; the
// header now correctly cites EW102 — resolved in the v1.0.0 release change,
// closed as #67.)
// ---------------------------------------------------------------------------

void test_lpi_advantage() {
    // lpi_advantage = 10*log10(TB) / 4
    // TB = 100   → 10*log10(100)/4   = 10*2/4  = 5.0 dB
    // TB = 10000 → 10*log10(10000)/4 = 10*4/4  = 10.0 dB
    ASSERT_NEAR(lpi_advantage(100.0).value, 5.0, 0.01);
    ASSERT_NEAR(lpi_advantage(10000.0).value, 10.0, 0.01);
}

// ---------------------------------------------------------------------------
// Detection statistics.
//
// Sources: W. J. Albersheim, "Closed-Form Approximation to Robertson's
// Detection Characteristics", Proc. IEEE 69(7), 1981 (as given in
// M. A. Richards, "Noncoherent Integration Gain, and its Approximation",
// 2010, eq. 12 — note the 4.545 constant); D. A. Shnidman, "Determination
// of Required SNR Values", IEEE Trans. AES 38(3), 2002.
//
// Two kinds of expected values below:
//  (1) Formula-fidelity spot values: the published closed forms evaluated
//      independently in Python (tools-free transcription check, 1e-6 tol).
//  (2) Exact-theory gates: required SNR computed from first principles
//      (square-law detector, threshold from Q_gamma(N,T)=Pfa; Sw0 via the
//      noncentral-chi-square SF, Sw1/Sw3 by averaging over the RCS pdf,
//      Sw2 via Gamma(N, 1+chi), Sw4 by 1e7-sample Monte Carlo; bisection
//      on chi). Oracle: scipy-based script, recorded values below; worst
//      |Shnidman - exact| observed over the 60-point validation grid is
//      0.30 dB, within Shnidman's published 0.5 dB envelope.
//      Closed-form anchor validating the oracle itself: Swerling 1, N=1
//      has PD = PFA^(1/(1+chi)), so chi = ln(Pfa)/ln(Pd) - 1 exactly.
// ---------------------------------------------------------------------------

void test_albersheim_formula_fidelity() {
    // Independent Python evaluation of eq. 12:
    ASSERT_NEAR(required_snr_albersheim(0.9, 1.0e-6, 1).value,  13.1200180104, 1e-6);
    ASSERT_NEAR(required_snr_albersheim(0.5, 1.0e-6, 10).value,  3.5580323886, 1e-6);
    ASSERT_NEAR(required_snr_albersheim(0.9, 1.0e-4, 30).value,  0.8278671048, 1e-6);
}

void test_albersheim_vs_exact() {
    // Exact square-law values (oracle). Albersheim fits a *linear* detector
    // (~0.2 dB from square-law), so gate at 0.5 dB.
    ASSERT_NEAR(required_snr_albersheim(0.9, 1.0e-6, 1).value,  13.183, 0.5);
    ASSERT_NEAR(required_snr_albersheim(0.5, 1.0e-6, 10).value,  3.651, 0.5);
    ASSERT_NEAR(required_snr_albersheim(0.9, 1.0e-4, 30).value,  0.977, 0.5);
}

void test_shnidman_formula_fidelity() {
    // Independent Python evaluation of Shnidman's closed form. The 0.95
    // case exercises the Pd > 0.872 C2 branch; the N=39/40 pair exercises
    // the alpha breakpoint (0 below 40 pulses, 0.25 at and above).
    ASSERT_NEAR(required_snr_shnidman(0.9,  1.0e-6, 1,  Swerling::case1).value, 21.3460831936, 1e-6);
    ASSERT_NEAR(required_snr_shnidman(0.9,  1.0e-6, 10, Swerling::case2).value,  6.1583306442, 1e-6);
    ASSERT_NEAR(required_snr_shnidman(0.9,  1.0e-4, 30, Swerling::case3).value,  4.9865636374, 1e-6);
    ASSERT_NEAR(required_snr_shnidman(0.5,  1.0e-6, 10, Swerling::case4).value,  3.8124525580, 1e-6);
    ASSERT_NEAR(required_snr_shnidman(0.95, 1.0e-6, 10, Swerling::case1).value, 17.0031488483, 1e-6);
    ASSERT_NEAR(required_snr_shnidman(0.9,  1.0e-6, 40, Swerling::case2).value,  1.4098749674, 1e-6);
    ASSERT_NEAR(required_snr_shnidman(0.9,  1.0e-6, 39, Swerling::case2).value,  1.4681739259, 1e-6);
}

void test_shnidman_vs_exact() {
    // Exact-theory oracle values; gate at Shnidman's published 0.5 dB.
    // Swerling 1, N=1 closed-form anchors (exact to all digits shown):
    ASSERT_NEAR(required_snr_shnidman(0.5, 1.0e-6, 1, Swerling::case1).value, 12.7719, 0.5);
    ASSERT_NEAR(required_snr_shnidman(0.9, 1.0e-4, 1, Swerling::case1).value, 19.3660, 0.5);
    // Grid points across all cases:
    ASSERT_NEAR(required_snr_shnidman(0.9, 1.0e-6, 1,  Swerling::nonfluctuating).value, 13.183, 0.5);
    ASSERT_NEAR(required_snr_shnidman(0.9, 1.0e-6, 10, Swerling::case2).value,  6.292, 0.5);
    ASSERT_NEAR(required_snr_shnidman(0.9, 1.0e-6, 30, Swerling::case1).value, 10.367, 0.5);
    ASSERT_NEAR(required_snr_shnidman(0.9, 1.0e-6, 10, Swerling::case3).value,  9.601, 0.5);
    ASSERT_NEAR(required_snr_shnidman(0.9, 1.0e-6, 10, Swerling::case4).value,  5.808, 0.5);
    ASSERT_NEAR(required_snr_shnidman(0.5, 1.0e-4, 30, Swerling::case3).value, -0.076, 0.5);
}

void test_fluctuation_loss() {
    // Definition: SNR(case) - SNR(Sw0) at identical Pd/Pfa/N.
    const Db l1 = fluctuation_loss(0.9, 1.0e-6, 10, Swerling::case1);
    ASSERT_NEAR(l1.value,
                required_snr_shnidman(0.9, 1.0e-6, 10, Swerling::case1).value
                    - required_snr_shnidman(0.9, 1.0e-6, 10,
                                            Swerling::nonfluctuating).value,
                1e-12);
    // Exact-theory value: 13.500 - 5.267 = 8.23 dB (oracle grid).
    ASSERT_NEAR(l1.value, 8.23, 0.5);
    // Scan-to-scan fluctuation at high Pd costs SNR; Sw0 loss is zero.
    ASSERT_TRUE(l1.value > 0.0);
    ASSERT_NEAR(fluctuation_loss(0.9, 1.0e-6, 10,
                                 Swerling::nonfluctuating).value, 0.0, 1e-12);
}

void test_scan_timing() {
    // Dwell: 2° beam scanning at 36°/s (10 s rotation) → 55.6 ms.
    const Seconds td = dwell_time(Degrees{2.0}, 36.0);
    ASSERT_NEAR(td.value, 2.0 / 36.0, 1e-12);
    // Hits per scan: 55.6 ms × 1000 Hz PRF → 55.6 hits.
    ASSERT_NEAR(hits_per_scan(td, 1000.0), 2000.0 / 36.0, 1e-9);
    // FAR = Pfa · B: 1e-6 × 1 MHz → 1 false alarm per second.
    ASSERT_NEAR(false_alarm_rate_hz(1.0e-6, Mhz{1.0}), 1.0, 1e-12);
}

// ---------------------------------------------------------------------------
// Doppler, PRF ambiguity, and resolution.
// Sources: standard pulse-radar relations (Richards, Fundamentals of Radar
// Signal Processing; Skolnik, Introduction to Radar Systems). EW-series
// anchors (verified 2026-09-06): Doppler shift — Adamy EW102 Sec 3.6.1
// p.55; unambiguous range — stated in PRI terms (R_max < 0.5*PRI*c),
// equivalent to c/(2*PRF); blind speed — not covered in Ch. 3; range
// resolution — Sec 3.8.1's SAR form d = c*PW/2 is the uncompressed-pulse
// equivalent of c/(2B); cross-range — Sec 3.8.2 gives only the SAR form
// lambda*R/(2L), a different sensor model from the implemented real-beam
// R*theta (method delta, see docs/formulas.md). Skolnik is a primary
// reference for Adamy, but Adamy does not use Skolnik's formulae directly.
// Expected values hand-derived with c = 299 792 458 m/s.
// ---------------------------------------------------------------------------

void test_doppler_shift() {
    // f = 10 GHz, v = 300 m/s closing:
    // f_d = 2·300·1e10/c = 6e12/299792458 = 20013.84... Hz
    ASSERT_NEAR(doppler_shift_hz(Mhz{10000.0}, 300.0), 20013.845, 0.01);
    // Opening target: sign flips.
    ASSERT_NEAR(doppler_shift_hz(Mhz{10000.0}, -300.0), -20013.845, 0.01);
    // Zero radial speed: zero shift.
    ASSERT_NEAR(doppler_shift_hz(Mhz{10000.0}, 0.0), 0.0, 1e-12);
}

void test_unambiguous_range() {
    // PRF = 1 kHz: R_u = c/2000 m = 149.896229 km
    ASSERT_NEAR(unambiguous_range(1000.0).value, 149.896229, 1e-5);
    // PRF = 10 kHz: 14.9896229 km
    ASSERT_NEAR(unambiguous_range(10000.0).value, 14.9896229, 1e-6);
}

void test_blind_speed_and_unambiguous_velocity() {
    // f = 10 GHz → λ = 0.0299792458 m; PRF = 1 kHz:
    // v_b = λ·PRF/2 = 14.9896229 m/s; v_u = λ·PRF/4 = 7.49481145 m/s
    ASSERT_NEAR(blind_speed_m_s(Mhz{10000.0}, 1000.0), 14.9896229, 1e-6);
    ASSERT_NEAR(unambiguous_velocity_m_s(Mhz{10000.0}, 1000.0), 7.49481145, 1e-7);
    // Doppler dilemma invariant: R_u·v_u = c·λ/8 regardless of PRF.
    const double product_1k  = unambiguous_range(1000.0).value * 1000.0
                               * unambiguous_velocity_m_s(Mhz{10000.0}, 1000.0);
    const double product_10k = unambiguous_range(10000.0).value * 1000.0
                               * unambiguous_velocity_m_s(Mhz{10000.0}, 10000.0);
    ASSERT_NEAR(product_1k, product_10k, 1e-3);
    ASSERT_NEAR(product_1k, 299792458.0 * 0.0299792458 / 8.0, 1.0);
}

void test_range_resolution() {
    // B = 1 MHz: ΔR = c/2e6 = 149.896229 m
    ASSERT_NEAR(range_resolution(Mhz{1.0}).value, 149.896229, 1e-5);
    // B = 100 MHz: 1.49896229 m
    ASSERT_NEAR(range_resolution(Mhz{100.0}).value, 1.49896229, 1e-7);
}

void test_cross_range_resolution() {
    // R = 100 km, θ = 2°: ΔX = 1e5 · 2·π/180 = 3490.6585... m
    ASSERT_NEAR(cross_range_resolution(Km{100.0}, Degrees{2.0}).value,
                3490.6585, 1e-3);
    // Linear in both range and beamwidth.
    ASSERT_NEAR(cross_range_resolution(Km{50.0}, Degrees{4.0}).value,
                3490.6585, 1e-3);
}

int main() {
    std::cout << "=== test_radar ===\n";
    RUN_TEST(test_pulse_compression_gain);
    RUN_TEST(test_coherent_integration_gain);
    RUN_TEST(test_radar_range_computed);
    RUN_TEST(test_wavelength);
    RUN_TEST(test_lpi_advantage);
    RUN_TEST(test_albersheim_formula_fidelity);
    RUN_TEST(test_albersheim_vs_exact);
    RUN_TEST(test_shnidman_formula_fidelity);
    RUN_TEST(test_shnidman_vs_exact);
    RUN_TEST(test_fluctuation_loss);
    RUN_TEST(test_scan_timing);
    RUN_TEST(test_doppler_shift);
    RUN_TEST(test_unambiguous_range);
    RUN_TEST(test_blind_speed_and_unambiguous_velocity);
    RUN_TEST(test_range_resolution);
    RUN_TEST(test_cross_range_resolution);
    return test::summary();
}
