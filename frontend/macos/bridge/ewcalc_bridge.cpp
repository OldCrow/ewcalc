// ewcalc_bridge.cpp
// C bridge implementation. Each presenter is wrapped in a struct that owns
// the C++ presenter object plus the registered C callback + context pointer.

#include "ewcalc_bridge.h"

#include <ewpresenter/propagation_presenter.h>
#include <ewpresenter/link_presenter.h>
#include <ewpresenter/receiver_presenter.h>
#include <ewpresenter/jamming_presenter.h>
#include <ewpresenter/location_presenter.h>
#include <ewpresenter/radar_presenter.h>

#include <cstring>
#include <vector>

namespace {

// Copy a std::string into a fixed-size C char array, always null-terminating.
template<std::size_t N>
void copy_str(char (&dst)[N], const std::string& src) noexcept {
    std::size_t n = std::min(src.size(), N - 1);
    std::memcpy(dst, src.data(), n);
    dst[n] = '\0';
}

// ── Propagation ──────────────────────────────────────────────────────────────

struct PropagationWrapper {
    ewpresenter::PropagationPresenter presenter;
    EwpPropagationCallback cb = nullptr;
    void* ctx = nullptr;
};

static EwpPropagationOutput to_c(const ewpresenter::PropagationPresenter::Output& o) noexcept {
    EwpPropagationOutput out{};
    copy_str(out.fspl_str,        o.fspl_str);
    copy_str(out.two_ray_loss_str,o.two_ray_loss_str);
    copy_str(out.fresnel_zone_str,o.fresnel_zone_str);
    copy_str(out.path_loss_str,   o.path_loss_str);
    copy_str(out.regime_str,      o.regime_str);
    out.valid = o.valid;
    return out;
}

// ── Link ─────────────────────────────────────────────────────────────────────

struct LinkWrapper {
    ewpresenter::LinkPresenter presenter;
    EwpLinkCallback cb = nullptr;
    void* ctx = nullptr;
};

static EwpLinkOutput to_c(const ewpresenter::LinkPresenter::Output& o) noexcept {
    EwpLinkOutput out{};
    copy_str(out.received_power_str,  o.received_power_str);
    copy_str(out.path_loss_str,       o.path_loss_str);
    copy_str(out.fresnel_zone_str,    o.fresnel_zone_str);
    copy_str(out.regime_str,          o.regime_str);
    copy_str(out.link_margin_str,     o.link_margin_str);
    copy_str(out.effective_range_str, o.effective_range_str);
    copy_str(out.range_regime_str,    o.range_regime_str);
    out.valid = o.valid;
    return out;
}

// ── Receiver ─────────────────────────────────────────────────────────────────

struct ReceiverWrapper {
    ewpresenter::ReceiverPresenter presenter;
    EwpReceiverCallback cb = nullptr;
    void* ctx = nullptr;
};

static EwpReceiverOutput to_c(const ewpresenter::ReceiverPresenter::Output& o) noexcept {
    EwpReceiverOutput out{};
    copy_str(out.sensitivity_str, o.sensitivity_str);
    copy_str(out.cascaded_nf_str, o.cascaded_nf_str);
    copy_str(out.sfdr2_str,       o.sfdr2_str);
    copy_str(out.sfdr3_str,       o.sfdr3_str);
    copy_str(out.digital_dr_str,  o.digital_dr_str);
    out.valid = o.valid;
    return out;
}

// ── Jamming ──────────────────────────────────────────────────────────────────

struct JammingWrapper {
    ewpresenter::JammingPresenter presenter;
    EwpJammingCallback cb = nullptr;
    void* ctx = nullptr;
};

static EwpJammingOutput to_c(const ewpresenter::JammingPresenter::Output& o) noexcept {
    EwpJammingOutput out{};
    copy_str(out.js_ratio_str,     o.js_ratio_str);
    copy_str(out.signal_at_rx_str, o.signal_at_rx_str);
    copy_str(out.jammer_at_rx_str, o.jammer_at_rx_str);
    copy_str(out.optimum_bw_str,   o.optimum_bw_str);
    copy_str(out.duty_cycle_str,   o.duty_cycle_str);
    out.valid = o.valid;
    return out;
}

// ── Location ─────────────────────────────────────────────────────────────────

struct LocationWrapper {
    ewpresenter::LocationPresenter presenter;
    EwpLocationCallback cb = nullptr;
    void* ctx = nullptr;
};

static EwpLocationOutput to_c(const ewpresenter::LocationPresenter::Output& o) noexcept {
    EwpLocationOutput out{};
    copy_str(out.cep_aoa_str, o.cep_aoa_str);
    copy_str(out.cep_eep_str, o.cep_eep_str);
    out.valid = o.valid;
    return out;
}

// ── Radar ────────────────────────────────────────────────────────────────────

struct RadarWrapper {
    ewpresenter::RadarPresenter presenter;
    EwpRadarCallback cb = nullptr;
    void* ctx = nullptr;
};

static EwpRadarOutput to_c(const ewpresenter::RadarPresenter::Output& o) noexcept {
    EwpRadarOutput out{};
    copy_str(out.max_range_str,    o.max_range_str);
    copy_str(out.two_way_loss_str, o.two_way_loss_str);
    copy_str(out.pc_gain_str,      o.pulse_compression_gain_str);
    copy_str(out.ci_gain_str,      o.coherent_integration_gain_str);
    out.valid = o.valid;
    return out;
}

// Helper: cast opaque ref to wrapper pointer
template<typename W>
W* cast(void* ref) noexcept { return static_cast<W*>(ref); }

} // namespace

