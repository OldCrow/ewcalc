#pragma once
#include "../ui_helpers.h"
#include "ewpresenter/receiver_presenter.h"

namespace ewcalc {

class ReceiverPanel {
public:
    ReceiverPanel();
    [[nodiscard]] winrt::Microsoft::UI::Xaml::UIElement root() const noexcept;

private:
    ewpresenter::ReceiverPresenter presenter_;
    winrt::Microsoft::UI::Xaml::Controls::ScrollViewer root_;
    winrt::Microsoft::UI::Xaml::Controls::TextBox tb_bw_, tb_nf_, tb_snr_;
    winrt::Microsoft::UI::Xaml::Controls::TextBox tb_ip2_, tb_ip3_, tb_bits_;
    winrt::Microsoft::UI::Xaml::Controls::TextBlock out_sensitivity_, out_cascaded_nf_;
    winrt::Microsoft::UI::Xaml::Controls::TextBlock out_sfdr2_, out_sfdr3_, out_digital_dr_;
    void update_outputs();
};

} // namespace ewcalc
