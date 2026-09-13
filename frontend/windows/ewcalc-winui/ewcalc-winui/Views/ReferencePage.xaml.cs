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

            // One Grid per section so every formula row shares its standard-form
            // column: an Auto column sizes to the widest standard form in the
            // section, which puts all of that section's log forms on a common
            // left edge. Laying each row out on its own let every log form start
            // wherever its own standard form happened to end — a ~330 px spread
            // on Propagation. Everything that is not a formula pair (diagram,
            // value row, formula header) spans both columns, so interleaving
            // order is preserved and only the pairs are column-bound.
            var grid = new Grid();
            grid.ColumnDefinitions.Add(new ColumnDefinition
            {
                Width = GridLength.Auto,
                // Cap the shared column so one very wide standard form cannot
                // squeeze the log column away on a narrow window; past the cap
                // the Viewbox in the cell scales that form down.
                MaxWidth = 400,
            });
            grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });

            if (section.Diagram is string diagram)
                AddFullWidth(grid, BuildSectionDiagram(diagram));

            foreach (var row in section.Rows)
            {
                if (row.RowKind == RefRowKind.Formula)
                    AddFormulaRow(grid, row);
                else
                    AddFullWidth(grid, BuildValueRow(row));
            }
            card.Child = grid;
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

    // Appends an element as its own Grid row spanning both columns.
    private static void AddFullWidth(Grid grid, FrameworkElement element)
    {
        grid.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });
        Grid.SetRow(element, grid.RowDefinitions.Count - 1);
        Grid.SetColumnSpan(element, 2);
        grid.Children.Add(element);
    }

    // A formula row occupies two Grid rows: the label and copy button spanning
    // both columns, then the standard form in the shared Auto column with the
    // log form beside it in the star column.
    private static void AddFormulaRow(Grid grid, RefRow row)
    {
        var header = new Grid { Margin = new Thickness(0, 4, 0, 8) };
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

        AddFullWidth(grid, header);

        grid.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });
        var formsRow = grid.RowDefinitions.Count - 1;

        // Each form sits in a Viewbox because a cell hands its child a finite
        // width: DownOnly keeps natural size the ceiling and scales a form down
        // only when its column is too narrow, which is the frontends' shared
        // shrink-never-upscale rule. At any width that fits, both forms render
        // at natural size and so still match glyph for glyph.
        var std = ShrinkToFit(MakeFormulaImage(row.SvgBase, "std", row.Value));
        std.Margin = new Thickness(0, 0, 16, 12);
        Grid.SetRow(std, formsRow);
        Grid.SetColumn(std, 0);
        grid.Children.Add(std);

        if (row.LogValue is string logValue)
        {
            var logForm = ShrinkToFit(MakeFormulaImage(row.SvgBase, "log", logValue));
            logForm.Margin = new Thickness(0, 0, 0, 12);
            Grid.SetRow(logForm, formsRow);
            Grid.SetColumn(logForm, 1);
            grid.Children.Add(logForm);
        }
    }

    private static Viewbox ShrinkToFit(Image image) => new()
    {
        Child = image,
        Stretch = Microsoft.UI.Xaml.Media.Stretch.Uniform,
        StretchDirection = StretchDirection.DownOnly,
        HorizontalAlignment = HorizontalAlignment.Left,
        VerticalAlignment = VerticalAlignment.Center,
    };

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