// ============================================================================
// Propagation implementation
// ============================================================================

extern "C" {

EwpPropagationRef ewp_propagation_create(void) {
    auto* w = new PropagationWrapper();
    w->presenter.set_on_change([w](const ewpresenter::PropagationPresenter::Output& o) {
        if (w->cb) w->cb(to_c(o), w->ctx);
    });
    return w;
}
void ewp_propagation_destroy(EwpPropagationRef ref) { delete cast<PropagationWrapper>(ref); }

void ewp_propagation_set_distance(EwpPropagationRef ref, double km)  { cast<PropagationWrapper>(ref)->presenter.set_distance(km); }
void ewp_propagation_set_frequency(EwpPropagationRef ref, double mhz){ cast<PropagationWrapper>(ref)->presenter.set_frequency(mhz); }
void ewp_propagation_set_tx_height(EwpPropagationRef ref, double m)  { cast<PropagationWrapper>(ref)->presenter.set_tx_height(m); }
void ewp_propagation_set_rx_height(EwpPropagationRef ref, double m)  { cast<PropagationWrapper>(ref)->presenter.set_rx_height(m); }

void ewp_propagation_set_callback(EwpPropagationRef ref, EwpPropagationCallback cb, void* ctx) {
    auto* w = cast<PropagationWrapper>(ref);
    w->cb = cb; w->ctx = ctx;
}

double               ewp_propagation_distance(EwpPropagationRef ref)  { return cast<PropagationWrapper>(ref)->presenter.distance_km(); }
double               ewp_propagation_frequency(EwpPropagationRef ref) { return cast<PropagationWrapper>(ref)->presenter.frequency_mhz(); }
double               ewp_propagation_tx_height(EwpPropagationRef ref) { return cast<PropagationWrapper>(ref)->presenter.tx_height_m(); }
double               ewp_propagation_rx_height(EwpPropagationRef ref) { return cast<PropagationWrapper>(ref)->presenter.rx_height_m(); }
EwpPropagationOutput ewp_propagation_output(EwpPropagationRef ref)    { return to_c(cast<PropagationWrapper>(ref)->presenter.output()); }

// ============================================================================
// Link implementation
// ============================================================================

EwpLinkRef ewp_link_create(void) {
    auto* w = new LinkWrapper();
    w->presenter.set_on_change([w](const ewpresenter::LinkPresenter::Output& o) {
        if (w->cb) w->cb(to_c(o), w->ctx);
    });
    return w;
}
void ewp_link_destroy(EwpLinkRef ref) { delete cast<LinkWrapper>(ref); }

void ewp_link_set_tx_power(EwpLinkRef ref, double dbm)      { cast<LinkWrapper>(ref)->presenter.set_tx_power(dbm); }
void ewp_link_set_tx_gain(EwpLinkRef ref, double db)         { cast<LinkWrapper>(ref)->presenter.set_tx_gain(db); }
void ewp_link_set_rx_gain(EwpLinkRef ref, double db)         { cast<LinkWrapper>(ref)->presenter.set_rx_gain(db); }
void ewp_link_set_distance(EwpLinkRef ref, double km)        { cast<LinkWrapper>(ref)->presenter.set_distance(km); }
void ewp_link_set_tx_height(EwpLinkRef ref, double m)        { cast<LinkWrapper>(ref)->presenter.set_tx_height(m); }
void ewp_link_set_rx_height(EwpLinkRef ref, double m)        { cast<LinkWrapper>(ref)->presenter.set_rx_height(m); }
void ewp_link_set_frequency(EwpLinkRef ref, double mhz)      { cast<LinkWrapper>(ref)->presenter.set_frequency(mhz); }
void ewp_link_set_rx_sensitivity(EwpLinkRef ref, double dbm) { cast<LinkWrapper>(ref)->presenter.set_rx_sensitivity(dbm); }

void ewp_link_set_callback(EwpLinkRef ref, EwpLinkCallback cb, void* ctx) {
    auto* w = cast<LinkWrapper>(ref); w->cb = cb; w->ctx = ctx;
}

double        ewp_link_tx_power(EwpLinkRef ref)       { return cast<LinkWrapper>(ref)->presenter.tx_power_dbm(); }
double        ewp_link_tx_gain(EwpLinkRef ref)         { return cast<LinkWrapper>(ref)->presenter.tx_gain_db(); }
double        ewp_link_rx_gain(EwpLinkRef ref)         { return cast<LinkWrapper>(ref)->presenter.rx_gain_db(); }
double        ewp_link_distance(EwpLinkRef ref)        { return cast<LinkWrapper>(ref)->presenter.distance_km(); }
double        ewp_link_tx_height(EwpLinkRef ref)       { return cast<LinkWrapper>(ref)->presenter.tx_height_m(); }
double        ewp_link_rx_height(EwpLinkRef ref)       { return cast<LinkWrapper>(ref)->presenter.rx_height_m(); }
double        ewp_link_frequency(EwpLinkRef ref)       { return cast<LinkWrapper>(ref)->presenter.frequency_mhz(); }
double        ewp_link_rx_sensitivity(EwpLinkRef ref)  { return cast<LinkWrapper>(ref)->presenter.rx_sensitivity_dbm(); }
EwpLinkOutput ewp_link_output(EwpLinkRef ref)          { return to_c(cast<LinkWrapper>(ref)->presenter.output()); }

// ============================================================================
// Receiver implementation
// ============================================================================

EwpReceiverRef ewp_receiver_create(void) {
    auto* w = new ReceiverWrapper();
    w->presenter.set_on_change([w](const ewpresenter::ReceiverPresenter::Output& o) {
        if (w->cb) w->cb(to_c(o), w->ctx);
    });
    return w;
}
void ewp_receiver_destroy(EwpReceiverRef ref) { delete cast<ReceiverWrapper>(ref); }

void ewp_receiver_set_bandwidth(EwpReceiverRef ref, double mhz)      { cast<ReceiverWrapper>(ref)->presenter.set_bandwidth(mhz); }
void ewp_receiver_set_noise_figure(EwpReceiverRef ref, double db)    { cast<ReceiverWrapper>(ref)->presenter.set_noise_figure(db); }
void ewp_receiver_set_required_snr(EwpReceiverRef ref, double db)    { cast<ReceiverWrapper>(ref)->presenter.set_required_snr(db); }
void ewp_receiver_set_second_order_ip(EwpReceiverRef ref, double dbm){ cast<ReceiverWrapper>(ref)->presenter.set_second_order_ip(dbm); }
void ewp_receiver_set_third_order_ip(EwpReceiverRef ref, double dbm) { cast<ReceiverWrapper>(ref)->presenter.set_third_order_ip(dbm); }
void ewp_receiver_set_adc_bits(EwpReceiverRef ref, int bits)         { cast<ReceiverWrapper>(ref)->presenter.set_adc_bits(bits); }

void ewp_receiver_set_stages(EwpReceiverRef ref, const EwpStageInput* stages, int count) {
    std::vector<ewpresenter::ReceiverPresenter::StageInput> v;
    v.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i)
        v.push_back({stages[i].noise_figure_db, stages[i].gain_db});
    cast<ReceiverWrapper>(ref)->presenter.set_stages(std::move(v));
}

