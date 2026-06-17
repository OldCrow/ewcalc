/// @file test_bridge.cpp
/// @brief Integration tests for the C bridge layer (ewcalc_bridge.h/cpp).
///
/// Covers for each presenter:
///   - Default state is valid.
///   - Setting an out-of-range field marks output invalid and sets the correct
///     EwpFieldError on the per-field error accessor.
///   - Restoring a valid value clears both.
///   - Callback fires exactly once per setter call.

#include "test_main.h"
#include "ewcalc_bridge.h"

// ============================================================================
// PropagationPresenter
// ============================================================================

void test_propagation_defaults_valid() {
    EwpPropagationRef ref = ewp_propagation_create();
    ASSERT_TRUE(ewp_propagation_output(ref).valid);
    ASSERT_TRUE(ewp_propagation_distance_error(ref) == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_propagation_frequency_error(ref) == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_propagation_tx_height_error(ref) == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_propagation_rx_height_error(ref) == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_propagation_obstruction_height_error(ref) == EWP_FIELD_OK);
    ewp_propagation_destroy(ref);
}

void test_propagation_invalid_distance() {
    EwpPropagationRef ref = ewp_propagation_create();
    // Use a sub-minimum positive value (min = 0.01 km) to get BELOW_MINIMUM.
    ewp_propagation_set_distance(ref, 0.001);
    ASSERT_FALSE(ewp_propagation_output(ref).valid);
    ASSERT_TRUE(ewp_propagation_distance_error(ref) == EWP_FIELD_BELOW_MINIMUM);
    ewp_propagation_set_distance(ref, 32.6);
    ASSERT_TRUE(ewp_propagation_output(ref).valid);
    ASSERT_TRUE(ewp_propagation_distance_error(ref) == EWP_FIELD_OK);
    ewp_propagation_destroy(ref);
}

void test_propagation_callback() {
    EwpPropagationRef ref = ewp_propagation_create();
    int count = 0;
    ewp_propagation_set_callback(ref,
        [](EwpPropagationOutput, void* ctx) { ++(*static_cast<int*>(ctx)); },
        &count);
    ewp_propagation_set_distance(ref, 20.0);
    ewp_propagation_set_frequency(ref, 200.0);
    ASSERT_TRUE(count == 2);
    ewp_propagation_destroy(ref);
}

// ============================================================================
// LinkPresenter
// ============================================================================

void test_link_defaults_valid() {
    EwpLinkRef ref = ewp_link_create();
    ASSERT_TRUE(ewp_link_output(ref).valid);
    ASSERT_TRUE(ewp_link_tx_power_error(ref)      == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_link_tx_gain_error(ref)        == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_link_rx_gain_error(ref)        == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_link_distance_error(ref)       == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_link_tx_height_error(ref)      == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_link_rx_height_error(ref)      == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_link_frequency_error(ref)      == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_link_rx_sensitivity_error(ref) == EWP_FIELD_OK);
    ewp_link_destroy(ref);
}

void test_link_invalid_frequency() {
    EwpLinkRef ref = ewp_link_create();
    // Use a sub-minimum positive value (min = 0.1 MHz) to get BELOW_MINIMUM.
    ewp_link_set_frequency(ref, 0.05);
    ASSERT_FALSE(ewp_link_output(ref).valid);
    ASSERT_TRUE(ewp_link_frequency_error(ref) == EWP_FIELD_BELOW_MINIMUM);
    ewp_link_set_frequency(ref, 100.0);
    ASSERT_TRUE(ewp_link_output(ref).valid);
    ewp_link_destroy(ref);
}

// ============================================================================
// ReceiverPresenter
// ============================================================================

void test_receiver_defaults_valid() {
    EwpReceiverRef ref = ewp_receiver_create();
    ASSERT_TRUE(ewp_receiver_output(ref).valid);
    ASSERT_TRUE(ewp_receiver_bandwidth_error(ref)       == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_receiver_noise_figure_error(ref)    == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_receiver_required_snr_error(ref)    == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_receiver_second_order_ip_error(ref) == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_receiver_third_order_ip_error(ref)  == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_receiver_adc_bits_error(ref)        == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_receiver_stage_nf_error(ref)        == EWP_FIELD_OK);
    ewp_receiver_destroy(ref);
}

void test_receiver_invalid_adc_bits() {
    EwpReceiverRef ref = ewp_receiver_create();
    ewp_receiver_set_adc_bits(ref, 0);
    ASSERT_FALSE(ewp_receiver_output(ref).valid);
    ASSERT_TRUE(ewp_receiver_adc_bits_error(ref) == EWP_FIELD_BELOW_MINIMUM);
    ewp_receiver_set_adc_bits(ref, 12);
    ASSERT_TRUE(ewp_receiver_output(ref).valid);
    ewp_receiver_destroy(ref);
}

