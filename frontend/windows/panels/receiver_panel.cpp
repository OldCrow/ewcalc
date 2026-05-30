#include "receiver_panel.h"
#include <cmath>

namespace ewcalc {

using namespace ui;

ReceiverPanel::ReceiverPanel() {
    auto [scroll, stack] = make_panel_root();
    root_ = scroll;

    stack.Children().Append(make_section_header(L"Sensitivity"));
    auto tb_bw  = make_number_input(L"Bandwidth (MHz)",  presenter_.bandwidth_mhz());
    auto tb_nf  = make_number_input(L"Noise figure (dB)", presenter_.noise_figure_db());
    auto tb_snr = make_number_input(L"Required SNR (dB)", presenter_.required_snr_db());
    stack.Children().Append(tb_bw); stack.Children().Append(tb_nf); stack.Children().Append(tb_snr);

    stack.Children().Append(make_section_header(L"Dynamic Range"));
    auto tb_ip2  = make_number_input(L"IP2 (dBm)", presenter_.second_order_ip_dbm());
    auto tb_ip3  = make_number_input(L"IP3 (dBm)", presenter_.third_order_ip_dbm());
    auto tb_bits = make_number_input(L"ADC bits",  static_cast<double>(presenter_.adc_bits()));
    stack.Children().Append(tb_ip2); stack.Children().Append(tb_ip3); stack.Children().Append(tb_bits);

    stack.Children().Append(make_section_header(L"Results"));
    auto [rs, os] = make_result_row(L"Sensitivity:", presenter_.output().sensitivity_str);
    auto [rn, on] = make_result_row(L"Cascaded NF:", presenter_.output().cascaded_nf_str);
    auto [r2, o2] = make_result_row(L"SFDR2:",       presenter_.output().sfdr2_str);
    auto [r3, o3] = make_result_row(L"SFDR3:",       presenter_.output().sfdr3_str);
    auto [rd, od] = make_result_row(L"Digital DR:",  presenter_.output().digital_dr_str);
    out_sensitivity_=os; out_cascaded_nf_=on; out_sfdr2_=o2; out_sfdr3_=o3; out_digital_dr_=od;
    for (auto& r : {rs, rn, r2, r3, rd}) stack.Children().Append(r);

    tb_bw.TextChanged(  [this](auto&&,auto&&){ double v=parse_textbox(tb_bw_);   if(!std::isnan(v)){presenter_.set_bandwidth(v);        update_outputs();} });
    tb_nf.TextChanged(  [this](auto&&,auto&&){ double v=parse_textbox(tb_nf_);   if(!std::isnan(v)){presenter_.set_noise_figure(v);     update_outputs();} });
    tb_snr.TextChanged( [this](auto&&,auto&&){ double v=parse_textbox(tb_snr_);  if(!std::isnan(v)){presenter_.set_required_snr(v);     update_outputs();} });
    tb_ip2.TextChanged( [this](auto&&,auto&&){ double v=parse_textbox(tb_ip2_);  if(!std::isnan(v)){presenter_.set_second_order_ip(v);  update_outputs();} });
    tb_ip3.TextChanged( [this](auto&&,auto&&){ double v=parse_textbox(tb_ip3_);  if(!std::isnan(v)){presenter_.set_third_order_ip(v);   update_outputs();} });
    tb_bits.TextChanged([this](auto&&,auto&&){ double v=parse_textbox(tb_bits_); if(!std::isnan(v)){presenter_.set_adc_bits(static_cast<int>(v)); update_outputs();} });

    tb_bw_=tb_bw; tb_nf_=tb_nf; tb_snr_=tb_snr;
    tb_ip2_=tb_ip2; tb_ip3_=tb_ip3; tb_bits_=tb_bits;
}

winrt::Microsoft::UI::Xaml::UIElement ReceiverPanel::root() const noexcept { return root_; }

void ReceiverPanel::update_outputs() {
    const auto& o = presenter_.output();
    if (!o.valid) return;
    out_sensitivity_.Text(winrt::to_hstring(o.sensitivity_str));
    out_cascaded_nf_.Text(winrt::to_hstring(o.cascaded_nf_str));
    out_sfdr2_.Text(winrt::to_hstring(o.sfdr2_str));
    out_sfdr3_.Text(winrt::to_hstring(o.sfdr3_str));
    out_digital_dr_.Text(winrt::to_hstring(o.digital_dr_str));
}

} // namespace ewcalc
