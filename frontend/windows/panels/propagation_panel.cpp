#include "propagation_panel.h"
#include <cmath>

namespace ewcalc {

using namespace ui;

PropagationPanel::PropagationPanel() {
    auto [scroll, stack] = make_panel_root();
    root_ = scroll;

    stack.Children().Append(make_section_header(L"Inputs"));
    auto tb_dist = make_number_input(L"Distance (km)",        presenter_.distance_km());
    auto tb_freq = make_number_input(L"Frequency (MHz)",       presenter_.frequency_mhz());
    auto tb_ht   = make_number_input(L"Tx antenna height (m)", presenter_.tx_height_m());
    auto tb_hr   = make_number_input(L"Rx antenna height (m)", presenter_.rx_height_m());
    stack.Children().Append(tb_dist);
    stack.Children().Append(tb_freq);
    stack.Children().Append(tb_ht);
    stack.Children().Append(tb_hr);

    stack.Children().Append(make_section_header(L"Results"));
    auto [row_pl,  out_pl]  = make_result_row(L"Path loss:",         presenter_.output().path_loss_str);
    auto [row_fsp, out_fsp] = make_result_row(L"FSPL:",              presenter_.output().fspl_str);
    auto [row_2r,  out_2r]  = make_result_row(L"2-Ray loss:",        presenter_.output().two_ray_loss_str);
    auto [row_fz,  out_fz]  = make_result_row(L"Fresnel zone dist:", presenter_.output().fresnel_zone_str);
    auto [row_reg, out_reg] = make_result_row(L"Regime:",            presenter_.output().regime_str);
    out_path_loss_ = out_pl; out_fspl_ = out_fsp;
    out_two_ray_ = out_2r; out_fresnel_ = out_fz; out_regime_ = out_reg;
    for (auto& r : {row_pl, row_fsp, row_2r, row_fz, row_reg}) stack.Children().Append(r);

    tb_dist.TextChanged([this](auto&&, auto&&) { double v = parse_textbox(tb_dist_); if (!std::isnan(v)) { presenter_.set_distance(v);   update_outputs(); } });
    tb_freq.TextChanged([this](auto&&, auto&&) { double v = parse_textbox(tb_freq_); if (!std::isnan(v)) { presenter_.set_frequency(v);  update_outputs(); } });
    tb_ht.TextChanged(  [this](auto&&, auto&&) { double v = parse_textbox(tb_ht_);   if (!std::isnan(v)) { presenter_.set_tx_height(v);  update_outputs(); } });
    tb_hr.TextChanged(  [this](auto&&, auto&&) { double v = parse_textbox(tb_hr_);   if (!std::isnan(v)) { presenter_.set_rx_height(v);  update_outputs(); } });

    tb_dist_ = tb_dist; tb_freq_ = tb_freq; tb_ht_ = tb_ht; tb_hr_ = tb_hr;
}

winrt::Microsoft::UI::Xaml::UIElement PropagationPanel::root() const noexcept {
    return root_;
}

void PropagationPanel::update_outputs() {
    const auto& o = presenter_.output();
    if (!o.valid) return;
    out_path_loss_.Text(winrt::to_hstring(o.path_loss_str));
    out_fspl_.Text(winrt::to_hstring(o.fspl_str));
    out_two_ray_.Text(winrt::to_hstring(o.two_ray_loss_str));
    out_fresnel_.Text(winrt::to_hstring(o.fresnel_zone_str));
    out_regime_.Text(winrt::to_hstring(o.regime_str));
}

} // namespace ewcalc
