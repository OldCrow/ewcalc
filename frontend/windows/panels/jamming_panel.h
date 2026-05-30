#pragma once
#include "../ui_helpers.h"
#include "ewpresenter/jamming_presenter.h"

namespace ewcalc {

class JammingPanel {
public:
    JammingPanel();
    [[nodiscard]] winrt::Microsoft::UI::Xaml::UIElement root() const noexcept;

private:
    ewpresenter::JammingPresenter presenter_;
    winrt::Microsoft::UI::Xaml::Controls::ScrollViewer root_;
    winrt::Microsoft::UI::Xaml::Controls::TextBox tb_serp_, tb_sdist_, tb_sht_;
    winrt::Microsoft::UI::Xaml::Controls::TextBox tb_jerp_, tb_jdist_, tb_jht_;
    winrt::Microsoft::UI::Xaml::Controls::TextBox tb_rxht_, tb_freq_, tb_sbw_, tb_hop_;
    winrt::Microsoft::UI::Xaml::Controls::TextBlock out_js_, out_signal_rx_, out_jammer_rx_;
    winrt::Microsoft::UI::Xaml::Controls::TextBlock out_opt_bw_, out_duty_cycle_;
    void update_outputs();
};

} // namespace ewcalc
