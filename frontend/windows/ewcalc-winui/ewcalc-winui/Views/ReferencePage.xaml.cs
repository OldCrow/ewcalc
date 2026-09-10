// Views/ReferencePage.xaml.cs
//
// Renders ewpresenter::refdata content (#73) via the RefData interop
// wrapper. Row counts and shapes vary per section (and a page can mix Value
// and Formula rows), so the tree is built once, in code, at construction
// time rather than templated in XAML — mirrors the "chrome in XAML, content
// in code-behind" split the goal for #73 calls for.
using EwCalc.Helpers;
using EwPresenterNet;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media.Imaging;

namespace EwCalc.Views;

public sealed partial class ReferencePage : Page
{
    // The Reference tab currently shows a single refdata::Page ("quick-values");
    // additional pages would need navigation, out of scope for #73.
    private const int PageIndex = 0;

    public ReferencePage()
    {
        InitializeComponent();
        BuildContent();
    }

    private void BuildContent()
    {
        var pages = RefData.GetPages();
        if (pages.Length <= PageIndex) return;

        var page = pages[PageIndex];
        PageTitleBlock.Text = page.Title;
        PageSubtitleBlock.Text = page.Subtitle;

        foreach (var section in page.Sections)
        {
            ItemHost.Children.Add(new TextBlock
            {
                // Uppercase headers to match the app's section-header look, but
                // restore case-sensitive physics notation (Eb = energy per bit).
                Text = section.Title.ToUpperInvariant().Replace("EB/N₀", "Eb/N₀"),
                Style = (Style)Application.Current.Resources["SectionHeaderStyle"],
            });

            var card = new Border { Style = (Style)Application.Current.Resources["ResultCardStyle"] };
            var rows = new StackPanel { Spacing = 0 };
            foreach (var row in section.Rows)
                rows.Children.Add(row.RowKind == RefRowKind.Formula ? BuildFormulaRow(row) : BuildValueRow(row));
            card.Child = rows;
            ItemHost.Children.Add(card);
        }
    }

    // Declared as FrameworkElement (rather than the concrete Grid/StackPanel each
    // returns) so the row.RowKind ternary in BuildContent has a common type to
    // resolve to — Grid and StackPanel are unrelated sibling Panel types with no
    // direct implicit conversion between them.
    private static FrameworkElement BuildValueRow(RefRow row)
    {
        var grid = new Grid { MinHeight = 36, Padding = new Thickness(0, 4, 0, 4) };
        grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });
        grid.ColumnDefinitions.Add(new ColumnDefinition { Width = GridLength.Auto });
        grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(32) });

        var label = new TextBlock
        {
            Text = row.Label,
            VerticalAlignment = VerticalAlignment.Center,
        };
        // ResultLabelStyle/ResultValueStyle live in App.xaml; StaticResource isn't
        // available for elements built in code, so resolve them via Application.Resources.
        label.Style = (Style)Application.Current.Resources["ResultLabelStyle"];
        Grid.SetColumn(label, 0);

        var value = new TextBlock
        {
            Text = row.Value,
            VerticalAlignment = VerticalAlignment.Center,
            TextAlignment = TextAlignment.Right,
            Style = (Style)Application.Current.Resources["ResultValueStyle"],
        };
        Grid.SetColumn(value, 1);

        grid.Children.Add(label);
        grid.Children.Add(value);

        if (row.CopyValue is string copyValue)
        {
            var button = MakeCopyButton($"Copy {row.Label}", copyValue);
            Grid.SetColumn(button, 2);
            grid.Children.Add(button);
        }
        else
        {
            var placeholder = new Border { Width = 32 };
            Grid.SetColumn(placeholder, 2);
            grid.Children.Add(placeholder);
        }

        return grid;
    }

    private static FrameworkElement BuildFormulaRow(RefRow row)
    {
        var container = new StackPanel { Spacing = 8, Margin = new Thickness(0, 4, 0, 4) };

        var header = new Grid();
        header.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });
        header.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(32) });

        var name = new TextBlock
        {
            Text = row.Label,
            VerticalAlignment = VerticalAlignment.Center,
            Style = (Style)Application.Current.Resources["ResultLabelStyle"],
        };
        Grid.SetColumn(name, 0);
        header.Children.Add(name);

        // Formula rows never carry a copy_value of their own — the copy button
        // always copies the standard-form Unicode text (row.Value), matching
        // the C bridge's ewp_ref_row_copy_value contract for formula rows.
        var copyButton = MakeCopyButton($"Copy {row.Label} formula", row.Value);
        Grid.SetColumn(copyButton, 1);
        header.Children.Add(copyButton);

        container.Children.Add(header);

        var forms = new StackPanel { Orientation = Orientation.Horizontal, Spacing = 16 };
        forms.Children.Add(MakeFormulaImage(row.SvgBase, "std", row.Value));
        if (row.LogValue is string logValue)
            forms.Children.Add(MakeFormulaImage(row.SvgBase, "log", logValue));
        container.Children.Add(forms);

        return container;
    }

    private static Button MakeCopyButton(string automationName, string copyText)
    {
        var button = new Button
        {
            Content = "\xE8C8",
            FontFamily = new Microsoft.UI.Xaml.Media.FontFamily("Segoe MDL2 Assets"),
            FontSize = 12,
            Padding = new Thickness(4),
            VerticalAlignment = VerticalAlignment.Center,
        };
        Microsoft.UI.Xaml.Automation.AutomationProperties.SetName(button, automationName);
        button.Click += (_, _) => ClipboardHelper.CopyText(copyText);
        return button;
    }

    // The formula PNGs are 2x renders (assets/formulas/png, see
    // scripts/render-diagrams.sh); displaying them at native pixel size would
    // show a formula twice the size of the surrounding UI. Reading
    // PixelWidth/PixelHeight from the decoded bitmap and halving it — rather
    // than hardcoding the known logical size — keeps this correct if the
    // render script's scale factor ever changes.
    private static Image MakeFormulaImage(string svgBase, string suffix, string altText)
    {
        var image = new Image
        {
            Source = new BitmapImage(new System.Uri($"ms-appx:///Assets/Formulas/{svgBase}-{suffix}.png")),
        };
        Microsoft.UI.Xaml.Automation.AutomationProperties.SetName(image, altText);
        ToolTipService.SetToolTip(image, altText);
        image.ImageOpened += FormulaImage_ImageOpened;
        return image;
    }

    private static void FormulaImage_ImageOpened(object sender, RoutedEventArgs e)
    {
        if (sender is Image { Source: BitmapImage bitmap } image)
        {
            image.Width = bitmap.PixelWidth / 2.0;
            image.Height = bitmap.PixelHeight / 2.0;
        }
    }
}
