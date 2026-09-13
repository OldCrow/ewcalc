// Views/ReferencePage.xaml.cs
//
// Renders ewpresenter::refdata content (#73/#74) via the RefData interop
// wrapper. Row counts and shapes vary per section (and a page can mix Value
// and Formula rows), so the tree is built once, in code, at construction
// time rather than templated in XAML — mirrors the "chrome in XAML, content
// in code-behind" split the goal for #73 calls for.
//
// Which refdata::Page to render is a navigation parameter (#74: refdata now
// publishes multiple pages, one per Reference nav item — see MainWindow's
// dynamically built nav items and NavView_SelectionChanged), not a
// hardcoded index.
using EwCalc.Helpers;
using EwPresenterNet;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media.Imaging;
using Microsoft.UI.Xaml.Navigation;
using System.Text.RegularExpressions;

namespace EwCalc.Views;

public sealed partial class ReferencePage : Page
{
    // Frame.Navigate(sourcePageType, parameter) is a no-op (per WinUI's Frame,
    // it skips navigating and OnNavigatedTo never fires) when both the target
    // page type and parameter already match the current navigation entry.
    // MainWindow's reset-inputs handler needs to force a reload of the
    // currently displayed reference page, so it wraps the page index in a
    // fresh instance of this reference-equality-only token instead of passing
    // the raw int back.
    internal sealed class ReloadToken
    {
        public required int PageIndex { get; init; }
    }

    public ReferencePage()
    {
        InitializeComponent();
    }

    // Frame.Navigate(typeof(ReferencePage), pageIndex) delivers the selected
    // refdata page index here; content is built on arrival rather than in the
    // constructor since the navigation parameter isn't known until then.
    protected override void OnNavigatedTo(NavigationEventArgs e)
    {
        base.OnNavigatedTo(e);
        var pageIndex = e.Parameter switch
        {
            int index => index,
            ReloadToken token => token.PageIndex,
            _ => 0,
        };
        BuildContent(pageIndex);
    }

    private void BuildContent(int pageIndex)
    {
        ItemHost.Children.Clear();

        var pages = RefData.GetPages();
        if (pages.Length <= pageIndex) return;

        var page = pages[pageIndex];
        PageTitleBlock.Text = page.Title;
        PageSubtitleBlock.Text = page.Subtitle;

        foreach (var section in page.Sections)
        {
            ItemHost.Children.Add(new TextBlock
            {
                Text = SectionHeaderText(section.Title),
                Style = (Style)Application.Current.Resources["SectionHeaderStyle"],
            });

            var card = new Border { Style = (Style)Application.Current.Resources["ResultCardStyle"] };
            var rows = new StackPanel { Spacing = 0 };

            if (section.Diagram is string diagram)
                rows.Children.Add(BuildSectionDiagram(diagram));

            foreach (var row in section.Rows)
            {
                // Branch rather than a ternary: the two builders return unrelated
                // Panel/Grid types, so a ternary would need a common FrameworkElement
                // return type and CA1859 flags that as an avoidable abstraction.
                // Children.Add takes UIElement, so each concrete type binds directly.
                if (row.RowKind == RefRowKind.Formula)
                    rows.Children.Add(BuildFormulaRow(row));
                else
                    rows.Children.Add(BuildValueRow(row));
            }
            card.Child = rows;
            ItemHost.Children.Add(card);
        }
    }

    // Section geometry thumbnail (#72 diagram PNG, packaged the same way the
    // calculator pages' Expander sections reference them — see
    // PropagationPage.xaml's Assets/Diagrams images), reduced to a ~480
    // logical-pixel max width and centered above the section's rows.
    private static Image BuildSectionDiagram(string diagram)
    {
        var image = new Image
        {
            Source = new BitmapImage(new System.Uri($"ms-appx:///Assets/Diagrams/{diagram}.png")),
            MaxWidth = 480,
            Stretch = Microsoft.UI.Xaml.Media.Stretch.Uniform,
            HorizontalAlignment = HorizontalAlignment.Center,
            Margin = new Thickness(0, 4, 0, 8),
        };
        var altText = diagram.Replace("-", " ");
        Microsoft.UI.Xaml.Automation.AutomationProperties.SetName(image, altText);
        ToolTipService.SetToolTip(image, altText);
        return image;
    }

    // Case-sensitive notation that ToUpperInvariant would corrupt ("dB" -> "DB",
    // "kT" -> "KT"). Section headers are uppercased to match the calculator
    // pages' header look (macOS and Linux show titles as authored); these
    // tokens keep their source spelling. Extend the list when a refdata
    // section title gains other unit or symbol notation.
    private static readonly string[] CaseSensitiveTokens =
    [
        "dB", "dBm", "dBW", "dBi", "dBd", "dBc", "dBsm",
        "Hz", "kHz", "MHz", "GHz", "kT", "Eb/N₀",
    ];

    private static string SectionHeaderText(string title)
    {
        // ToUpperInvariant maps char-for-char, so a token's index in the source
        // title is its index in the uppercased copy too.
        var chars = title.ToUpperInvariant().ToCharArray();
        foreach (var token in CaseSensitiveTokens)
        {
            // Letter lookarounds keep "dB" from matching inside "dBm" and the like.
            foreach (Match m in Regex.Matches(title, $@"(?<!\p{{L}}){Regex.Escape(token)}(?!\p{{L}})"))
                token.CopyTo(0, chars, m.Index, token.Length);
        }
        return new string(chars);
    }

    // Label column sized to the label, value column taking the rest and
    // word-wrapping: Glossary definitions and other prose values would
    // otherwise size an Auto column to their unwrapped width, crushing the
    // label to nothing and clipping the value and its copy button at the
    // default 860 px window. Matches Linux (word-wrapped value labels) and
    // macOS (Text wraps by default).
    private static Grid BuildValueRow(RefRow row)
    {
        var grid = new Grid { MinHeight = 36, Padding = new Thickness(0, 4, 0, 4) };
        grid.ColumnDefinitions.Add(new ColumnDefinition { Width = GridLength.Auto });
        grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });
        grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(32) });

        var label = new TextBlock
        {
            Text = row.Label,
            VerticalAlignment = VerticalAlignment.Center,
            // Cap the label so a long one cannot starve the value column.
            MaxWidth = 280,
            TextWrapping = TextWrapping.Wrap,
            Margin = new Thickness(0, 0, 16, 0),
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
            TextWrapping = TextWrapping.Wrap,
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

    private static StackPanel BuildFormulaRow(RefRow row)
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

        // Formula rows never carry a copy_value of their own (the C bridge's
        // ewp_ref_row_copy_value returns null for them), so the copy text is
        // composed here from the Unicode forms. Both other frontends join the
        // standard and log forms with a three-space pipe — keep that verbatim
        // so a copied formula is identical across platforms.
        var copyText = row.LogValue is string log ? $"{row.Value}   |   {log}" : row.Value;
        var copyButton = MakeCopyButton($"Copy {row.Label} formula", copyText);
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
            // Natural size is half the 2x asset's pixels. MaxWidth (not a
            // fixed Width) with Stretch=Uniform lets a wide master — the
            // stand-off J/S or the radar-range log line — shrink with the
            // container instead of clipping at narrow windows, and never
            // upscale past natural. Height follows via Uniform.
            image.MaxWidth = bitmap.PixelWidth / 2.0;
            image.MaxHeight = bitmap.PixelHeight / 2.0;
            image.Stretch = Microsoft.UI.Xaml.Media.Stretch.Uniform;
        }
    }
}
