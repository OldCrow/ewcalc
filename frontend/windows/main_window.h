#pragma once
#include "ewpresenter/propagation_presenter.h"
#include "ewpresenter/receiver_presenter.h"
#include <winrt/Microsoft.UI.Xaml.h>

/// Phase 3 PoC: TextBlock-only display proving libew -> ewpresenter -> WinUI 3.
/// Phase 3.1 will replace this with a XAML-compiler-based VS project that
/// supports the full WinUI 3 control set (TabView, TextBox, ScrollViewer, etc.).

namespace ewcalc {

class MainWindow {
public:
    explicit MainWindow(winrt::Microsoft::UI::Xaml::Window const& window);
private:
    winrt::Microsoft::UI::Xaml::Window window_;
    ewpresenter::PropagationPresenter propagation_presenter_;
    ewpresenter::ReceiverPresenter    receiver_presenter_;
};

} // namespace ewcalc
