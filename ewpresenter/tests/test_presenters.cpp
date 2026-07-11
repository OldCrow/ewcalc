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

void test_antenna_gain_validation_rejects_out_of_domain_beamwidth() {
    ewpresenter::AntennaPresenter p;

    // The circular beamwidth-from-gain approximation exceeds 360° below
    // about -6.3548 dBi, so the presenter rejects that low-gain range instead
    // of displaying a physically meaningless >360° beamwidth as valid.
    p.set_gain(-10.0);
    ASSERT_FALSE(p.output().valid);
    ASSERT_TRUE(p.gain_error() == ewpresenter::FieldError::below_minimum);

    p.set_gain(-6.35);
    ASSERT_TRUE(p.output().valid);
    ASSERT_TRUE(p.gain_error() == ewpresenter::FieldError::none);
    ASSERT_TRUE(p.output().beamwidth_from_gain.value <= 360.0);
}

// ============================================================================
// ReceiverPresenter
// ============================================================================

void test_receiver_default_valid() {
    ewpresenter::ReceiverPresenter p;
    ASSERT_TRUE(p.output().valid);
}

void test_receiver_sensitivity() {
    // With the default (non-empty) stage chain, sensitivity uses the cascaded
    // NF from the Friis formula rather than the manual NF input (issue #11).
    // Cascaded NF for the default chain {(1,-1),(3,20),(10,-10),(11,0)} dB
    // works out to ~6.111 dB via Friis, vs. the manual NF input of 6.5 dB.
    // sensitivity = -174 + 10*log10(bw_hz) + cascaded_NF + SNR
    //             = -174 + 10*log10(0.1e6) + 6.111 + 15
    //             = -174 + 50 + 6.111 + 15 = -102.89 dBm
    ewpresenter::ReceiverPresenter p;
    ASSERT_NEAR(p.output().sensitivity.value, -102.89, 0.05);
}

void test_receiver_sensitivity_manual_nf_fallback() {
    // With an empty stage chain, sensitivity falls back to the manual NF input.
    // sensitivity = -174 + 10*log10(0.1e6) + 6.5 + 15 = -102.5 dBm
    ewpresenter::ReceiverPresenter p;
    p.set_stages({});
    ASSERT_NEAR(p.output().sensitivity.value, -102.5, 0.05);
}

void test_receiver_sensitivity_uses_cascaded_nf() {
    // A single-stage chain collapses the Friis formula to that stage's own NF,
    // giving an exact expected cascaded_nf independent of the manual NF input.
    ewpresenter::ReceiverPresenter p;
    p.set_stages({ ewpresenter::ReceiverPresenter::StageInput{20.0, 0.0} });
    ASSERT_NEAR(p.output().cascaded_nf.value, 20.0, 0.01);
    // sensitivity = -174 + 50 + 20 + 15 = -89.0 dBm, not the manual-NF value (-102.5).
    ASSERT_NEAR(p.output().sensitivity.value, -89.0, 0.05);
}

void test_receiver_validation() {
    ewpresenter::ReceiverPresenter p;
    p.set_bandwidth(-1.0);
    ASSERT_FALSE(p.output().valid);
    p.set_bandwidth(0.1);
    ASSERT_TRUE(p.output().valid);
}

