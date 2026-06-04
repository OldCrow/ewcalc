#pragma once
/// @file ewcalc_bridge.h
/// @brief C bridge between ewpresenter (C++20) and Swift.
///
/// Exposes a plain-C API: opaque handles, value-type output structs with
/// fixed-size string fields, and C function-pointer callbacks.  Swift can
/// import this header directly via a bridging header; no Obj-C++ required.

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Maximum length of any formatted result string (including null terminator).
#define EWP_STR_MAX 80

// ============================================================================
// Propagation
// ============================================================================

typedef struct EwpPropagationOutput {
    char fspl_str[EWP_STR_MAX];
    char two_ray_loss_str[EWP_STR_MAX];
    char fresnel_zone_str[EWP_STR_MAX];
    char path_loss_str[EWP_STR_MAX];
    char regime_str[EWP_STR_MAX];
    char earth_bulge_str[EWP_STR_MAX];
    char horizon_range_str[EWP_STR_MAX];
    char diffraction_loss_str[EWP_STR_MAX];
    bool valid;
} EwpPropagationOutput;

typedef void* EwpPropagationRef;
typedef void (*EwpPropagationCallback)(EwpPropagationOutput output, void* ctx);

EwpPropagationRef ewp_propagation_create(void);
void              ewp_propagation_destroy(EwpPropagationRef ref);

void ewp_propagation_set_distance(EwpPropagationRef ref, double km);
void ewp_propagation_set_frequency(EwpPropagationRef ref, double mhz);
void ewp_propagation_set_tx_height(EwpPropagationRef ref, double m);
void ewp_propagation_set_rx_height(EwpPropagationRef ref, double m);
void ewp_propagation_set_obstruction_height(EwpPropagationRef ref, double m);
void ewp_propagation_set_callback(EwpPropagationRef ref, EwpPropagationCallback cb, void* ctx);

double               ewp_propagation_distance(EwpPropagationRef ref);
double               ewp_propagation_frequency(EwpPropagationRef ref);
double               ewp_propagation_tx_height(EwpPropagationRef ref);
double               ewp_propagation_rx_height(EwpPropagationRef ref);
double               ewp_propagation_obstruction_height(EwpPropagationRef ref);
EwpPropagationOutput ewp_propagation_output(EwpPropagationRef ref);

// ============================================================================
// Link Budget
// ============================================================================

typedef struct EwpLinkOutput {
    char received_power_str[EWP_STR_MAX];
    char path_loss_str[EWP_STR_MAX];
    char fresnel_zone_str[EWP_STR_MAX];
    char regime_str[EWP_STR_MAX];
    char link_margin_str[EWP_STR_MAX];
    char effective_range_str[EWP_STR_MAX];
    char range_regime_str[EWP_STR_MAX];
    bool valid;
} EwpLinkOutput;

typedef void* EwpLinkRef;
typedef void (*EwpLinkCallback)(EwpLinkOutput output, void* ctx);

EwpLinkRef ewp_link_create(void);
void       ewp_link_destroy(EwpLinkRef ref);

void ewp_link_set_tx_power(EwpLinkRef ref, double dbm);
void ewp_link_set_tx_gain(EwpLinkRef ref, double db);
void ewp_link_set_rx_gain(EwpLinkRef ref, double db);
void ewp_link_set_distance(EwpLinkRef ref, double km);
void ewp_link_set_tx_height(EwpLinkRef ref, double m);
void ewp_link_set_rx_height(EwpLinkRef ref, double m);
void ewp_link_set_frequency(EwpLinkRef ref, double mhz);
void ewp_link_set_rx_sensitivity(EwpLinkRef ref, double dbm);
void ewp_link_set_callback(EwpLinkRef ref, EwpLinkCallback cb, void* ctx);

double      ewp_link_tx_power(EwpLinkRef ref);
double      ewp_link_tx_gain(EwpLinkRef ref);
double      ewp_link_rx_gain(EwpLinkRef ref);
double      ewp_link_distance(EwpLinkRef ref);
double      ewp_link_tx_height(EwpLinkRef ref);
double      ewp_link_rx_height(EwpLinkRef ref);
double      ewp_link_frequency(EwpLinkRef ref);
double      ewp_link_rx_sensitivity(EwpLinkRef ref);
EwpLinkOutput ewp_link_output(EwpLinkRef ref);

// ============================================================================
// Receiver
// ============================================================================

typedef struct EwpStageInput {
    double noise_figure_db;
    double gain_db;
} EwpStageInput;

typedef struct EwpReceiverOutput {
    char sensitivity_str[EWP_STR_MAX];
    char cascaded_nf_str[EWP_STR_MAX];
    char sfdr2_str[EWP_STR_MAX];
    char sfdr3_str[EWP_STR_MAX];
    char digital_dr_str[EWP_STR_MAX];
    char system_noise_temp_str[EWP_STR_MAX];
    bool valid;
} EwpReceiverOutput;

typedef void* EwpReceiverRef;
typedef void (*EwpReceiverCallback)(EwpReceiverOutput output, void* ctx);

