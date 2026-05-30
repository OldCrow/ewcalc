#pragma once

/// @file ui_helpers.h
/// @brief Inline helpers for building programmatic WinUI 3 layouts.
///
/// Uses only XAML primitive controls (StackPanel, TextBlock, Border, TextBox)
/// that do not require XamlControlsResources template loading.
/// WinUI 3 custom controls (TabView, NumberBox) are intentionally avoided.

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <string>

namespace ewcalc::ui {

namespace xaml     = winrt::Microsoft::UI::Xaml;
namespace controls = winrt::Microsoft::UI::Xaml::Controls;

// ---------------------------------------------------------------------------
// Section header
// ---------------------------------------------------------------------------
inline controls::TextBlock make_section_header(const wchar_t* text) {
    controls::TextBlock tb{};
    tb.Text(text);
    tb.FontSize(14.0);
    tb.Margin({0, 8, 0, 2});
    return tb;
}

// ---------------------------------------------------------------------------
// TextBox numeric input (replaces NumberBox which needs XamlControlsResources)
// Returns the TextBox; caller wires TextChanged for live updates.
// ---------------------------------------------------------------------------
inline controls::TextBox make_number_input(const wchar_t* label, double value) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%g", value);

    controls::TextBox tb{};
    tb.Header(winrt::box_value(label));
    tb.Text(winrt::to_hstring(std::string{buf}));
    tb.Width(220.0);
    tb.HorizontalAlignment(xaml::HorizontalAlignment::Left);
    return tb;
}

/// Parse a TextBox value to double; returns NaN on failure.
inline double parse_textbox(controls::TextBox const& tb) noexcept {
    try {
        return std::stod(std::wstring(tb.Text().c_str()));
    } catch (...) {
        return std::numeric_limits<double>::quiet_NaN();
    }
}

// ---------------------------------------------------------------------------
// Result row — label + updatable value TextBlock
// ---------------------------------------------------------------------------
inline std::pair<controls::StackPanel, controls::TextBlock>
make_result_row(const wchar_t* label, const std::string& initial_value) {
    controls::StackPanel row{};
    row.Orientation(controls::Orientation::Horizontal);
    row.Spacing(8.0);

    controls::TextBlock lbl{};
    lbl.Text(label);
    lbl.Width(200.0);
    lbl.VerticalAlignment(xaml::VerticalAlignment::Center);

    controls::TextBlock val{};
    val.Text(winrt::to_hstring(initial_value));
    val.VerticalAlignment(xaml::VerticalAlignment::Center);

    row.Children().Append(lbl);
    row.Children().Append(val);

    return {row, val};
}

// ---------------------------------------------------------------------------
// Panel root — ScrollViewer + inner StackPanel
// ---------------------------------------------------------------------------
inline std::pair<controls::ScrollViewer, controls::StackPanel> make_panel_root() {
    controls::StackPanel content{};
    content.Margin({16, 12, 16, 16});
    content.Spacing(6.0);

    controls::ScrollViewer scroll{};
    scroll.Content(content);
    scroll.VerticalScrollBarVisibility(controls::ScrollBarVisibility::Auto);
    scroll.HorizontalScrollBarVisibility(controls::ScrollBarVisibility::Disabled);

    return {scroll, content};
}

} // namespace ewcalc::ui