void test_receiver_noise_figure_below_minimum() {
    // Stage/system NF has a physical minimum of 0 dB (issue #9).
    ewpresenter::ReceiverPresenter p;
    p.set_noise_figure(-1.0);
    ASSERT_TRUE(p.noise_figure_error() == ewpresenter::FieldError::below_minimum);
    ASSERT_FALSE(p.output().valid);
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

void test_digital_required_snr_for_eb_no_value() {
    // required_snr_for_eb_no = required_eb_no - 10*log10(BW/Rb)
    //                        = 10 - 10*log10(1.0/0.1) = 10 - 10 = 0 dB
    ewpresenter::DigitalPresenter p;
    ASSERT_NEAR(p.output().required_snr_for_eb_no.value, 0.0, 0.01);
}

void test_digital_required_snr_for_eb_no_tracks_required_eb_no() {
    // Raising required_eb_no by 5 dB should raise required_snr_for_eb_no by 5 dB,
    // independent of the received SNR input (no identity round-trip with snr_db_).
    ewpresenter::DigitalPresenter p;
    const double before = p.output().required_snr_for_eb_no.value;
    p.set_required_eb_no(15.0);   // default is 10.0
    ASSERT_NEAR(p.output().required_snr_for_eb_no.value, before + 5.0, 0.01);
    p.set_required_eb_no(10.0);   // restore
}

void test_digital_required_snr_for_eb_no_invalid_when_required_eb_no_invalid() {
    // An out-of-range required_eb_no must dash the required-SNR output while
    // leaving the Eb/N₀ section (and overall validity) untouched.
    ewpresenter::DigitalPresenter p;
    p.set_required_eb_no(100.0);   // exceeds max (30 dB)
    ASSERT_TRUE(p.output().valid);
    ASSERT_TRUE(p.output().required_snr_for_eb_no_str == "N/A");
    ASSERT_FALSE(p.output().eb_no_str == "N/A");
    p.set_required_eb_no(10.0);    // restore
    ASSERT_FALSE(p.output().required_snr_for_eb_no_str == "N/A");
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

void test_digital_chip_rate_below_data_rate() {
    // chip_rate < data_rate is physically invalid (negative process gain);
    // DSSS outputs must be N/A; Eb/N₀ section must remain valid.
    ewpresenter::DigitalPresenter p;
    p.set_chip_rate(0.05);   // less than default data_rate (0.1 Mbps)
    ASSERT_TRUE(p.output().valid);                          // Eb/N₀ valid
    ASSERT_TRUE(p.output().process_gain_str   == "N/A");
    ASSERT_TRUE(p.output().jamming_margin_str == "N/A");
    ASSERT_TRUE(p.output().required_js_str    == "N/A");
    p.set_chip_rate(10.0);   // restore
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

void test_location_or_validity_single_section() {
    // valid is an OR across AOA/TDOA/EEP (the sole exception among all 8
    // presenters, documented in AGENTS.md): invalidating every section except
    // one must still leave output().valid true, with only the invalid
    // sections' strings dashed.
    static constexpr const char* DASH = "\xe2\x80\x94";
    ewpresenter::LocationPresenter p;

    // Break AOA (shares aoa_range with TDOA) and TDOA-specific inputs,
    // leaving EEP as the only valid section.
    p.set_rms_bearing_error(-1.0);   // invalid: AOA broken
    p.set_rms_time_error(-1.0);      // invalid: TDOA broken
    ASSERT_TRUE(p.output().valid);   // EEP alone keeps output valid
    ASSERT_TRUE(p.output().cep_aoa_str  == DASH);
    ASSERT_TRUE(p.output().cep_tdoa_str == DASH);
    ASSERT_FALSE(p.output().cep_eep_str == DASH);

    p.set_rms_bearing_error(1.0);    // restore
    p.set_rms_time_error(10.0);      // restore
    ASSERT_TRUE(p.output().valid);
    ASSERT_FALSE(p.output().cep_aoa_str == DASH);
}

void test_location_or_validity_all_sections_invalid() {
    // Only when every section is broken does output().valid go false.
    ewpresenter::LocationPresenter p;
    p.set_rms_bearing_error(-1.0);   // breaks AOA
    p.set_rms_time_error(-1.0);      // breaks TDOA
    p.set_semi_major(-1.0);          // breaks EEP
    ASSERT_FALSE(p.output().valid);

    p.set_rms_bearing_error(1.0);    // restore
    p.set_rms_time_error(10.0);
    p.set_semi_major(2.0);
    ASSERT_TRUE(p.output().valid);
}

void test_location_eep_axis_cross_field_error() {
    // Cross-field check: semi_minor must not exceed semi_major. Both fields
    // individually pass their own bounds validation, so semi_major_error()/
    // semi_minor_error() stay none; only eep_axis_error() flags the violation.
    static constexpr const char* DASH = "\xe2\x80\x94";
    ewpresenter::LocationPresenter p;

    p.set_semi_major(1.0);
    p.set_semi_minor(2.0);   // minor > major: physically invalid ellipse
    ASSERT_TRUE(p.semi_major_error() == ewpresenter::FieldError::none);
    ASSERT_TRUE(p.semi_minor_error() == ewpresenter::FieldError::none);
    ASSERT_TRUE(p.eep_axis_error()   == ewpresenter::FieldError::above_maximum);
    ASSERT_TRUE(p.output().cep_eep_str == DASH);
    // AOA/TDOA are untouched and still valid, so overall validity holds (OR semantics).
    ASSERT_TRUE(p.output().valid);

    p.set_semi_major(2.0);  // restore: major >= minor again
    ASSERT_TRUE(p.eep_axis_error() == ewpresenter::FieldError::none);
    ASSERT_FALSE(p.output().cep_eep_str == DASH);
}

// ============================================================================
// LinkPresenter
// ============================================================================

void test_link_default_valid() {
    ewpresenter::LinkPresenter p;
    ASSERT_TRUE(p.output().valid);
}

void test_link_received_power_value() {
    // Defaults: tx_power=20 dBm, tx_gain=0, rx_gain=0, distance=32.6 km,
    // tx_height=rx_height=10 m, freq=100 MHz, rx_sensitivity=-120.5 dBm.
    // Same geometry as PropagationPresenter's defaults (two-ray regime;
    // see test_propagation_two_ray_regime), so received power sits close
    // to sensitivity by construction (link margin ≈ 0 dB).
    ewpresenter::LinkPresenter p;
    ASSERT_TRUE(p.output().two_ray_regime);
    ASSERT_NEAR(p.output().received_power.value, -120.5, 0.5);
}

void test_link_validation() {
    ewpresenter::LinkPresenter p;
    p.set_distance(-1.0);
    ASSERT_FALSE(p.output().valid);
    ASSERT_TRUE(p.distance_error() != ewpresenter::FieldError::none);
    p.set_distance(32.6);
    ASSERT_TRUE(p.output().valid);
    ASSERT_TRUE(p.distance_error() == ewpresenter::FieldError::none);
}

void test_link_margin_tracks_sensitivity() {
    // link_margin = received_power - rx_sensitivity; received power itself
    // depends only on tx_power/gains/geometry, not on rx_sensitivity_dbm_, so
    // raising the sensitivity requirement must change the formatted margin
    // string while leaving received_power untouched.
    ewpresenter::LinkPresenter p;
    const double received_before = p.output().received_power.value;
    const std::string margin_before = p.output().link_margin_str;

    p.set_rx_sensitivity(-100.5);   // 20 dB above default -120.5
    ASSERT_NEAR(p.output().received_power.value, received_before, 0.001);
    ASSERT_TRUE(p.output().link_margin_str != margin_before);
    ASSERT_TRUE(p.output().valid);

    p.set_rx_sensitivity(-120.5);   // restore
}

void test_link_callback() {
    ewpresenter::LinkPresenter p;
    int fire_count = 0;
    p.set_on_change([&](const ewpresenter::LinkPresenter::Output&) { ++fire_count; });
    p.set_tx_power(25.0);
    p.set_distance(20.0);
    ASSERT_TRUE(fire_count == 2);
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
    RUN_TEST(test_antenna_gain_validation_rejects_out_of_domain_beamwidth);

    RUN_TEST(test_receiver_default_valid);
    RUN_TEST(test_receiver_sensitivity);
    RUN_TEST(test_receiver_sensitivity_manual_nf_fallback);
    RUN_TEST(test_receiver_sensitivity_uses_cascaded_nf);
    RUN_TEST(test_receiver_validation);
    RUN_TEST(test_receiver_noise_figure_below_minimum);

    RUN_TEST(test_jamming_default_valid);
    RUN_TEST(test_jamming_js_value);
    RUN_TEST(test_jamming_partial_band_na_when_zero_hop);

    RUN_TEST(test_digital_default_valid);
    RUN_TEST(test_digital_eb_no_value);
    RUN_TEST(test_digital_required_snr_for_eb_no_value);
    RUN_TEST(test_digital_required_snr_for_eb_no_tracks_required_eb_no);
    RUN_TEST(test_digital_required_snr_for_eb_no_invalid_when_required_eb_no_invalid);
    RUN_TEST(test_digital_dsss_values);
    RUN_TEST(test_digital_validity_split_dsss_invalid);
    RUN_TEST(test_digital_chip_rate_below_data_rate);
    RUN_TEST(test_digital_validity_split_snr_invalid);

    RUN_TEST(test_radar_default_valid);
    RUN_TEST(test_radar_default_range);
    RUN_TEST(test_radar_validation);

    RUN_TEST(test_location_default_valid);
    RUN_TEST(test_location_or_validity_single_section);
    RUN_TEST(test_location_or_validity_all_sections_invalid);
    RUN_TEST(test_location_eep_axis_cross_field_error);

    RUN_TEST(test_link_default_valid);
    RUN_TEST(test_link_received_power_value);
    RUN_TEST(test_link_validation);
    RUN_TEST(test_link_margin_tracks_sensitivity);
    RUN_TEST(test_link_callback);

    return test::summary();
}
