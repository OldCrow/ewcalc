// Helpers/PageHelper.cs
// WinUI 3 XAML parses NumberBox.Minimum/Maximum from string through float32,
// so Minimum="0.01" becomes 0.009999999776... instead of 0.01 as double.
// All NumberBox min/max and formatters must be set in code, not XAML.

using Microsoft.UI.Xaml.Controls;
using Windows.Globalization.NumberFormatting;

namespace EwCalc.Helpers;

public static class PageHelper
{
    /// <summary>
    /// Shared formatter: 3 decimal places, no thousands separator.
    /// Prevents IEEE 754 artefacts like 0.009999... displaying instead of 0.010.
    /// </summary>
    public static readonly DecimalFormatter Fmt = new()
    {
        FractionDigits = 3,
        IsGrouped      = false,
    };

    /// <summary>Configure a NumberBox: set precise double bounds and the shared formatter.</summary>
    public static void Setup(this NumberBox nb, double min, double max)
    {
        nb.Minimum         = min;
        nb.Maximum         = max;
        nb.NumberFormatter = Fmt;
    }
}