void test_receiver_stage_negative_nf() {
    EwpReceiverRef ref = ewp_receiver_create();
    // Inject a stage with physically impossible NF
    EwpStageInput bad_stage = {-5.0, 20.0};
    ewp_receiver_set_stages(ref, &bad_stage, 1);
    ASSERT_FALSE(ewp_receiver_output(ref).valid);
    ASSERT_TRUE(ewp_receiver_stage_nf_error(ref) == EWP_FIELD_INVALID_NEGATIVE);
    // Restore a valid stage
    EwpStageInput good_stage = {3.0, 20.0};
    ewp_receiver_set_stages(ref, &good_stage, 1);
    ASSERT_TRUE(ewp_receiver_output(ref).valid);
    ewp_receiver_destroy(ref);
}

// ============================================================================
// JammingPresenter
// ============================================================================

void test_jamming_defaults_valid() {
    EwpJammingRef ref = ewp_jamming_create();
    ASSERT_TRUE(ewp_jamming_output(ref).valid);
    ASSERT_TRUE(ewp_jamming_signal_erp_error(ref)        == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_jamming_jammer_erp_error(ref)        == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_jamming_signal_dist_error(ref)       == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_jamming_jammer_dist_error(ref)       == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_jamming_signal_height_error(ref)     == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_jamming_jammer_height_error(ref)     == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_jamming_rx_height_error(ref)         == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_jamming_frequency_error(ref)         == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_jamming_rx_gain_signal_error(ref)    == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_jamming_rx_gain_jammer_error(ref)    == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_jamming_signal_bandwidth_error(ref)  == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_jamming_hop_range_error(ref)         == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_jamming_js_threshold_error(ref)      == EWP_FIELD_OK);
    ewp_jamming_destroy(ref);
}

void test_jamming_invalid_rx_gain() {
    EwpJammingRef ref = ewp_jamming_create();
    // Gain too large
    ewp_jamming_set_rx_gain_signal(ref, 999.0);
    ASSERT_FALSE(ewp_jamming_output(ref).valid);
    ASSERT_TRUE(ewp_jamming_rx_gain_signal_error(ref) == EWP_FIELD_ABOVE_MAXIMUM);
    ewp_jamming_set_rx_gain_signal(ref, 0.0);
    ASSERT_TRUE(ewp_jamming_output(ref).valid);
    ewp_jamming_destroy(ref);
}

// ============================================================================
// LocationPresenter
// ============================================================================

void test_location_defaults_valid() {
    EwpLocationRef ref = ewp_location_create();
    ASSERT_TRUE(ewp_location_output(ref).valid);
    ASSERT_TRUE(ewp_location_rms_bearing_field_error(ref) == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_location_aoa_range_error(ref)         == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_location_rms_time_field_error(ref)    == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_location_baseline_error(ref)          == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_location_semi_major_error(ref)        == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_location_semi_minor_error(ref)        == EWP_FIELD_OK);
    ewp_location_destroy(ref);
}

void test_location_invalid_bearing_error() {
    EwpLocationRef ref = ewp_location_create();
    ewp_location_set_rms_bearing_error(ref, 0.0);   // must be > 0
    ASSERT_FALSE(ewp_location_output(ref).valid);
    ASSERT_TRUE(ewp_location_rms_bearing_field_error(ref) == EWP_FIELD_INVALID_ZERO);
    ewp_location_set_rms_bearing_error(ref, 1.0);
    ASSERT_TRUE(ewp_location_output(ref).valid);
    ewp_location_destroy(ref);
}

// ============================================================================
// RadarPresenter
// ============================================================================

void test_radar_defaults_valid() {
    EwpRadarRef ref = ewp_radar_create();
    ASSERT_TRUE(ewp_radar_output(ref).valid);
    ASSERT_TRUE(ewp_radar_tx_power_error(ref)      == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_radar_antenna_gain_error(ref)  == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_radar_target_rcs_error(ref)    == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_radar_frequency_error(ref)     == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_radar_system_losses_error(ref) == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_radar_noise_figure_error(ref)  == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_radar_bandwidth_error(ref)     == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_radar_required_snr_error(ref)  == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_radar_time_bandwidth_error(ref)== EWP_FIELD_OK);
    ASSERT_TRUE(ewp_radar_num_pulses_error(ref)    == EWP_FIELD_OK);
    ewp_radar_destroy(ref);
}

