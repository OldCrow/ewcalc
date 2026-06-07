/// @file test_presenters.cpp
/// @brief Assertion-based tests for the ewpresenter layer.
///
/// Tests cover default validity, key computed values, validation round-trips,
/// callback wiring, and the DigitalPresenter two-tier validity split.

#include "test_main.h"
#include "ewpresenter/ewpresenter.h"

// ============================================================================
// PropagationPresenter
// ============================================================================

void test_propagation_default_valid() {
    ewpresenter::PropagationPresenter p;
    ASSERT_TRUE(p.output().valid);
    ASSERT_TRUE(p.distance_error() == ewpresenter::FieldError::none);
}

void test_propagation_fspl_value() {
    // FSPL at 32.6 km, 100 MHz:
    //   32.44 + 20*log10(32.6) + 20*log10(100) ≈ 102.70 dB
    ewpresenter::PropagationPresenter p;
    ASSERT_NEAR(p.output().fspl.value, 102.70, 0.1);
}

void test_propagation_two_ray_regime() {
    // Fresnel crossover at defaults ≈ 0.13 km; 32.6 km >> crossover → two-ray.
    ewpresenter::PropagationPresenter p;
    ASSERT_TRUE(p.output().two_ray_regime);
}

void test_propagation_validation() {
    ewpresenter::PropagationPresenter p;
    p.set_distance(-1.0);
    ASSERT_FALSE(p.output().valid);
    ASSERT_TRUE(p.distance_error() != ewpresenter::FieldError::none);
    p.set_distance(32.6);
    ASSERT_TRUE(p.output().valid);
    ASSERT_TRUE(p.distance_error() == ewpresenter::FieldError::none);
}

void test_propagation_obstruction_negative_rejected() {
    // obstruction_height uses validate_non_negative; negative is invalid, zero is OK.
    ewpresenter::PropagationPresenter p;
    p.set_obstruction_height(-1.0);
    ASSERT_FALSE(p.output().valid);
    ASSERT_TRUE(p.obstruction_height_error() == ewpresenter::FieldError::invalid_negative);
    p.set_obstruction_height(0.0);
    ASSERT_TRUE(p.output().valid);
}

void test_propagation_callback() {
    ewpresenter::PropagationPresenter p;
    int fire_count = 0;
    p.set_on_change([&](const ewpresenter::PropagationPresenter::Output&) { ++fire_count; });
    p.set_distance(20.0);
    p.set_frequency(200.0);
    ASSERT_TRUE(fire_count == 2);
}

// ============================================================================
// AntennaPresenter
// ============================================================================

void test_antenna_default_valid() {
    ewpresenter::AntennaPresenter p;
    ASSERT_TRUE(p.output().valid);
}

void test_antenna_erp() {
    // ERP = tx_power + gain = 30 dBm + 0 dBi = 30 dBm
    ewpresenter::AntennaPresenter p;
    ASSERT_NEAR(p.output().erp.value, 30.0, 0.01);
}

void test_antenna_wavelength() {
    // λ = c/f ≈ 2.998e8 / 1e9 ≈ 0.300 m at default 1000 MHz
    ewpresenter::AntennaPresenter p;
    ASSERT_NEAR(p.output().wavelength.value, 0.300, 0.001);
}

void test_antenna_validation() {
    ewpresenter::AntennaPresenter p;
    p.set_frequency(-1.0);
    ASSERT_FALSE(p.output().valid);
    p.set_frequency(1000.0);
    ASSERT_TRUE(p.output().valid);
}

// ============================================================================
// ReceiverPresenter
// ============================================================================

void test_receiver_default_valid() {
    ewpresenter::ReceiverPresenter p;
    ASSERT_TRUE(p.output().valid);
}

void test_receiver_sensitivity() {
    // sensitivity = -174 + 10*log10(bw_hz) + NF + SNR
    //             = -174 + 10*log10(0.1e6) + 6.5 + 15
    //             = -174 + 50 + 6.5 + 15 = -102.5 dBm
    ewpresenter::ReceiverPresenter p;
    ASSERT_NEAR(p.output().sensitivity.value, -102.5, 0.1);
}

void test_receiver_validation() {
    ewpresenter::ReceiverPresenter p;
    p.set_bandwidth(-1.0);
    ASSERT_FALSE(p.output().valid);
    p.set_bandwidth(0.1);
    ASSERT_TRUE(p.output().valid);
}

// ============================================================================
// JammingPresenter
// ============================================================================

void test_jamming_default_valid() {
    ewpresenter::JammingPresenter p;
    ASSERT_TRUE(p.output().valid);
}

void test_jamming_js_value() {
    // Spreadsheet reference: J/S ≈ 36.478 dB at default inputs
    ewpresenter::JammingPresenter p;
    ASSERT_NEAR(p.output().js_ratio.value, 36.478, 0.5);
}

void test_jamming_partial_band_na_when_zero_hop() {
    // hop_range = 0 → non-hopping signal; partial-band shows "N/A", J/S still valid.
    ewpresenter::JammingPresenter p;
    p.set_hop_range(0.0);
    ASSERT_TRUE(p.output().valid);
    ASSERT_TRUE(p.output().optimum_bw_str == "N/A");
    ASSERT_TRUE(p.output().duty_cycle_str == "N/A");
}

// ============================================================================
// DigitalPresenter
// ============================================================================

