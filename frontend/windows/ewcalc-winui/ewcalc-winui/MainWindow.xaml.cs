// MainWindow.xaml.cs
using EwCalc.Views;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Navigation;
using System;
using System.IO;

namespace EwCalc;

public sealed partial class MainWindow : Window
{
    private static readonly (string Tag, Type Page)[] PageMap =
    [
        ("Propagation", typeof(PropagationPage)),
        ("Link",        typeof(LinkPage)),
        ("Receiver",    typeof(ReceiverPage)),
        ("Jamming",     typeof(JammingPage)),
        ("Location",    typeof(LocationPage)),
        ("Radar",       typeof(RadarPage)),
        ("Digital",     typeof(DigitalPage)),
        ("Antenna",     typeof(AntennaPage)),
        ("Reference",   typeof(ReferencePage)),
    ];

    public MainWindow()
    {
        InitializeComponent();
        AppWindow.Resize(new Windows.Graphics.SizeInt32(860, 660));
        AppWindow.SetIcon(Path.Combine(AppContext.BaseDirectory, "Assets", "icon.ico"));
        NavView.SelectedItem = NavPropagation;
        ContentFrame.Navigate(typeof(PropagationPage));
        Closed += (_, _) => Helpers.SettingsService.Flush();
    }

    private void ResetInputsButton_Click(object sender, RoutedEventArgs e)
    {
        Helpers.SettingsService.ResetToDefaults();
        // Re-navigating to the current page type creates a fresh Page/ViewModel/adapter
        // (NavigationCacheMode is Disabled by default), so the active page immediately
        // reflects the reset. Other pages pick it up next time they're navigated to.
        var current = ContentFrame.CurrentSourcePageType;
        if (current is not null) ContentFrame.Navigate(current);
    }

    private void NavView_SelectionChanged(
        NavigationView sender,
        NavigationViewSelectionChangedEventArgs args)
    {
        if (args.IsSettingsSelected) return;

        if (args.SelectedItem is NavigationViewItem item &&
            item.Tag is string tag)
        {
            foreach (var (pageTag, pageType) in PageMap)
            {
                if (pageTag == tag)
                {
                    if (ContentFrame.CurrentSourcePageType != pageType)
                        ContentFrame.Navigate(pageType);
                    return;
                }
            }
        }
    }
}
