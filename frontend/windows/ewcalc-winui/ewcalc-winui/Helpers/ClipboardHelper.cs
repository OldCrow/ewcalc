// Helpers/ClipboardHelper.cs
using Windows.ApplicationModel.DataTransfer;

namespace EwCalc.Helpers;

/// <summary>Shared plain-text clipboard copy used by every page's "Copy results" affordance.</summary>
public static class ClipboardHelper
{
    public static void CopyText(string value)
    {
        var pkg = new DataPackage();
        pkg.SetText(value);
        Clipboard.SetContent(pkg);
    }
}
