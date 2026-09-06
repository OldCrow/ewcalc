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
        ("Detection",   typeof(DetectionPage)),
        ("Doppler",     typeof(DopplerPage)),
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
        // Force a same-page reload by changing the navigation parameter; plain
        // Navigate(current) can short-circuit when the page type is unchanged.
        var current = ContentFrame.CurrentSourcePageType;
        if (current is not null) ContentFrame.Navigate(current, Guid.NewGuid());
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
