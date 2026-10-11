#include "test_main.h"
#include "libew/jamming/jamming.h"
#include "libew/core/constants.h"
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
// Source: Adamy EW102 (2004) Sec 5.8.1 "Jamming-to-Signal Ratio", p.138;
//         equivalently Adamy EW103 (2008) Sec 9.1, p.252.
//
//
// Burnthrough range: analytic inversion of the same LOS/2-ray path-loss model
// used by libew::propagation, solved for J/S == threshold (the range at which
// jamming just achieves the minimum effective J/S).
// Source: Adamy EW101 Sec 9.3, pp.187-191 — the fullest Adamy treatment
// (self-protection, stand-off, and comms burnthrough together); also
// Adamy EW102 Sec 5.8, pp.137-140.
//
//
// Partial-band jamming: BW_opt = signal_bw * 10^(J/S_single/10), capped only
// by hop_range_bandwidth. Per v0.7.0 fix, this widens the jamming bandwidth
// for J/S_single >= 0 dB rather than capping it at signal_bandwidth (see
// libew/src/jamming.cpp). Concept and treatment per Adamy EW102
// Sec 5.9.1.2, pp.144-147; equivalently Adamy EW103 Sec 9.3.1, pp.259-262.
//
// The doc comment in libew/include/libew/jamming/jamming.h (partial-band
// section) now matches this implementation and these tests (issue #38).

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

// ---------------------------------------------------------------------------
// Radar jamming (#88). Oracle: the linear radar and one-way link equations,
// independent of the dB forms and their 70.99 constant.
//   S = ERP_S·σ·Gr·λ² / ((4π)³ R⁴),  J = ERP_J·Gr'·λ² / ((4π)² R_J²)
//   SPJ (R_J = R, Gr' = Gr):  J/S = ERP_J·4π·R² / (ERP_S·σ)
// Example: ERP_S = 110 dBm, ERP_J = 40 dBm, σ = 10 m², R = 20 km.
// ---------------------------------------------------------------------------

namespace {
double dbm_to_w(double dbm) { return std::pow(10.0, (dbm - 30.0) / 10.0); }
double to_db(double x)      { return 10.0 * std::log10(x); }
constexpr double kFourPi = 12.566370614359172;
}

void test_radar_js_constant() {
    ASSERT_NEAR(libew::constants::radar_js_constant_db, to_db(kFourPi * 1.0e6), 1e-5);
}

void test_self_protection_js_linear_oracle() {
    const double r_m = 20.0e3;
    const double expected = to_db(dbm_to_w(40.0) * kFourPi * r_m * r_m
                                  / (dbm_to_w(110.0) * 10.0));
    const Db js = self_protection_js(110.0_dBm, 40.0_dBm, 20.0_km, Dbsm{10.0});
    ASSERT_NEAR(js.value, expected, 1e-4);
    ASSERT_NEAR(js.value, 17.013, 1e-3);   // 40 − 110 + 70.992 + 26.021 − 10
}

void test_self_protection_twenty_db_per_decade() {
    const Db near = self_protection_js(110.0_dBm, 40.0_dBm, 2.0_km,  Dbsm{10.0});
    const Db far  = self_protection_js(110.0_dBm, 40.0_dBm, 20.0_km, Dbsm{10.0});
    ASSERT_NEAR(far.value - near.value, 20.0, 1e-9);
}

void test_self_protection_burnthrough_round_trip() {
    const Km bt = self_protection_burnthrough(110.0_dBm, 40.0_dBm, Dbsm{10.0}, 0.0_dB);
    ASSERT_NEAR(bt.value, std::pow(10.0, (110.0 - 40.0 - 70.99210 + 10.0) / 20.0), 1e-6);
    const Db at_bt = self_protection_js(110.0_dBm, 40.0_dBm, bt, Dbsm{10.0});
    ASSERT_NEAR(at_bt.value, 0.0, 1e-9);
    // A higher threshold pushes burnthrough outward.
    const Km bt10 = self_protection_burnthrough(110.0_dBm, 40.0_dBm, Dbsm{10.0}, 10.0_dB);
    ASSERT_NEAR(bt10.value / bt.value, std::pow(10.0, 0.5), 1e-9);
}

void test_stand_off_js_linear_oracle() {
    // G_M = 30 dB toward the target, G_S = 0 dB toward the jammer.
    // J/S = ERP_J·G_S·4π·R_T⁴ / (ERP_S·G_M·σ·R_J²)
    const double rt = 20.0e3, rj = 50.0e3;
    const double expected = to_db(dbm_to_w(40.0) * 1.0 * kFourPi * std::pow(rt, 4)
                                  / (dbm_to_w(110.0) * 1000.0 * 10.0 * rj * rj));
    const Db js = stand_off_js(110.0_dBm, 40.0_dBm, 20.0_km, 50.0_km,
                               30.0_dB, 0.0_dB, Dbsm{10.0});
    ASSERT_NEAR(js.value, expected, 1e-4);
}

void test_stand_off_reduces_to_self_protection() {
    // Jammer on the target: R_J = R_T and G_S = G_M.
    const Db spj = self_protection_js(110.0_dBm, 40.0_dBm, 35.0_km, Dbsm{3.0});
    const Db soj = stand_off_js(110.0_dBm, 40.0_dBm, 35.0_km, 35.0_km,
                                25.0_dB, 25.0_dB, Dbsm{3.0});
    ASSERT_NEAR(soj.value, spj.value, 1e-9);
}

void test_stand_off_burnthrough_round_trip() {
    const Km bt = stand_off_burnthrough(110.0_dBm, 40.0_dBm, 50.0_km,
                                        30.0_dB, 0.0_dB, Dbsm{10.0}, 0.0_dB);
    const Db at_bt = stand_off_js(110.0_dBm, 40.0_dBm, bt, 50.0_km,
                                  30.0_dB, 0.0_dB, Dbsm{10.0});
    ASSERT_NEAR(at_bt.value, 0.0, 1e-9);
    // Two-way target path: +10 dB threshold moves R_T by 10^(10/40).
    const Km bt10 = stand_off_burnthrough(110.0_dBm, 40.0_dBm, 50.0_km,
                                          30.0_dB, 0.0_dB, Dbsm{10.0}, 10.0_dB);
    ASSERT_NEAR(bt10.value / bt.value, std::pow(10.0, 0.25), 1e-9);
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
    RUN_TEST(test_radar_js_constant);
    RUN_TEST(test_self_protection_js_linear_oracle);
    RUN_TEST(test_self_protection_twenty_db_per_decade);
    RUN_TEST(test_self_protection_burnthrough_round_trip);
    RUN_TEST(test_stand_off_js_linear_oracle);
    RUN_TEST(test_stand_off_reduces_to_self_protection);
    RUN_TEST(test_stand_off_burnthrough_round_trip);
    return test::summary();
}
