#include "test_main.h"
#include "libew/jamming/jamming.h"

using namespace libew::jamming;
using namespace libew::units;
using namespace libew::units::literals;

// Spreadsheet reference values (EW Equations.xls, Jamming Formulae sheet):
//   J/S: signal ERP=30 dBm, jammer ERP=50 dBm, jammer-rx dist=50 km,
//        signal-rx dist=5 km, h_signal=1.5 m, h_jammer=1000 m, h_rx=1.5 m,
//        f=100 MHz, Grx_signal=0 dB, Grx_jammer=0 dB → J/S = 36.478 dB
//   Partial band: signal_bw=0.025 MHz, hop_range=58 MHz, J/S_single=0 dB
//        → BW_opt=0.025 MHz, duty=0.000431

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

int main() {
    std::cout << "=== test_jamming ===\n";
    RUN_TEST(test_js_ratio_spreadsheet);
    RUN_TEST(test_partial_band_zero_js);
    RUN_TEST(test_partial_band_negative_js);
    RUN_TEST(test_js_ratio_self_screen);
    return test::summary();
}
