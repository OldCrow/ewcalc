#pragma once
#include "../ui_helpers.h"
#include "ewpresenter/location_presenter.h"

namespace ewcalc {

class LocationPanel {
public:
    LocationPanel();
    [[nodiscard]] winrt::Microsoft::UI::Xaml::UIElement root() const noexcept;

private:
    ewpresenter::LocationPresenter presenter_;
    winrt::Microsoft::UI::Xaml::Controls::ScrollViewer root_;
    winrt::Microsoft::UI::Xaml::Controls::TextBox tb_err_, tb_rng_, tb_maj_, tb_min_;
    winrt::Microsoft::UI::Xaml::Controls::TextBlock out_cep_aoa_, out_cep_eep_;
    void update_outputs();
};

} // namespace ewcalc