void ewp_receiver_set_callback(EwpReceiverRef ref, EwpReceiverCallback cb, void* ctx) {
    auto* w = cast<ReceiverWrapper>(ref); w->cb = cb; w->ctx = ctx;
}

double            ewp_receiver_bandwidth(EwpReceiverRef ref)       { return cast<ReceiverWrapper>(ref)->presenter.bandwidth_mhz(); }
double            ewp_receiver_noise_figure(EwpReceiverRef ref)    { return cast<ReceiverWrapper>(ref)->presenter.noise_figure_db(); }
double            ewp_receiver_required_snr(EwpReceiverRef ref)    { return cast<ReceiverWrapper>(ref)->presenter.required_snr_db(); }
double            ewp_receiver_second_order_ip(EwpReceiverRef ref) { return cast<ReceiverWrapper>(ref)->presenter.second_order_ip_dbm(); }
double            ewp_receiver_third_order_ip(EwpReceiverRef ref)  { return cast<ReceiverWrapper>(ref)->presenter.third_order_ip_dbm(); }
int               ewp_receiver_adc_bits(EwpReceiverRef ref)        { return cast<ReceiverWrapper>(ref)->presenter.adc_bits(); }

int ewp_receiver_stage_count(EwpReceiverRef ref) {
    return static_cast<int>(cast<ReceiverWrapper>(ref)->presenter.stages().size());
}
EwpStageInput ewp_receiver_stage(EwpReceiverRef ref, int index) {
    const auto& s = cast<ReceiverWrapper>(ref)->presenter.stages()[static_cast<std::size_t>(index)];
    return {s.noise_figure_db, s.gain_db};
}
EwpReceiverOutput ewp_receiver_output(EwpReceiverRef ref) { return to_c(cast<ReceiverWrapper>(ref)->presenter.output()); }