EwpReceiverRef ewp_receiver_create(void);
void           ewp_receiver_destroy(EwpReceiverRef ref);

void ewp_receiver_set_bandwidth(EwpReceiverRef ref, double mhz);
void ewp_receiver_set_noise_figure(EwpReceiverRef ref, double db);
void ewp_receiver_set_required_snr(EwpReceiverRef ref, double db);
void ewp_receiver_set_second_order_ip(EwpReceiverRef ref, double dbm);
void ewp_receiver_set_third_order_ip(EwpReceiverRef ref, double dbm);
void ewp_receiver_set_adc_bits(EwpReceiverRef ref, int bits);
/// Replace the full stage chain. `stages` must remain valid only for the
/// duration of this call; the bridge copies the data.
void ewp_receiver_set_stages(EwpReceiverRef ref, const EwpStageInput* stages, int count);
void ewp_receiver_set_callback(EwpReceiverRef ref, EwpReceiverCallback cb, void* ctx);

double         ewp_receiver_bandwidth(EwpReceiverRef ref);
double         ewp_receiver_noise_figure(EwpReceiverRef ref);
double         ewp_receiver_required_snr(EwpReceiverRef ref);
double         ewp_receiver_second_order_ip(EwpReceiverRef ref);
double         ewp_receiver_third_order_ip(EwpReceiverRef ref);
int            ewp_receiver_adc_bits(EwpReceiverRef ref);
int            ewp_receiver_stage_count(EwpReceiverRef ref);
EwpStageInput  ewp_receiver_stage(EwpReceiverRef ref, int index);
EwpReceiverOutput ewp_receiver_output(EwpReceiverRef ref);

// ============================================================================
// Jamming
// ============================================================================

typedef struct EwpJammingOutput {
    char js_ratio_str[EWP_STR_MAX];
    char signal_at_rx_str[EWP_STR_MAX];
    char jammer_at_rx_str[EWP_STR_MAX];
    char optimum_bw_str[EWP_STR_MAX];
    char duty_cycle_str[EWP_STR_MAX];
    char burnthrough_range_str[EWP_STR_MAX];
    bool valid;
} EwpJammingOutput;

typedef void* EwpJammingRef;
typedef void (*EwpJammingCallback)(EwpJammingOutput output, void* ctx);

EwpJammingRef ewp_jamming_create(void);
void          ewp_jamming_destroy(EwpJammingRef ref);

void ewp_jamming_set_signal_erp(EwpJammingRef ref, double dbm);
void ewp_jamming_set_jammer_erp(EwpJammingRef ref, double dbm);
void ewp_jamming_set_signal_dist(EwpJammingRef ref, double km);
void ewp_jamming_set_jammer_dist(EwpJammingRef ref, double km);
void ewp_jamming_set_signal_height(EwpJammingRef ref, double m);
void ewp_jamming_set_jammer_height(EwpJammingRef ref, double m);
void ewp_jamming_set_rx_height(EwpJammingRef ref, double m);
void ewp_jamming_set_frequency(EwpJammingRef ref, double mhz);
void ewp_jamming_set_rx_gain_signal(EwpJammingRef ref, double db);
void ewp_jamming_set_rx_gain_jammer(EwpJammingRef ref, double db);
void ewp_jamming_set_signal_bandwidth(EwpJammingRef ref, double mhz);
void ewp_jamming_set_hop_range(EwpJammingRef ref, double mhz);
void ewp_jamming_set_js_threshold(EwpJammingRef ref, double db);
void ewp_jamming_set_callback(EwpJammingRef ref, EwpJammingCallback cb, void* ctx);

double        ewp_jamming_signal_erp(EwpJammingRef ref);
double        ewp_jamming_jammer_erp(EwpJammingRef ref);
double        ewp_jamming_signal_dist(EwpJammingRef ref);
double        ewp_jamming_jammer_dist(EwpJammingRef ref);
double        ewp_jamming_signal_height(EwpJammingRef ref);
double        ewp_jamming_jammer_height(EwpJammingRef ref);
double        ewp_jamming_rx_height(EwpJammingRef ref);
double        ewp_jamming_frequency(EwpJammingRef ref);
double        ewp_jamming_signal_bandwidth(EwpJammingRef ref);
double        ewp_jamming_hop_range(EwpJammingRef ref);
double        ewp_jamming_js_threshold(EwpJammingRef ref);
EwpJammingOutput ewp_jamming_output(EwpJammingRef ref);

// ============================================================================
// Location
// ============================================================================

typedef struct EwpLocationOutput {
    char cep_aoa_str[EWP_STR_MAX];
    char cep_tdoa_str[EWP_STR_MAX];
    char cep_eep_str[EWP_STR_MAX];
    bool valid;
} EwpLocationOutput;

typedef void* EwpLocationRef;
typedef void (*EwpLocationCallback)(EwpLocationOutput output, void* ctx);

EwpLocationRef ewp_location_create(void);
void           ewp_location_destroy(EwpLocationRef ref);