void test_digital_default_valid() {
    ewpresenter::DigitalPresenter p;
    ASSERT_TRUE(p.output().valid);
}

void test_digital_eb_no_value() {
    // Eb/N₀ = SNR + 10*log10(BW/Rb) = 10 + 10*log10(1.0/0.1) = 10 + 10 = 20 dB
    ewpresenter::DigitalPresenter p;
    ASSERT_NEAR(p.output().eb_no.value, 20.0, 0.01);
}

void test_digital_snr_roundtrip() {
    // snr_from_eb_no should recover the original SNR input (10 dB)
    ewpresenter::DigitalPresenter p;
    ASSERT_NEAR(p.output().snr_from_eb_no.value, 10.0, 0.01);
}

void test_digital_dsss_values() {
    // Process gain = 10*log10(chip/rb) = 10*log10(10/0.1) = 10*log10(100) = 20 dB
    // Jamming margin = PG - req_eb_no - impl_loss = 20 - 10 - 1 = 9 dB
    // Required J/S = -JM = -9 dB
    ewpresenter::DigitalPresenter p;
    ASSERT_NEAR(p.output().process_gain.value,    20.0, 0.01);
    ASSERT_NEAR(p.output().jamming_margin.value,   9.0, 0.01);
    ASSERT_NEAR(p.output().required_js.value,     -9.0, 0.01);
}

void test_digital_validity_split_dsss_invalid() {
    // An invalid chip_rate must not suppress the Eb/N₀ output.
    static constexpr const char* DASH = "\xe2\x80\x94";
    ewpresenter::DigitalPresenter p;

    p.set_chip_rate(-1.0);                              // chip_rate <= 0: invalid
    ASSERT_TRUE(p.output().valid);                      // eb_no_valid is still true
    ASSERT_FALSE(p.output().eb_no_str == DASH);         // Eb/N₀ was computed
    ASSERT_TRUE(p.output().process_gain_str   == "N/A");
    ASSERT_TRUE(p.output().jamming_margin_str == "N/A");
    ASSERT_TRUE(p.output().required_js_str    == "N/A");

    p.set_chip_rate(10.0);                              // restore
    ASSERT_TRUE(p.output().valid);
    ASSERT_FALSE(p.output().process_gain_str == "N/A");
}

void test_digital_validity_split_snr_invalid() {
    // An invalid SNR must suppress all outputs including Eb/N₀.
    static constexpr const char* DASH = "\xe2\x80\x94";
    ewpresenter::DigitalPresenter p;

    p.set_snr(100.0);                                   // exceeds max (60 dB)
    ASSERT_FALSE(p.output().valid);
    ASSERT_TRUE(p.output().eb_no_str        == DASH);
    ASSERT_TRUE(p.output().process_gain_str == DASH);

    p.set_snr(10.0);                                    // restore
    ASSERT_TRUE(p.output().valid);
}

// ============================================================================
// RadarPresenter
// ============================================================================

void test_radar_default_valid() {
    ewpresenter::RadarPresenter p;
    ASSERT_TRUE(p.output().valid);
}

void test_radar_default_range() {
    // Default inputs: Pt=60 dBm, G=30 dBi, σ=0 dBsm, f=3 GHz, L=3 dB,
    //                 NF=5 dB, B=1 MHz, SNR=13 dB → max range ≈ 10.01 km
    ewpresenter::RadarPresenter p;
    ASSERT_NEAR(p.output().max_range.value, 10.01, 0.05);
}

void test_radar_validation() {
    ewpresenter::RadarPresenter p;
    p.set_frequency(-1.0);
    ASSERT_FALSE(p.output().valid);
    p.set_frequency(3000.0);
    ASSERT_TRUE(p.output().valid);
}

// ============================================================================
// LocationPresenter
// ============================================================================

void test_location_default_valid() {
    ewpresenter::LocationPresenter p;
    ASSERT_TRUE(p.output().valid);
}

// ============================================================================
// main
// ============================================================================

int main() {
    std::cout << "=== test_presenters ===\n";

    RUN_TEST(test_propagation_default_valid);
    RUN_TEST(test_propagation_fspl_value);
    RUN_TEST(test_propagation_two_ray_regime);
    RUN_TEST(test_propagation_validation);
    RUN_TEST(test_propagation_obstruction_negative_rejected);
    RUN_TEST(test_propagation_callback);

    RUN_TEST(test_antenna_default_valid);
    RUN_TEST(test_antenna_erp);
    RUN_TEST(test_antenna_wavelength);
    RUN_TEST(test_antenna_validation);

    RUN_TEST(test_receiver_default_valid);
    RUN_TEST(test_receiver_sensitivity);
    RUN_TEST(test_receiver_validation);

    RUN_TEST(test_jamming_default_valid);
    RUN_TEST(test_jamming_js_value);
    RUN_TEST(test_jamming_partial_band_na_when_zero_hop);

    RUN_TEST(test_digital_default_valid);
    RUN_TEST(test_digital_eb_no_value);
    RUN_TEST(test_digital_snr_roundtrip);
    RUN_TEST(test_digital_dsss_values);
    RUN_TEST(test_digital_validity_split_dsss_invalid);
    RUN_TEST(test_digital_validity_split_snr_invalid);

    RUN_TEST(test_radar_default_valid);
    RUN_TEST(test_radar_default_range);
    RUN_TEST(test_radar_validation);

    RUN_TEST(test_location_default_valid);

    return test::summary();
}
