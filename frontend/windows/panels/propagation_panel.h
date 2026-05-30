#pragma once
#include "../ui_helpers.h"
#include "ewpresenter/propagation_presenter.h"
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace ewcalc {

class PropagationPanel {
public:
    PropagationPanel();
    [[nodiscard]] winrt::Microsoft::UI::Xaml::UIElement root() const noexcept;

private:
    ewpresenter::PropagationPresenter presenter_;
    winrt::Microsoft::UI::Xaml::Controls::ScrollViewer root_;
    // Input TextBoxes (held to read values in TextChanged handlers)
    winrt::Microsoft::UI::Xaml::Controls::TextBox tb_dist_;
    winrt::Microsoft::UI::Xaml::Controls::TextBox tb_freq_;
    winrt::Microsoft::UI::Xaml::Controls::TextBox tb_ht_;
    winrt::Microsoft::UI::Xaml::Controls::TextBox tb_hr_;
    // Output TextBlocks
    winrt::Microsoft::UI::Xaml::Controls::TextBlock out_fspl_;
    winrt::Microsoft::UI::Xaml::Controls::TextBlock out_two_ray_;
    winrt::Microsoft::UI::Xaml::Controls::TextBlock out_fresnel_;
    winrt::Microsoft::UI::Xaml::Controls::TextBlock out_path_loss_;
    winrt::Microsoft::UI::Xaml::Controls::TextBlock out_regime_;
    void update_outputs();
};

} // namespace ewcalc
