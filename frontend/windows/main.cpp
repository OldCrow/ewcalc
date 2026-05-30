/// @file main.cpp
/// @brief ewcalc Windows frontend — Phase 3 proof-of-concept.
///
/// Bootstraps the Windows App Runtime, creates a WinUI 3 window
/// programmatically (no XAML), and displays a live propagation result
/// from the ewpresenter layer to confirm the full stack is wired.
///
/// Build: cmake -B build -DEWCALC_BUILD_GUI=ON
/// Run:   winapp run .\build\bin\Release  (for package identity during development)
///        or directly: .\build\bin\Release\ewcalc.exe (unpackaged, runtime must be installed)

// WinRT headers must precede <windows.h> includes
// NOMINMAX / WIN32_LEAN_AND_MEAN / WINRT_LEAN_AND_MEAN set via CMake target_compile_definitions
#include <windows.h>
// GetCurrentTime is a Win32 macro that conflicts with WinUI 3 animation APIs
#undef GetCurrentTime

// C++/WinRT
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>

// Windows App SDK bootstrap
#include <MddBootstrap.h>
#include <WindowsAppSDK-VersionInfo.h>

// ewpresenter
#include "ewpresenter/ewpresenter.h"

// ---------------------------------------------------------------------------
// Namespace aliases
// ---------------------------------------------------------------------------
namespace xaml    = winrt::Microsoft::UI::Xaml;
namespace controls = winrt::Microsoft::UI::Xaml::Controls;

// ---------------------------------------------------------------------------
// Application class — subclasses WinUI 3 Application, no XAML required
// ---------------------------------------------------------------------------
namespace ewcalc::implementation {

struct App : xaml::ApplicationT<App> {
    void OnLaunched([[maybe_unused]] xaml::LaunchActivatedEventArgs const&) {
        build_window();
    }

private:
    xaml::Window window_{nullptr};

    void build_window() {
        window_ = xaml::Window{};
        window_.Title(L"ewcalc \u2014 EW Engineering Calculator");

        // Root layout
        auto root = controls::StackPanel{};
        root.Margin({16, 16, 16, 16});
        root.Spacing(8.0);

        // Title label
        auto title = controls::TextBlock{};
        title.Text(L"ewcalc \u2014 EW Engineering Calculator");
        title.FontSize(22.0);
        root.Children().Append(title);

        // Subtitle / version
        auto version = controls::TextBlock{};
        version.Text(L"Phase 3 proof-of-concept \u2014 Windows App SDK 2.1 / WinUI 3 / C++20");
        version.FontSize(12.0);
        root.Children().Append(version);

        // Separator-like spacer
        auto sep = controls::TextBlock{};
        sep.Text(L"\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500");
        root.Children().Append(sep);

        // Live result from ewpresenter to verify the full stack
        auto result_label = controls::TextBlock{};
        result_label.Text(L"Propagation (32.6 km, 100 MHz, 10 m / 10 m):");
        root.Children().Append(result_label);

        // Run ewpresenter to get a result
        ewpresenter::PropagationPresenter prop;
        prop.set_distance(32.6);
        prop.set_frequency(100.0);
        prop.set_tx_height(10.0);
        prop.set_rx_height(10.0);
        const auto& out = prop.output();

        auto make_row = [&](const wchar_t* label, const std::string& value) {
            auto row = controls::StackPanel{};
            row.Orientation(controls::Orientation::Horizontal);
            row.Spacing(8.0);

            auto lbl = controls::TextBlock{};
            lbl.Text(label);
            lbl.Width(180.0);
            row.Children().Append(lbl);

            auto val = controls::TextBlock{};
            val.Text(winrt::to_hstring(value));
            row.Children().Append(val);

            return row;
        };

        root.Children().Append(make_row(L"Path loss:",          out.path_loss_str));
        root.Children().Append(make_row(L"FSPL:",               out.fspl_str));
        root.Children().Append(make_row(L"2-Ray loss:",         out.two_ray_loss_str));
        root.Children().Append(make_row(L"Fresnel zone:",       out.fresnel_zone_str));
        root.Children().Append(make_row(L"Regime:",             out.regime_str));

        window_.Content(root);
        window_.Activate();
    }
};

} // namespace ewcalc::implementation

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------
int WINAPI WinMain(
    [[maybe_unused]] HINSTANCE hInstance,
    [[maybe_unused]] HINSTANCE hPrevInstance,
    [[maybe_unused]] LPSTR     lpCmdLine,
    [[maybe_unused]] int       nShowCmd)
{
    // Bootstrap the Windows App Runtime (required for unpackaged apps).
    // The runtime was confirmed installed by `winapp init`.
    const HRESULT hr = MddBootstrapInitialize2(
        WINDOWSAPPSDK_RELEASE_MAJORMINOR,
        WINDOWSAPPSDK_RELEASE_VERSION_TAG_W,
        PACKAGE_VERSION{},
        MddBootstrapInitializeOptions_None);

    if (FAILED(hr)) {
        MessageBoxW(
            nullptr,
            L"Failed to initialize Windows App Runtime.\n\n"
            L"Ensure the Windows App SDK 2.x Runtime is installed.\n"
            L"Download from: https://aka.ms/windowsappsdk",
            L"ewcalc \u2014 startup error",
            MB_ICONERROR | MB_OK);
        return static_cast<int>(hr);
    }

    winrt::init_apartment(winrt::apartment_type::single_threaded);

    // Start the WinUI 3 application — this runs the message loop until exit.
    xaml::Application::Start([](auto&&) {
        winrt::make<ewcalc::implementation::App>();
    });

    MddBootstrapShutdown();
    return 0;
}