void test_radar_invalid_num_pulses() {
    EwpRadarRef ref = ewp_radar_create();
    ewp_radar_set_num_pulses(ref, 0);
    ASSERT_FALSE(ewp_radar_output(ref).valid);
    ASSERT_TRUE(ewp_radar_num_pulses_error(ref) == EWP_FIELD_BELOW_MINIMUM);
    ewp_radar_set_num_pulses(ref, 16);
    ASSERT_TRUE(ewp_radar_output(ref).valid);
    ewp_radar_destroy(ref);
}

void test_radar_output_field_names() {
    // Confirms the renamed struct fields compile and can be read.
    EwpRadarRef ref = ewp_radar_create();
    EwpRadarOutput out = ewp_radar_output(ref);
    // Access renamed fields; if the old names existed this would fail to compile.
    ASSERT_TRUE(out.pulse_compression_gain_str[0] != '\0');
    ASSERT_TRUE(out.coherent_integration_gain_str[0] != '\0');
    ewp_radar_destroy(ref);
}

// ============================================================================
// DigitalPresenter
// ============================================================================

void test_digital_defaults_valid() {
    EwpDigitalRef ref = ewp_digital_create();
    ASSERT_TRUE(ewp_digital_output(ref).valid);
    ASSERT_TRUE(ewp_digital_snr_error(ref)                  == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_digital_bandwidth_error(ref)            == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_digital_data_rate_error(ref)            == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_digital_chip_rate_error(ref)            == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_digital_required_eb_no_error(ref)       == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_digital_implementation_loss_error(ref)  == EWP_FIELD_OK);
    ewp_digital_destroy(ref);
}

void test_digital_invalid_snr() {
    EwpDigitalRef ref = ewp_digital_create();
    ewp_digital_set_snr(ref, 999.0);
    ASSERT_FALSE(ewp_digital_output(ref).valid);
    ASSERT_TRUE(ewp_digital_snr_error(ref) == EWP_FIELD_ABOVE_MAXIMUM);
    ewp_digital_set_snr(ref, 10.0);
    ASSERT_TRUE(ewp_digital_output(ref).valid);
    ewp_digital_destroy(ref);
}

// ============================================================================
// AntennaPresenter
// ============================================================================

void test_antenna_defaults_valid() {
    EwpAntennaRef ref = ewp_antenna_create();
    ASSERT_TRUE(ewp_antenna_output(ref).valid);
    ASSERT_TRUE(ewp_antenna_gain_error(ref)          == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_antenna_az_beamwidth_error(ref)  == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_antenna_el_beamwidth_error(ref)  == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_antenna_tx_power_error(ref)      == EWP_FIELD_OK);
    ASSERT_TRUE(ewp_antenna_frequency_error(ref)     == EWP_FIELD_OK);
    ewp_antenna_destroy(ref);
}

void test_antenna_invalid_frequency() {
    EwpAntennaRef ref = ewp_antenna_create();
    // Use a sub-minimum positive value (min = 0.1 MHz) to get BELOW_MINIMUM.
    ewp_antenna_set_frequency(ref, 0.05);
    ASSERT_FALSE(ewp_antenna_output(ref).valid);
    ASSERT_TRUE(ewp_antenna_frequency_error(ref) == EWP_FIELD_BELOW_MINIMUM);
    ewp_antenna_set_frequency(ref, 1000.0);
    ASSERT_TRUE(ewp_antenna_output(ref).valid);
    ewp_antenna_destroy(ref);
}

// ============================================================================
// main
// ============================================================================

int main() {
    std::cout << "=== test_bridge ===\n";

    RUN_TEST(test_propagation_defaults_valid);
    RUN_TEST(test_propagation_invalid_distance);
    RUN_TEST(test_propagation_callback);

    RUN_TEST(test_link_defaults_valid);
    RUN_TEST(test_link_invalid_frequency);

    RUN_TEST(test_receiver_defaults_valid);
    RUN_TEST(test_receiver_invalid_adc_bits);
    RUN_TEST(test_receiver_stage_negative_nf);

    RUN_TEST(test_jamming_defaults_valid);
    RUN_TEST(test_jamming_invalid_rx_gain);

    RUN_TEST(test_location_defaults_valid);
    RUN_TEST(test_location_invalid_bearing_error);

    RUN_TEST(test_radar_defaults_valid);
    RUN_TEST(test_radar_invalid_num_pulses);
    RUN_TEST(test_radar_output_field_names);

    RUN_TEST(test_digital_defaults_valid);
    RUN_TEST(test_digital_invalid_snr);

    RUN_TEST(test_antenna_defaults_valid);
    RUN_TEST(test_antenna_invalid_frequency);

    return test::summary();
}