void ewp_location_set_rms_bearing_error(EwpLocationRef ref, double deg);
void ewp_location_set_aoa_range(EwpLocationRef ref, double km);
void ewp_location_set_rms_time_error(EwpLocationRef ref, double ns);
void ewp_location_set_baseline(EwpLocationRef ref, double km);
void ewp_location_set_semi_major(EwpLocationRef ref, double km);
void ewp_location_set_semi_minor(EwpLocationRef ref, double km);
void ewp_location_set_callback(EwpLocationRef ref, EwpLocationCallback cb, void* ctx);

double         ewp_location_rms_bearing_error(EwpLocationRef ref);
double         ewp_location_aoa_range(EwpLocationRef ref);
double         ewp_location_rms_time_error(EwpLocationRef ref);
double         ewp_location_baseline(EwpLocationRef ref);
double         ewp_location_semi_major(EwpLocationRef ref);
double         ewp_location_semi_minor(EwpLocationRef ref);
EwpLocationOutput ewp_location_output(EwpLocationRef ref);

// ============================================================================
// Radar
// ============================================================================

typedef struct EwpRadarOutput {
    char max_range_str[EWP_STR_MAX];
    char two_way_loss_str[EWP_STR_MAX];
    char pc_gain_str[EWP_STR_MAX];
    char ci_gain_str[EWP_STR_MAX];
    char lpi_advantage_str[EWP_STR_MAX];
    bool valid;
} EwpRadarOutput;

typedef void* EwpRadarRef;
typedef void (*EwpRadarCallback)(EwpRadarOutput output, void* ctx);

EwpRadarRef ewp_radar_create(void);
void        ewp_radar_destroy(EwpRadarRef ref);

void ewp_radar_set_tx_power(EwpRadarRef ref, double dbm);
void ewp_radar_set_antenna_gain(EwpRadarRef ref, double dbi);
void ewp_radar_set_target_rcs(EwpRadarRef ref, double dbsm);
void ewp_radar_set_frequency(EwpRadarRef ref, double mhz);
void ewp_radar_set_system_losses(EwpRadarRef ref, double db);
void ewp_radar_set_noise_figure(EwpRadarRef ref, double db);
void ewp_radar_set_bandwidth(EwpRadarRef ref, double mhz);
void ewp_radar_set_required_snr(EwpRadarRef ref, double db);
void ewp_radar_set_time_bandwidth(EwpRadarRef ref, double tb);
void ewp_radar_set_num_pulses(EwpRadarRef ref, int n);
void ewp_radar_set_callback(EwpRadarRef ref, EwpRadarCallback cb, void* ctx);

double        ewp_radar_tx_power(EwpRadarRef ref);
double        ewp_radar_antenna_gain(EwpRadarRef ref);
double        ewp_radar_target_rcs(EwpRadarRef ref);
double        ewp_radar_frequency(EwpRadarRef ref);
double        ewp_radar_system_losses(EwpRadarRef ref);
double        ewp_radar_noise_figure(EwpRadarRef ref);
double        ewp_radar_bandwidth(EwpRadarRef ref);
double        ewp_radar_required_snr(EwpRadarRef ref);
double        ewp_radar_time_bandwidth(EwpRadarRef ref);
int           ewp_radar_num_pulses(EwpRadarRef ref);
EwpRadarOutput ewp_radar_output(EwpRadarRef ref);

// ============================================================================
// Digital / Spread Spectrum
// ============================================================================

typedef struct EwpDigitalOutput {
    char eb_no_str[EWP_STR_MAX];
    char snr_from_eb_no_str[EWP_STR_MAX];
    char process_gain_str[EWP_STR_MAX];
    char jamming_margin_str[EWP_STR_MAX];
    char required_js_str[EWP_STR_MAX];
    bool valid;
} EwpDigitalOutput;

typedef void* EwpDigitalRef;
typedef void (*EwpDigitalCallback)(EwpDigitalOutput output, void* ctx);

EwpDigitalRef ewp_digital_create(void);
void          ewp_digital_destroy(EwpDigitalRef ref);

void ewp_digital_set_snr(EwpDigitalRef ref, double db);
void ewp_digital_set_bandwidth(EwpDigitalRef ref, double mhz);
void ewp_digital_set_data_rate(EwpDigitalRef ref, double mbps);
void ewp_digital_set_chip_rate(EwpDigitalRef ref, double mcps);
void ewp_digital_set_required_eb_no(EwpDigitalRef ref, double db);
void ewp_digital_set_implementation_loss(EwpDigitalRef ref, double db);
void ewp_digital_set_callback(EwpDigitalRef ref, EwpDigitalCallback cb, void* ctx);

double           ewp_digital_snr(EwpDigitalRef ref);
double           ewp_digital_bandwidth(EwpDigitalRef ref);
double           ewp_digital_data_rate(EwpDigitalRef ref);
double           ewp_digital_chip_rate(EwpDigitalRef ref);
double           ewp_digital_required_eb_no(EwpDigitalRef ref);
double           ewp_digital_implementation_loss(EwpDigitalRef ref);
EwpDigitalOutput ewp_digital_output(EwpDigitalRef ref);

#ifdef __cplusplus
}
#endif