// ============================================================================
// Jamming implementation
// ============================================================================

EwpJammingRef ewp_jamming_create(void) {
    auto* w = new JammingWrapper();
    w->presenter.set_on_change([w](const ewpresenter::JammingPresenter::Output& o) {
        if (w->cb) w->cb(to_c(o), w->ctx);
    });
    return w;
}
void ewp_jamming_destroy(EwpJammingRef ref) { delete cast<JammingWrapper>(ref); }

void ewp_jamming_set_signal_erp(EwpJammingRef ref, double dbm)    { cast<JammingWrapper>(ref)->presenter.set_signal_erp(dbm); }
void ewp_jamming_set_jammer_erp(EwpJammingRef ref, double dbm)    { cast<JammingWrapper>(ref)->presenter.set_jammer_erp(dbm); }
void ewp_jamming_set_signal_dist(EwpJammingRef ref, double km)    { cast<JammingWrapper>(ref)->presenter.set_signal_to_rx_dist(km); }
void ewp_jamming_set_jammer_dist(EwpJammingRef ref, double km)    { cast<JammingWrapper>(ref)->presenter.set_jammer_to_rx_dist(km); }
void ewp_jamming_set_signal_height(EwpJammingRef ref, double m)   { cast<JammingWrapper>(ref)->presenter.set_signal_tx_height(m); }
void ewp_jamming_set_jammer_height(EwpJammingRef ref, double m)   { cast<JammingWrapper>(ref)->presenter.set_jammer_height(m); }
void ewp_jamming_set_rx_height(EwpJammingRef ref, double m)       { cast<JammingWrapper>(ref)->presenter.set_rx_height(m); }
void ewp_jamming_set_frequency(EwpJammingRef ref, double mhz)     { cast<JammingWrapper>(ref)->presenter.set_frequency(mhz); }
void ewp_jamming_set_rx_gain_signal(EwpJammingRef ref, double db) { cast<JammingWrapper>(ref)->presenter.set_rx_gain_signal(db); }
void ewp_jamming_set_rx_gain_jammer(EwpJammingRef ref, double db) { cast<JammingWrapper>(ref)->presenter.set_rx_gain_jammer(db); }
void ewp_jamming_set_signal_bandwidth(EwpJammingRef ref, double mhz){ cast<JammingWrapper>(ref)->presenter.set_signal_bandwidth(mhz); }
void ewp_jamming_set_hop_range(EwpJammingRef ref, double mhz)     { cast<JammingWrapper>(ref)->presenter.set_hop_range(mhz); }

