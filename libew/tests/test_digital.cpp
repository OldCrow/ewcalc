#include "test_main.h"
#include "libew/digital/digital.h"
#include <cmath>

using namespace libew::digital;
using namespace libew::units;
using namespace libew::units::literals;

// ---------------------------------------------------------------------------
// Eb/N₀ ↔ SNR conversions
//
// Eb/N₀ = SNR + 10·log10(BW / Rb)
// Source: Adamy EW102; Proakis Digital Communications.
// ---------------------------------------------------------------------------

void test_eb_no_equals_snr_when_bw_equals_rate() {
    // BW = Rb → 10·log10(1) = 0 dB → Eb/N₀ = SNR
    ASSERT_NEAR(eb_no_from_snr(10.0_dB, 1.0_MHz, 1.0_MHz).value, 10.0, 1e-9);
    ASSERT_NEAR(eb_no_from_snr(0.0_dB,  5.0_MHz, 5.0_MHz).value,  0.0, 1e-9);
}

void test_eb_no_higher_when_bw_exceeds_rate() {
    // BW = 10 · Rb → Eb/N₀ = SNR + 10 dB
    ASSERT_NEAR(eb_no_from_snr(5.0_dB, 10.0_MHz, 1.0_MHz).value, 15.0, 0.001);
}

void test_eb_no_lower_when_rate_exceeds_bw() {
    // BW = 0.1 · Rb → Eb/N₀ = SNR - 10 dB
    ASSERT_NEAR(eb_no_from_snr(5.0_dB, 1.0_MHz, 10.0_MHz).value, -5.0, 0.001);
}

void test_snr_from_eb_no_is_inverse() {
    // snr_from_eb_no is the exact inverse of eb_no_from_snr
    const Db snr_original{12.3};
    const Mhz bw{4.0};
    const Mhz rb{1.0};
    const Db eb_no = eb_no_from_snr(snr_original, bw, rb);
    const Db snr_recovered = snr_from_eb_no(eb_no, bw, rb);
    ASSERT_NEAR(snr_recovered.value, snr_original.value, 1e-9);
}

// ---------------------------------------------------------------------------
// DSSS process gain
//
// PG = 10·log10(chip_rate / data_rate)
// Source: Adamy EW103; Dixon Spread Spectrum Systems.
// ---------------------------------------------------------------------------

void test_dsss_process_gain_1000x() {
    // 10 Mcps / 0.01 Mbps = 1000 → 30 dB
    ASSERT_NEAR(dsss_process_gain(10.0_MHz, Mhz{0.01}).value, 30.0, 0.001);
}

void test_dsss_process_gain_unity() {
    // chip_rate == data_rate → 0 dB gain (no spreading)
    ASSERT_NEAR(dsss_process_gain(1.0_MHz, 1.0_MHz).value, 0.0, 1e-9);
}

void test_dsss_process_gain_doubles_with_10x_chip_rate() {
    // 10× chip rate → 10 dB additional gain
    const Db pg1 = dsss_process_gain(1.0_MHz,  Mhz{0.001});
    const Db pg2 = dsss_process_gain(10.0_MHz, Mhz{0.001});
    ASSERT_NEAR(pg2.value - pg1.value, 10.0, 0.001);
}

// ---------------------------------------------------------------------------
// DSSS jamming margin and required J/S
//
// JM = PG - Eb/N₀_req - L_impl
// J/S_req = -JM
// Source: Adamy EW103; Torrieri Principles of Spread-Spectrum Communication.
// ---------------------------------------------------------------------------

void test_dsss_jamming_margin_zero_loss() {
    // PG=30 dB, Eb/N₀_req=10 dB, L_impl=0 dB → JM = 20 dB
    ASSERT_NEAR(dsss_jamming_margin(30.0_dB, 10.0_dB, 0.0_dB).value, 20.0, 1e-9);
}

void test_dsss_jamming_margin_with_implementation_loss() {
    // PG=30 dB, Eb/N₀_req=10 dB, L_impl=3 dB → JM = 17 dB
    ASSERT_NEAR(dsss_jamming_margin(30.0_dB, 10.0_dB, 3.0_dB).value, 17.0, 1e-9);
}

void test_dsss_required_js_is_negation_of_margin() {
    // J/S_req = -JM for any parameters
    const Db pg{25.0};
    const Db eb_no_req{8.0};
    const Db l_impl{2.0};
    const Db jm  = dsss_jamming_margin(pg, eb_no_req, l_impl);
    const Db js  = dsss_required_js(pg, eb_no_req, l_impl);
    ASSERT_NEAR(js.value, -jm.value, 1e-9);
}

void test_dsss_required_js_known_value() {
    // PG=30 dB, Eb/N₀_req=10 dB, L_impl=0 dB → J/S_req = -20 dB
    ASSERT_NEAR(dsss_required_js(30.0_dB, 10.0_dB, 0.0_dB).value, -20.0, 1e-9);
}

int main() {
    std::cout << "=== test_digital ===\n";
    RUN_TEST(test_eb_no_equals_snr_when_bw_equals_rate);
    RUN_TEST(test_eb_no_higher_when_bw_exceeds_rate);
    RUN_TEST(test_eb_no_lower_when_rate_exceeds_bw);
    RUN_TEST(test_snr_from_eb_no_is_inverse);
    RUN_TEST(test_dsss_process_gain_1000x);
    RUN_TEST(test_dsss_process_gain_unity);
    RUN_TEST(test_dsss_process_gain_doubles_with_10x_chip_rate);
    RUN_TEST(test_dsss_jamming_margin_zero_loss);
    RUN_TEST(test_dsss_jamming_margin_with_implementation_loss);
    RUN_TEST(test_dsss_required_js_is_negation_of_margin);
    RUN_TEST(test_dsss_required_js_known_value);
    return test::summary();
}
