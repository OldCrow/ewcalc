#include "link_panel.h"
#include <cmath>

namespace ewcalc {

using namespace ui;

LinkPanel::LinkPanel() {
    auto [scroll, stack] = make_panel_root();
    root_ = scroll;

    stack.Children().Append(make_section_header(L"Transmitter"));
    auto tb_pwr  = make_number_input(L"Tx power (dBm)",       presenter_.tx_power_dbm());
    auto tb_gtx  = make_number_input(L"Tx antenna gain (dB)",  presenter_.tx_gain_db());
    stack.Children().Append(tb_pwr); stack.Children().Append(tb_gtx);

    stack.Children().Append(make_section_header(L"Geometry"));
    auto tb_dist = make_number_input(L"Distance (km)",         presenter_.distance_km());
    auto tb_ht   = make_number_input(L"Tx height (m)",          presenter_.tx_height_m());
    auto tb_hr   = make_number_input(L"Rx height (m)",          presenter_.rx_height_m());
    auto tb_freq = make_number_input(L"Frequency (MHz)",        presenter_.frequency_mhz());
    stack.Children().Append(tb_dist); stack.Children().Append(tb_ht);
    stack.Children().Append(tb_hr);   stack.Children().Append(tb_freq);

    stack.Children().Append(make_section_header(L"Receiver"));
    auto tb_grx  = make_number_input(L"Rx antenna gain (dB)",   presenter_.rx_gain_db());
    auto tb_sens = make_number_input(L"Rx sensitivity (dBm)",    presenter_.rx_sensitivity_dbm());
    stack.Children().Append(tb_grx); stack.Children().Append(tb_sens);

    stack.Children().Append(make_section_header(L"Results"));
    auto [rp,  op]  = make_result_row(L"Received power:",   presenter_.output().received_power_str);
    auto [rl,  ol]  = make_result_row(L"Link margin:",      presenter_.output().link_margin_str);
    auto [rpl, opl] = make_result_row(L"Path loss:",        presenter_.output().path_loss_str);
    auto [rfz, ofz] = make_result_row(L"Fresnel zone:",     presenter_.output().fresnel_zone_str);
    auto [rrg, org] = make_result_row(L"Regime:",           presenter_.output().regime_str);
    auto [rer, oer] = make_result_row(L"Effective range:",  presenter_.output().effective_range_str);
    auto [rrr, orr] = make_result_row(L"Range regime:",     presenter_.output().range_regime_str);
    out_received_power_ = op; out_link_margin_ = ol;
    out_path_loss_ = opl;     out_fresnel_     = ofz;
    out_regime_ = org;        out_eff_range_   = oer; out_range_regime_ = orr;
    for (auto& r : {rp, rl, rpl, rfz, rrg, rer, rrr}) stack.Children().Append(r);

    tb_pwr.TextChanged( [this](auto&&,auto&&){ double v=parse_textbox(tb_pwr_);  if(!std::isnan(v)){presenter_.set_tx_power(v);       update_outputs();} });
    tb_gtx.TextChanged( [this](auto&&,auto&&){ double v=parse_textbox(tb_gtx_);  if(!std::isnan(v)){presenter_.set_tx_gain(v);        update_outputs();} });
    tb_grx.TextChanged( [this](auto&&,auto&&){ double v=parse_textbox(tb_grx_);  if(!std::isnan(v)){presenter_.set_rx_gain(v);        update_outputs();} });
    tb_dist.TextChanged([this](auto&&,auto&&){ double v=parse_textbox(tb_dist_); if(!std::isnan(v)){presenter_.set_distance(v);       update_outputs();} });
    tb_ht.TextChanged(  [this](auto&&,auto&&){ double v=parse_textbox(tb_ht_);   if(!std::isnan(v)){presenter_.set_tx_height(v);      update_outputs();} });
    tb_hr.TextChanged(  [this](auto&&,auto&&){ double v=parse_textbox(tb_hr_);   if(!std::isnan(v)){presenter_.set_rx_height(v);      update_outputs();} });
    tb_freq.TextChanged([this](auto&&,auto&&){ double v=parse_textbox(tb_freq_); if(!std::isnan(v)){presenter_.set_frequency(v);      update_outputs();} });
    tb_sens.TextChanged([this](auto&&,auto&&){ double v=parse_textbox(tb_sens_); if(!std::isnan(v)){presenter_.set_rx_sensitivity(v); update_outputs();} });

    tb_pwr_=tb_pwr; tb_gtx_=tb_gtx; tb_grx_=tb_grx;
    tb_dist_=tb_dist; tb_ht_=tb_ht; tb_hr_=tb_hr; tb_freq_=tb_freq; tb_sens_=tb_sens;
}

winrt::Microsoft::UI::Xaml::UIElement LinkPanel::root() const noexcept { return root_; }

void LinkPanel::update_outputs() {
    const auto& o = presenter_.output();
    if (!o.valid) return;
    out_received_power_.Text(winrt::to_hstring(o.received_power_str));
    out_link_margin_.Text(winrt::to_hstring(o.link_margin_str));
    out_path_loss_.Text(winrt::to_hstring(o.path_loss_str));
    out_fresnel_.Text(winrt::to_hstring(o.fresnel_zone_str));
    out_regime_.Text(winrt::to_hstring(o.regime_str));
    out_eff_range_.Text(winrt::to_hstring(o.effective_range_str));
    out_range_regime_.Text(winrt::to_hstring(o.range_regime_str));
}

} // namespace ewcalc