void ewp_jamming_set_callback(EwpJammingRef ref, EwpJammingCallback cb, void* ctx) {
    auto* w = cast<JammingWrapper>(ref); w->cb = cb; w->ctx = ctx;
}

double           ewp_jamming_signal_erp(EwpJammingRef ref)       { return cast<JammingWrapper>(ref)->presenter.signal_erp_dbm(); }
double           ewp_jamming_jammer_erp(EwpJammingRef ref)       { return cast<JammingWrapper>(ref)->presenter.jammer_erp_dbm(); }
double           ewp_jamming_signal_dist(EwpJammingRef ref)      { return cast<JammingWrapper>(ref)->presenter.signal_to_rx_dist_km(); }
double           ewp_jamming_jammer_dist(EwpJammingRef ref)      { return cast<JammingWrapper>(ref)->presenter.jammer_to_rx_dist_km(); }
double           ewp_jamming_signal_height(EwpJammingRef ref)    { return cast<JammingWrapper>(ref)->presenter.signal_tx_height_m(); }
double           ewp_jamming_jammer_height(EwpJammingRef ref)    { return cast<JammingWrapper>(ref)->presenter.jammer_height_m(); }
double           ewp_jamming_rx_height(EwpJammingRef ref)        { return cast<JammingWrapper>(ref)->presenter.rx_height_m(); }
double           ewp_jamming_frequency(EwpJammingRef ref)        { return cast<JammingWrapper>(ref)->presenter.frequency_mhz(); }
double           ewp_jamming_signal_bandwidth(EwpJammingRef ref) { return cast<JammingWrapper>(ref)->presenter.signal_bandwidth_mhz(); }
double           ewp_jamming_hop_range(EwpJammingRef ref)        { return cast<JammingWrapper>(ref)->presenter.hop_range_mhz(); }
EwpJammingOutput ewp_jamming_output(EwpJammingRef ref)           { return to_c(cast<JammingWrapper>(ref)->presenter.output()); }

// ============================================================================
// Location implementation
// ============================================================================

EwpLocationRef ewp_location_create(void) {
    auto* w = new LocationWrapper();
    w->presenter.set_on_change([w](const ewpresenter::LocationPresenter::Output& o) {
        if (w->cb) w->cb(to_c(o), w->ctx);
    });
    return w;
}
void ewp_location_destroy(EwpLocationRef ref) { delete cast<LocationWrapper>(ref); }

void ewp_location_set_rms_bearing_error(EwpLocationRef ref, double deg){ cast<LocationWrapper>(ref)->presenter.set_rms_bearing_error(deg); }
void ewp_location_set_aoa_range(EwpLocationRef ref, double km)         { cast<LocationWrapper>(ref)->presenter.set_aoa_range(km); }
void ewp_location_set_semi_major(EwpLocationRef ref, double km)        { cast<LocationWrapper>(ref)->presenter.set_semi_major(km); }
void ewp_location_set_semi_minor(EwpLocationRef ref, double km)        { cast<LocationWrapper>(ref)->presenter.set_semi_minor(km); }

void ewp_location_set_callback(EwpLocationRef ref, EwpLocationCallback cb, void* ctx) {
    auto* w = cast<LocationWrapper>(ref); w->cb = cb; w->ctx = ctx;
}

double            ewp_location_rms_bearing_error(EwpLocationRef ref) { return cast<LocationWrapper>(ref)->presenter.rms_bearing_error_deg(); }
double            ewp_location_aoa_range(EwpLocationRef ref)         { return cast<LocationWrapper>(ref)->presenter.aoa_range_km(); }
double            ewp_location_semi_major(EwpLocationRef ref)        { return cast<LocationWrapper>(ref)->presenter.semi_major_km(); }
double            ewp_location_semi_minor(EwpLocationRef ref)        { return cast<LocationWrapper>(ref)->presenter.semi_minor_km(); }
EwpLocationOutput ewp_location_output(EwpLocationRef ref)            { return to_c(cast<LocationWrapper>(ref)->presenter.output()); }

