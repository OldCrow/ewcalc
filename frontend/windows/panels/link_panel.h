#pragma once
#include "../ui_helpers.h"
#include "ewpresenter/link_presenter.h"

namespace ewcalc {

class LinkPanel {
public:
    LinkPanel();
    [[nodiscard]] winrt::Microsoft::UI::Xaml::UIElement root() const noexcept;

private:
    ewpresenter::LinkPresenter presenter_;
    winrt::Microsoft::UI::Xaml::Controls::ScrollViewer root_;
    winrt::Microsoft::UI::Xaml::Controls::TextBox tb_pwr_, tb_gtx_, tb_grx_;
    winrt::Microsoft::UI::Xaml::Controls::TextBox tb_dist_, tb_ht_, tb_hr_, tb_freq_, tb_sens_;
    winrt::Microsoft::UI::Xaml::Controls::TextBlock out_received_power_, out_path_loss_;
    winrt::Microsoft::UI::Xaml::Controls::TextBlock out_link_margin_, out_fresnel_;
    winrt::Microsoft::UI::Xaml::Controls::TextBlock out_regime_, out_eff_range_, out_range_regime_;
    void update_outputs();
};

} // namespace ewcalc
