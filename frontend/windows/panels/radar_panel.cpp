#include "radar_panel.h"
#include <cmath>

namespace ewcalc {

using namespace ui;

RadarPanel::RadarPanel() {
    auto [scroll, stack] = make_panel_root();
    root_ = scroll;

    stack.Children().Append(make_section_header(L"Transmitter / Antenna"));
    auto tb_pwr  = make_number_input(L"Tx power (dBm)",     presenter_.tx_power_dbm());
    auto tb_gain = make_number_input(L"Antenna gain (dBi)", presenter_.antenna_gain_dbi());
    auto tb_freq = make_number_input(L"Frequency (MHz)",    presenter_.frequency_mhz());
    stack.Children().Append(tb_pwr); stack.Children().Append(tb_gain); stack.Children().Append(tb_freq);

    stack.Children().Append(make_section_header(L"Target & Losses"));
    auto tb_rcs  = make_number_input(L"Target RCS (dBsm)", presenter_.target_rcs_dbsm());
    auto tb_loss = make_number_input(L"System losses (dB)", presenter_.system_losses_db());
    stack.Children().Append(tb_rcs); stack.Children().Append(tb_loss);

    stack.Children().Append(make_section_header(L"Receiver"));
    auto tb_nf  = make_number_input(L"Noise figure (dB)", presenter_.noise_figure_db());
    auto tb_bw  = make_number_input(L"Bandwidth (MHz)",   presenter_.bandwidth_mhz());
    auto tb_snr = make_number_input(L"Required SNR (dB)", presenter_.required_snr_db());
    stack.Children().Append(tb_nf); stack.Children().Append(tb_bw); stack.Children().Append(tb_snr);

    stack.Children().Append(make_section_header(L"Signal Processing"));
    auto tb_tb = make_number_input(L"Time-BW product", presenter_.time_bandwidth_product());
    auto tb_np = make_number_input(L"Coherent pulses", static_cast<double>(presenter_.num_pulses()));
    stack.Children().Append(tb_tb); stack.Children().Append(tb_np);

    stack.Children().Append(make_section_header(L"Results"));
    auto [rr,  or_] = make_result_row(L"Max range:",              presenter_.output().max_range_str);
    auto [rtw, otw] = make_result_row(L"Two-way loss:",           presenter_.output().two_way_loss_str);
    auto [rpc, opc] = make_result_row(L"Pulse compression gain:", presenter_.output().pulse_compression_gain_str);
    auto [rci, oci] = make_result_row(L"Coherent int. gain:",     presenter_.output().coherent_integration_gain_str);
    out_range_=or_; out_two_way_loss_=otw; out_pc_gain_=opc; out_coh_gain_=oci;
    for (auto& r : {rr, rtw, rpc, rci}) stack.Children().Append(r);

    tb_pwr.TextChanged( [this](auto&&,auto&&){ double v=parse_textbox(tb_pwr_);  if(!std::isnan(v)){presenter_.set_tx_power(v);              update_outputs();} });
    tb_gain.TextChanged([this](auto&&,auto&&){ double v=parse_textbox(tb_gain_); if(!std::isnan(v)){presenter_.set_antenna_gain(v);           update_outputs();} });
    tb_freq.TextChanged([this](auto&&,auto&&){ double v=parse_textbox(tb_freq_); if(!std::isnan(v)){presenter_.set_frequency(v);              update_outputs();} });
    tb_rcs.TextChanged( [this](auto&&,auto&&){ double v=parse_textbox(tb_rcs_);  if(!std::isnan(v)){presenter_.set_target_rcs(v);             update_outputs();} });
    tb_loss.TextChanged([this](auto&&,auto&&){ double v=parse_textbox(tb_loss_); if(!std::isnan(v)){presenter_.set_system_losses(v);          update_outputs();} });
    tb_nf.TextChanged(  [this](auto&&,auto&&){ double v=parse_textbox(tb_nf_);   if(!std::isnan(v)){presenter_.set_noise_figure(v);           update_outputs();} });
    tb_bw.TextChanged(  [this](auto&&,auto&&){ double v=parse_textbox(tb_bw_);   if(!std::isnan(v)){presenter_.set_bandwidth(v);              update_outputs();} });
    tb_snr.TextChanged( [this](auto&&,auto&&){ double v=parse_textbox(tb_snr_);  if(!std::isnan(v)){presenter_.set_required_snr(v);           update_outputs();} });
    tb_tb.TextChanged(  [this](auto&&,auto&&){ double v=parse_textbox(tb_tb_);   if(!std::isnan(v)){presenter_.set_time_bandwidth_product(v); update_outputs();} });
    tb_np.TextChanged(  [this](auto&&,auto&&){ double v=parse_textbox(tb_np_);   if(!std::isnan(v)){presenter_.set_num_pulses(static_cast<int>(v)); update_outputs();} });

    tb_pwr_=tb_pwr; tb_gain_=tb_gain; tb_freq_=tb_freq;
    tb_rcs_=tb_rcs; tb_loss_=tb_loss; tb_nf_=tb_nf; tb_bw_=tb_bw; tb_snr_=tb_snr;
    tb_tb_=tb_tb; tb_np_=tb_np;
}

winrt::Microsoft::UI::Xaml::UIElement RadarPanel::root() const noexcept { return root_; }

void RadarPanel::update_outputs() {
    const auto& o = presenter_.output();
    if (!o.valid) return;
    out_range_.Text(winrt::to_hstring(o.max_range_str));
    out_two_way_loss_.Text(winrt::to_hstring(o.two_way_loss_str));
    out_pc_gain_.Text(winrt::to_hstring(o.pulse_compression_gain_str));
    out_coh_gain_.Text(winrt::to_hstring(o.coherent_integration_gain_str));
}

} // namespace ewcalc