// ============================================================================
// Radar implementation
// ============================================================================

EwpRadarRef ewp_radar_create(void) {
    auto* w = new RadarWrapper();
    w->presenter.set_on_change([w](const ewpresenter::RadarPresenter::Output& o) {
        if (w->cb) w->cb(to_c(o), w->ctx);
    });
    return w;
}
void ewp_radar_destroy(EwpRadarRef ref) { delete cast<RadarWrapper>(ref); }

void ewp_radar_set_tx_power(EwpRadarRef ref, double dbm)     { cast<RadarWrapper>(ref)->presenter.set_tx_power(dbm); }
void ewp_radar_set_antenna_gain(EwpRadarRef ref, double dbi) { cast<RadarWrapper>(ref)->presenter.set_antenna_gain(dbi); }
void ewp_radar_set_target_rcs(EwpRadarRef ref, double dbsm)  { cast<RadarWrapper>(ref)->presenter.set_target_rcs(dbsm); }
void ewp_radar_set_frequency(EwpRadarRef ref, double mhz)    { cast<RadarWrapper>(ref)->presenter.set_frequency(mhz); }
void ewp_radar_set_system_losses(EwpRadarRef ref, double db)  { cast<RadarWrapper>(ref)->presenter.set_system_losses(db); }
void ewp_radar_set_noise_figure(EwpRadarRef ref, double db)  { cast<RadarWrapper>(ref)->presenter.set_noise_figure(db); }
void ewp_radar_set_bandwidth(EwpRadarRef ref, double mhz)    { cast<RadarWrapper>(ref)->presenter.set_bandwidth(mhz); }
void ewp_radar_set_required_snr(EwpRadarRef ref, double db)  { cast<RadarWrapper>(ref)->presenter.set_required_snr(db); }
void ewp_radar_set_time_bandwidth(EwpRadarRef ref, double tb){ cast<RadarWrapper>(ref)->presenter.set_time_bandwidth_product(tb); }
void ewp_radar_set_num_pulses(EwpRadarRef ref, int n)        { cast<RadarWrapper>(ref)->presenter.set_num_pulses(n); }

void ewp_radar_set_callback(EwpRadarRef ref, EwpRadarCallback cb, void* ctx) {
    auto* w = cast<RadarWrapper>(ref); w->cb = cb; w->ctx = ctx;
}

double         ewp_radar_tx_power(EwpRadarRef ref)      { return cast<RadarWrapper>(ref)->presenter.tx_power_dbm(); }
double         ewp_radar_antenna_gain(EwpRadarRef ref)  { return cast<RadarWrapper>(ref)->presenter.antenna_gain_dbi(); }
double         ewp_radar_target_rcs(EwpRadarRef ref)    { return cast<RadarWrapper>(ref)->presenter.target_rcs_dbsm(); }
double         ewp_radar_frequency(EwpRadarRef ref)     { return cast<RadarWrapper>(ref)->presenter.frequency_mhz(); }
double         ewp_radar_system_losses(EwpRadarRef ref) { return cast<RadarWrapper>(ref)->presenter.system_losses_db(); }
double         ewp_radar_noise_figure(EwpRadarRef ref)  { return cast<RadarWrapper>(ref)->presenter.noise_figure_db(); }
double         ewp_radar_bandwidth(EwpRadarRef ref)     { return cast<RadarWrapper>(ref)->presenter.bandwidth_mhz(); }
double         ewp_radar_required_snr(EwpRadarRef ref)  { return cast<RadarWrapper>(ref)->presenter.required_snr_db(); }
double         ewp_radar_time_bandwidth(EwpRadarRef ref){ return cast<RadarWrapper>(ref)->presenter.time_bandwidth_product(); }
int            ewp_radar_num_pulses(EwpRadarRef ref)    { return cast<RadarWrapper>(ref)->presenter.num_pulses(); }
EwpRadarOutput ewp_radar_output(EwpRadarRef ref)        { return to_c(cast<RadarWrapper>(ref)->presenter.output()); }

} // extern "C"
