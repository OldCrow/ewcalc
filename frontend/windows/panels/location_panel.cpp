#include "location_panel.h"
#include <cmath>

namespace ewcalc {

using namespace ui;

LocationPanel::LocationPanel() {
    auto [scroll, stack] = make_panel_root();
    root_ = scroll;

    stack.Children().Append(make_section_header(L"AOA Bearing Error Method"));
    auto tb_err = make_number_input(L"RMS bearing error (deg)", presenter_.rms_bearing_error_deg());
    auto tb_rng = make_number_input(L"Range to emitter (km)",   presenter_.aoa_range_km());
    stack.Children().Append(tb_err); stack.Children().Append(tb_rng);

    stack.Children().Append(make_section_header(L"Elliptical Error Probable Method"));
    auto tb_maj = make_number_input(L"Semi-major axis (km)", presenter_.semi_major_km());
    auto tb_min = make_number_input(L"Semi-minor axis (km)", presenter_.semi_minor_km());
    stack.Children().Append(tb_maj); stack.Children().Append(tb_min);

    stack.Children().Append(make_section_header(L"Results"));
    auto [raoa, oa] = make_result_row(L"CEP (AOA):", presenter_.output().cep_aoa_str);
    auto [reep, oe] = make_result_row(L"CEP (EEP):", presenter_.output().cep_eep_str);
    out_cep_aoa_=oa; out_cep_eep_=oe;
    stack.Children().Append(raoa); stack.Children().Append(reep);

    tb_err.TextChanged([this](auto&&,auto&&){ double v=parse_textbox(tb_err_); if(!std::isnan(v)){presenter_.set_rms_bearing_error(v); update_outputs();} });
    tb_rng.TextChanged([this](auto&&,auto&&){ double v=parse_textbox(tb_rng_); if(!std::isnan(v)){presenter_.set_aoa_range(v);         update_outputs();} });
    tb_maj.TextChanged([this](auto&&,auto&&){ double v=parse_textbox(tb_maj_); if(!std::isnan(v)){presenter_.set_semi_major(v);        update_outputs();} });
    tb_min.TextChanged([this](auto&&,auto&&){ double v=parse_textbox(tb_min_); if(!std::isnan(v)){presenter_.set_semi_minor(v);        update_outputs();} });

    tb_err_=tb_err; tb_rng_=tb_rng; tb_maj_=tb_maj; tb_min_=tb_min;
}

winrt::Microsoft::UI::Xaml::UIElement LocationPanel::root() const noexcept { return root_; }

void LocationPanel::update_outputs() {
    const auto& o = presenter_.output();
    out_cep_aoa_.Text(winrt::to_hstring(o.cep_aoa_str));
    out_cep_eep_.Text(winrt::to_hstring(o.cep_eep_str));
}

} // namespace ewcalc
