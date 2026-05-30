#include "main_window.h"
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Windows.Foundation.Collections.h>

/// @note Phase 3 PoC — TextBlock-only display using only XAML primitives.
/// WinUI 3 controls (TextBox, ScrollViewer, TabView, Button, NumberBox) all
/// require XamlControlsResources template loading which is only available
/// through the XAML compiler (App.xaml). A proper tabbed UI with interactive
/// inputs requires a XAML-based VS project (Phase 3.1).

namespace ewcalc {

namespace xaml     = winrt::Microsoft::UI::Xaml;
namespace controls = winrt::Microsoft::UI::Xaml::Controls;

static controls::TextBlock make_row(const wchar_t* label, const std::string& value) {
    controls::TextBlock tb{};
    wchar_t buf[256];
    std::swprintf(buf, 256, L"%-22ls %hs", label, value.c_str());
    tb.Text(buf);
    tb.FontFamily(winrt::Microsoft::UI::Xaml::Media::FontFamily{L"Consolas"});
    return tb;
}

MainWindow::MainWindow(xaml::Window const& window) : window_{window} {
    window_.Title(L"ewcalc \u2014 EW Engineering Calculator (Phase 3 PoC)");

    controls::StackPanel root{};
    root.Margin({16, 16, 16, 16});
    root.Spacing(4.0);

    // Title
    controls::TextBlock title{};
    title.Text(L"ewcalc  \u2014  EW Engineering Calculator");
    title.FontSize(18.0);
    root.Children().Append(title);

    controls::TextBlock sub{};
    sub.Text(L"Phase 3 proof-of-concept  |  libew \u2192 ewpresenter \u2192 WinUI 3");
    sub.FontSize(11.0);
    root.Children().Append(sub);

    controls::TextBlock sep{};
    sep.Text(L"\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500");
    root.Children().Append(sep);

    // Propagation panel results (default inputs from presenter)
    controls::TextBlock hdr_prop{};
    hdr_prop.Text(L"Propagation  (32.6 km / 100 MHz / 10 m)");
    hdr_prop.FontSize(13.0);
    hdr_prop.Margin({0, 8, 0, 2});
    root.Children().Append(hdr_prop);
    const auto& p = propagation_presenter_.output();
    root.Children().Append(make_row(L"Path loss:",   p.path_loss_str));
    root.Children().Append(make_row(L"FSPL:",         p.fspl_str));
    root.Children().Append(make_row(L"2-Ray loss:",   p.two_ray_loss_str));
    root.Children().Append(make_row(L"Fresnel zone:", p.fresnel_zone_str));
    root.Children().Append(make_row(L"Regime:",       p.regime_str));

    // Receiver results
    controls::TextBlock hdr_rx{};
    hdr_rx.Text(L"Receiver  (0.1 MHz / 6.5 dB NF / 15 dB SNR)");
    hdr_rx.FontSize(13.0);
    hdr_rx.Margin({0, 8, 0, 2});
    root.Children().Append(hdr_rx);
    const auto& r = receiver_presenter_.output();
    root.Children().Append(make_row(L"Sensitivity:",  r.sensitivity_str));
    root.Children().Append(make_row(L"Cascaded NF:",  r.cascaded_nf_str));
    root.Children().Append(make_row(L"Digital DR:",   r.digital_dr_str));

    controls::TextBlock note{};
    note.Text(L"\u2500\u2500  Full tabbed UI with interactive inputs requires XAML compiler (Phase 3.1)  \u2500\u2500");
    note.FontSize(10.0);
    note.Margin({0, 12, 0, 0});
    root.Children().Append(note);

    window_.Content(root);
    window_.Activate();
}

} // namespace ewcalc
