#include "test_main.h"
#include "libew/jamming/jamming.h"
#include <cmath>

using namespace libew::jamming;
using namespace libew::units;
using namespace libew::units::literals;

// Spreadsheet reference values (EW Equations.xls, Jamming Formulae sheet):
//   J/S: signal ERP=30 dBm, jammer ERP=50 dBm, jammer-rx dist=50 km,
//        signal-rx dist=5 km, h_signal=1.5 m, h_jammer=1000 m, h_rx=1.5 m,
//        f=100 MHz, Grx_signal=0 dB, Grx_jammer=0 dB → J/S = 36.478 dB
//   Partial band: signal_bw=0.025 MHz, hop_range=58 MHz, J/S_single=0 dB
//        → BW_opt=0.025 MHz, duty=0.000431
//
// J/S ratio: J/S = J_rx - S_rx (dB), each term from libew::propagation::path_loss.
// Source: Adamy EW101 (2001) Ch.9 "Jamming", "Jamming-to-Signal Ratio";
//         Adamy EW102 (2004) Sec 5.8.1 "Jamming-to-Signal Ratio", p.138.
// [OPEN: exact eq # TBD]
//
// Burnthrough range: analytic inversion of the same LOS/2-ray path-loss model
// used by libew::propagation, solved for J/S == threshold (the range at which
// jamming just achieves the minimum effective J/S).
// Source: Adamy EW101 Ch.9 "Burn-Through"; Adamy EW102 Sec 5.8, p.137-140.
// [OPEN: exact eq # TBD]
//
// Partial-band jamming: BW_opt = signal_bw * 10^(J/S_single/10), capped only
// by hop_range_bandwidth. Per v0.7.0 fix, this widens the jamming bandwidth
// for J/S_single >= 0 dB rather than capping it at signal_bandwidth (see
// libew/src/jamming.cpp). Concept and treatment consistent with Adamy EW102
// Sec 5.9.1 "Jamming Frequency Hop Signals", p.141-146.
// [OPEN: exact eq # TBD]
// NOTE: the doc comment in libew/include/libew/jamming/jamming.h (partial
// band section) describes the opposite, pre-v0.7.0 behavior (BW capped at
// signal_bandwidth for J/S >= 0). That header comment is stale/incorrect
// relative to the current implementation and these tests; flagged as a
// discrepancy rather than corrected here (header edits out of scope).

void test_js_ratio_spreadsheet() {
    const JammingResult r = comms_jamming_js(
        30.0_dBm,    // signal ERP
        50.0_dBm,    // jammer ERP
        5.0_km,      // signal to rx
        50.0_km,     // jammer to rx
        1.5_m,       // signal tx height
        1000.0_m,    // jammer height
        1.5_m,       // rx height
        100.0_MHz,   // frequency
        0.0_dB,      // rx gain toward signal
        0.0_dB       // rx gain toward jammer
    );
    // Spreadsheet: J/S = 36.478 dB
    ASSERT_NEAR(r.js_ratio.value, 36.478, 0.5);
}

void test_partial_band_zero_js() {
    // single-channel J/S = 0 dB → jam full signal bandwidth
    const PartialBandResult r = partial_band_jamming(
        0.025_MHz,   // signal bandwidth
        58.0_MHz,    // hop range
        0.0_dB       // single-channel J/S
    );
    ASSERT_NEAR(r.optimum_jamming_bandwidth.value, 0.025, 1e-6);
    ASSERT_NEAR(r.duty_cycle, 0.025 / 58.0, 1e-5);
}

void test_partial_band_negative_js() {
    // J/S = -10 dB: jammer weaker than signal on each channel.
    // BW_opt = 0.025 * 10^(-10/10) = 0.025 * 0.1 = 0.0025 MHz
    // duty   = 0.0025 / 58.0 ≈ 4.310e-5
    const PartialBandResult r = partial_band_jamming(
        0.025_MHz,
        58.0_MHz,
        -10.0_dB
    );
    ASSERT_NEAR(r.optimum_jamming_bandwidth.value, 0.0025, 1e-6);
    ASSERT_NEAR(r.duty_cycle, 0.0025 / 58.0, 1e-7);
}

void test_js_ratio_self_screen() {
    // Equal ERP, equal range, equal height, same path → J/S = 0 dB (both see same loss)
    const JammingResult r = comms_jamming_js(
        40.0_dBm, 40.0_dBm,
        10.0_km, 10.0_km,
        10.0_m, 10.0_m, 10.0_m,
        150.0_MHz,
        0.0_dB, 0.0_dB
    );
    ASSERT_NEAR(r.js_ratio.value, 0.0, 0.1);
}

// ---------------------------------------------------------------------------
// burnthrough_range
//
// At the returned burnthrough range, a call to comms_jamming_js with
// the same geometry must give J/S == js_threshold.
// ---------------------------------------------------------------------------

