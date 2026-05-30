#include "jamming_panel.h"
#include <cmath>

namespace ewcalc {

using namespace ui;

JammingPanel::JammingPanel() { // NOLINT(readability-function-cognitive-complexity)
    auto [scroll, stack] = make_panel_root();
    root_ = scroll;

    stack.Children().Append(make_section_header(L"Signal"));
    auto tb_serp  = make_number_input(L"Signal ERP (dBm)",     presenter_.signal_erp_dbm());
    auto tb_sdist = make_number_input(L"Signal-Rx dist (km)",  presenter_.signal_to_rx_dist_km());
    auto tb_sht   = make_number_input(L"Signal Tx height (m)", presenter_.signal_tx_height_m());
    stack.Children().Append(tb_serp); stack.Children().Append(tb_sdist); stack.Children().Append(tb_sht);

    stack.Children().Append(make_section_header(L"Jammer"));
    auto tb_jerp  = make_number_input(L"Jammer ERP (dBm)",    presenter_.jammer_erp_dbm());
    auto tb_jdist = make_number_input(L"Jammer-Rx dist (km)", presenter_.jammer_to_rx_dist_km());
    auto tb_jht   = make_number_input(L"Jammer height (m)",   presenter_.jammer_height_m());
    stack.Children().Append(tb_jerp); stack.Children().Append(tb_jdist); stack.Children().Append(tb_jht);

    stack.Children().Append(make_section_header(L"Geometry & Frequency"));
    auto tb_rxht = make_number_input(L"Rx height (m)",   presenter_.rx_height_m());
    auto tb_freq = make_number_input(L"Frequency (MHz)", presenter_.frequency_mhz());
    stack.Children().Append(tb_rxht); stack.Children().Append(tb_freq);

    stack.Children().Append(make_section_header(L"Partial-Band"));
    auto tb_sbw = make_number_input(L"Signal BW (MHz)", presenter_.signal_bandwidth_mhz());
    auto tb_hop = make_number_input(L"Hop range (MHz)", presenter_.hop_range_mhz());
    stack.Children().Append(tb_sbw); stack.Children().Append(tb_hop);

    stack.Children().Append(make_section_header(L"Results"));
    auto [rjs, ojs] = make_result_row(L"J/S ratio:",    presenter_.output().js_ratio_str);
    auto [rsr, osr] = make_result_row(L"Signal at Rx:", presenter_.output().signal_at_rx_str);
    auto [rjr, ojr] = make_result_row(L"Jammer at Rx:", presenter_.output().jammer_at_rx_str);
    auto [rob, oob] = make_result_row(L"Optimum BW:",   presenter_.output().optimum_bw_str);
    auto [rdc, odc] = make_result_row(L"Duty cycle:",   presenter_.output().duty_cycle_str);
    out_js_=ojs; out_signal_rx_=osr; out_jammer_rx_=ojr; out_opt_bw_=oob; out_duty_cycle_=odc;
    for (auto& r : {rjs, rsr, rjr, rob, rdc}) stack.Children().Append(r);

    tb_serp.TextChanged( [this](auto&&,auto&&){ double v=parse_textbox(tb_serp_);  if(!std::isnan(v)){presenter_.set_signal_erp(v);          update_outputs();} });
    tb_sdist.TextChanged([this](auto&&,auto&&){ double v=parse_textbox(tb_sdist_); if(!std::isnan(v)){presenter_.set_signal_to_rx_dist(v);   update_outputs();} });
    tb_sht.TextChanged(  [this](auto&&,auto&&){ double v=parse_textbox(tb_sht_);   if(!std::isnan(v)){presenter_.set_signal_tx_height(v);    update_outputs();} });
    tb_jerp.TextChanged( [this](auto&&,auto&&){ double v=parse_textbox(tb_jerp_);  if(!std::isnan(v)){presenter_.set_jammer_erp(v);          update_outputs();} });
    tb_jdist.TextChanged([this](auto&&,auto&&){ double v=parse_textbox(tb_jdist_); if(!std::isnan(v)){presenter_.set_jammer_to_rx_dist(v);   update_outputs();} });
    tb_jht.TextChanged(  [this](auto&&,auto&&){ double v=parse_textbox(tb_jht_);   if(!std::isnan(v)){presenter_.set_jammer_height(v);       update_outputs();} });
    tb_rxht.TextChanged( [this](auto&&,auto&&){ double v=parse_textbox(tb_rxht_);  if(!std::isnan(v)){presenter_.set_rx_height(v);           update_outputs();} });
    tb_freq.TextChanged( [this](auto&&,auto&&){ double v=parse_textbox(tb_freq_);  if(!std::isnan(v)){presenter_.set_frequency(v);           update_outputs();} });
    tb_sbw.TextChanged(  [this](auto&&,auto&&){ double v=parse_textbox(tb_sbw_);   if(!std::isnan(v)){presenter_.set_signal_bandwidth(v);    update_outputs();} });
    tb_hop.TextChanged(  [this](auto&&,auto&&){ double v=parse_textbox(tb_hop_);   if(!std::isnan(v)){presenter_.set_hop_range(v);           update_outputs();} });

    tb_serp_=tb_serp; tb_sdist_=tb_sdist; tb_sht_=tb_sht;
    tb_jerp_=tb_jerp; tb_jdist_=tb_jdist; tb_jht_=tb_jht;
    tb_rxht_=tb_rxht; tb_freq_=tb_freq; tb_sbw_=tb_sbw; tb_hop_=tb_hop;
}

winrt::Microsoft::UI::Xaml::UIElement JammingPanel::root() const noexcept { return root_; }

void JammingPanel::update_outputs() {
    const auto& o = presenter_.output();
    if (!o.valid) return;
    out_js_.Text(winrt::to_hstring(o.js_ratio_str));
    out_signal_rx_.Text(winrt::to_hstring(o.signal_at_rx_str));
    out_jammer_rx_.Text(winrt::to_hstring(o.jammer_at_rx_str));
    out_opt_bw_.Text(winrt::to_hstring(o.optimum_bw_str));
    out_duty_cycle_.Text(winrt::to_hstring(o.duty_cycle_str));
}

} // namespace ewcalc
