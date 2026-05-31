// MainWindow.xaml.cs
using EwCalc.Views;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Navigation;
using System;

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
    ];

    public MainWindow()
    {
        InitializeComponent();
        AppWindow.Resize(new Windows.Graphics.SizeInt32(1024, 700));
        NavView.SelectedItem = NavPropagation;
        ContentFrame.Navigate(typeof(PropagationPage));
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