void test_burnthrough_range_los_regime() {
    // Parameters chosen so the LOS burnthrough distance is below the Fresnel
    // zone crossover for the signal path.
    //
    // Signal: ERP=30 dBm, h_tx=100 m, freq=100 MHz
    // Jammer: ERP=50 dBm, dist=50 km, h_j=1000 m, h_rx=1.5 m
    // FZ_signal = 100*1.5*100/24000 = 0.625 km
    // LOS burnthrough ≈ 35.6 km  — wait, 35.6 > 0.625, so this lands in 2-ray.
    // Use h_tx=100 m, h_rx=100 m, freq=100 MHz → FZ = 100*100*100/24000 = 41.7 km
    // LOS burnthrough ≈ 35.6 km < 41.7 km ✓
    const Km bt = burnthrough_range(
        30.0_dBm,   // signal ERP
        50.0_dBm,   // jammer ERP
        50.0_km,    // jammer-to-rx dist
        100.0_m,    // signal tx height
        1000.0_m,   // jammer height
        100.0_m,    // rx height
        100.0_MHz,  // frequency
        0.0_dB      // J/S threshold
    );
    // Verify: J/S at the returned burnthrough range must equal the threshold.
    const JammingResult check = comms_jamming_js(
        30.0_dBm, 50.0_dBm,
        bt, 50.0_km,
        100.0_m, 1000.0_m, 100.0_m,
        100.0_MHz,
        0.0_dB, 0.0_dB
    );
    ASSERT_NEAR(check.js_ratio.value, 0.0, 0.5);
    ASSERT_TRUE(bt.value > 0.0);
}

void test_burnthrough_range_two_ray_regime() {
    // Low signal antenna heights → very small Fresnel zone; LOS solution
    // would far exceed it, so the 2-ray formula applies.
    //
    // Signal: ERP=50 dBm, h_tx=1.5 m, freq=100 MHz
    // FZ_signal = 1.5*1.5*100/24000 ≈ 9 m ≈ 0.009 km
    // LOS burnthrough ~ 35 km >> 0.009 km → 2-ray inversion used.
    const Km bt = burnthrough_range(
        50.0_dBm,   // signal ERP
        70.0_dBm,   // jammer ERP
        50.0_km,    // jammer-to-rx dist
        1.5_m,      // signal tx height (low)
        1000.0_m,   // jammer height
        1.5_m,      // rx height (low)
        100.0_MHz,  // frequency
        0.0_dB      // J/S threshold
    );
    const JammingResult check = comms_jamming_js(
        50.0_dBm, 70.0_dBm,
        bt, 50.0_km,
        1.5_m, 1000.0_m, 1.5_m,
        100.0_MHz,
        0.0_dB, 0.0_dB
    );
    ASSERT_NEAR(check.js_ratio.value, 0.0, 0.5);
    ASSERT_TRUE(bt.value > 0.0);
}

void test_partial_band_zero_hop_range_returns_zero() {
    // Zero hop_range is invalid; must return {0, 0} rather than divide-by-zero.
    const PartialBandResult r = partial_band_jamming(
        0.025_MHz, Mhz{0.0}, 10.0_dB);
    ASSERT_NEAR(r.optimum_jamming_bandwidth.value, 0.0, 1e-12);
    ASSERT_NEAR(r.duty_cycle, 0.0, 1e-12);
}

void test_partial_band_positive_js() {
    // J/S = +6 dB: excess J/S over the minimum needed lets the jammer
    // spread its power across a wider bandwidth while still meeting
    // threshold on each channel.
    // BW_opt = signal_bw * 10^(6/10) = 0.025 * 10^0.6 = 0.025 * 3.9810717...
    //        ≈ 0.09952679 MHz  (< hop_range, so bw_jam == bw_opt)
    // duty   = bw_jam / hop_range ≈ 0.09952679 / 58.0 ≈ 0.0017160
    const double expected_bw_opt = 0.025 * std::pow(10.0, 6.0 / 10.0);
    const PartialBandResult r = partial_band_jamming(
        0.025_MHz,   // signal bandwidth
        58.0_MHz,    // hop range
        6.0_dB       // single-channel J/S
    );
    ASSERT_NEAR(r.optimum_jamming_bandwidth.value, expected_bw_opt, 1e-6);
    ASSERT_NEAR(r.duty_cycle, expected_bw_opt / 58.0, 1e-7);
}

int main() {
    std::cout << "=== test_jamming ===\n";
    RUN_TEST(test_js_ratio_spreadsheet);
    RUN_TEST(test_partial_band_zero_js);
    RUN_TEST(test_partial_band_negative_js);
    RUN_TEST(test_js_ratio_self_screen);
    RUN_TEST(test_burnthrough_range_los_regime);
    RUN_TEST(test_burnthrough_range_two_ray_regime);
    RUN_TEST(test_partial_band_zero_hop_range_returns_zero);
    RUN_TEST(test_partial_band_positive_js);
    return test::summary();
}
