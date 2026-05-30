#pragma once
#include "../ui_helpers.h"
#include "ewpresenter/radar_presenter.h"

namespace ewcalc {

class RadarPanel {
public:
    RadarPanel();
    [[nodiscard]] winrt::Microsoft::UI::Xaml::UIElement root() const noexcept;

private:
    ewpresenter::RadarPresenter presenter_;
    winrt::Microsoft::UI::Xaml::Controls::ScrollViewer root_;
    winrt::Microsoft::UI::Xaml::Controls::TextBox tb_pwr_, tb_gain_, tb_freq_;
    winrt::Microsoft::UI::Xaml::Controls::TextBox tb_rcs_, tb_loss_, tb_nf_, tb_bw_, tb_snr_;
    winrt::Microsoft::UI::Xaml::Controls::TextBox tb_tb_, tb_np_;
    winrt::Microsoft::UI::Xaml::Controls::TextBlock out_range_, out_two_way_loss_, out_pc_gain_, out_coh_gain_;
    void update_outputs();
};

} // namespace ewcalc
