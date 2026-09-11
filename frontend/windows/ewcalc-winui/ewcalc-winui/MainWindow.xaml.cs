// MainWindow.xaml.cs
using EwCalc.Views;
using EwPresenterNet;
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
    ];

    public MainWindow()
    {
        InitializeComponent();
        AppWindow.Resize(new Windows.Graphics.SizeInt32(860, 660));
        AppWindow.SetIcon(Path.Combine(AppContext.BaseDirectory, "Assets", "icon.ico"));
        BuildReferenceNavItems();
        NavView.SelectedItem = NavPropagation;
        ContentFrame.Navigate(typeof(PropagationPage));
        Closed += (_, _) => Helpers.SettingsService.Flush();
    }

    // Reference nav items (#74): one per ewpresenter::refdata page, enumerated
    // at startup so a new data-layer page becomes a nav entry automatically,
    // under the same REFERENCE separator/header the single static item used
    // to sit under. Tag is the page's index into RefData.GetPages() (an int,
    // distinct from the calculator items' string tags) — NavView_SelectionChanged
    // branches on the tag's runtime type to route either to a fixed
    // calculator page or to ReferencePage with that index as parameter.
    private void BuildReferenceNavItems()
    {
        NavView.MenuItems.Add(new NavigationViewItemSeparator());
        NavView.MenuItems.Add(new NavigationViewItemHeader { Content = "REFERENCE" });

        var pages = RefData.GetPages();
        for (var i = 0; i < pages.Length; i++)
        {
            var page = pages[i];
            NavView.MenuItems.Add(new NavigationViewItem
            {
                Tag = i,
                Content = page.Title,
                Icon = new FontIcon { Glyph = ReferenceNavGlyph(page.Id) },
            });
        }
    }

    // Icon rule (#74, ratified): a reference page whose domain mirrors a
    // calculator reuses that calculator's exact nav icon glyph; quick-values
    // keeps the glyph the single Reference item used before this page split
    // (E736); ref-db-units gets its own calculator/±-themed glyph; an
    // unrecognized future page id falls back to that same pre-split Reference
    // glyph. Keyed on the page's stable id, not its display title.
    private static string ReferenceNavGlyph(string pageId) => pageId switch
    {
        "quick-values"    => "", // Book — unchanged from the pre-#74 Reference item
        "ref-propagation" => "", // Matches NavPropagation's FontIcon glyph
        "ref-db-units"    => "", // Calculator — dB/units arithmetic
        _                 => "",
    };

    private void ResetInputsButton_Click(object sender, RoutedEventArgs e)
    {
        Helpers.SettingsService.ResetToDefaults();
        // Force a same-page reload by changing the navigation parameter; plain
        // Navigate(current) can short-circuit when the page type is unchanged.
        // ReferencePage's parameter is meaningful (the selected refdata page
        // index) rather than a dummy — reload it with that index preserved,
        // not a Guid, or a reset while on a Reference page would silently
        // jump back to page 0.
        var current = ContentFrame.CurrentSourcePageType;
        if (current == typeof(ReferencePage) && NavView.SelectedItem is NavigationViewItem { Tag: int pageIndex })
            ContentFrame.Navigate(current, new ReferencePage.ReloadToken { PageIndex = pageIndex });
        else if (current is not null)
            ContentFrame.Navigate(current, Guid.NewGuid());
    }

    private void NavView_SelectionChanged(
        NavigationView sender,
        NavigationViewSelectionChangedEventArgs args)
    {
        if (args.IsSettingsSelected) return;

        if (args.SelectedItem is not NavigationViewItem item) return;

        // Reference items carry their RefData.GetPages() index as an int tag;
        // calculator items carry their PageMap key as a string tag.
        if (item.Tag is int pageIndex)
        {
            ContentFrame.Navigate(typeof(ReferencePage), pageIndex);
            return;
        }

        if (item.Tag is string tag)
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
