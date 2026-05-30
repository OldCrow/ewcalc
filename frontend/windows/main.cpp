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
#include <optional>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>

// Windows App SDK bootstrap
#include <MddBootstrap.h>
#include <WindowsAppSDK-VersionInfo.h>

// Main window
#include "main_window.h"

// ---------------------------------------------------------------------------
// Namespace aliases
// ---------------------------------------------------------------------------
namespace xaml = winrt::Microsoft::UI::Xaml;

// ---------------------------------------------------------------------------
// Application class
// ---------------------------------------------------------------------------
namespace ewcalc::implementation {

struct App : xaml::ApplicationT<App> {
    void OnLaunched([[maybe_unused]] xaml::LaunchActivatedEventArgs const&) {
        try {
            main_window_.emplace(xaml::Window{});
        } catch (winrt::hresult_error const& e) {
            MessageBoxW(nullptr,
                e.message().c_str(),
                L"ewcalc \u2014 launch error (hresult)",
                MB_ICONERROR | MB_OK);
        } catch (...) {
            MessageBoxW(nullptr,
                L"An unknown exception occurred during application launch.",
                L"ewcalc \u2014 launch error",
                MB_ICONERROR | MB_OK);
        }
    }
private:
    std::optional<MainWindow> main_window_;
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
