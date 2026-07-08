// Views/ResultRow.xaml.cs
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Automation;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media;

namespace EwCalc.Views;

public sealed partial class ResultRow : UserControl
{
    public static readonly DependencyProperty LabelProperty =
        DependencyProperty.Register(nameof(Label), typeof(string), typeof(ResultRow),
            new PropertyMetadata(string.Empty, OnLabelChanged));

    // Every ResultRow across every page shares this control, so setting the value
    // TextBlock's automation name here (rather than per-page XAML) gives all output
    // controls an accessible name for free (#19).
    private static void OnLabelChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        var row = (ResultRow)d;
        var label = (string)e.NewValue;
        row.LabelBlock.Text = label;
        AutomationProperties.SetName(row.ValueBlock, label);
    }

    public string Label
    {
        get => (string)GetValue(LabelProperty);
        set => SetValue(LabelProperty, value);
    }

    public static readonly DependencyProperty ValueProperty =
        DependencyProperty.Register(nameof(Value), typeof(string), typeof(ResultRow),
            new PropertyMetadata(string.Empty,
                (d, e) => ((ResultRow)d).ValueBlock.Text = (string)e.NewValue));

    public string Value
    {
        get => (string)GetValue(ValueProperty);
        set => SetValue(ValueProperty, value);
    }

    public static readonly DependencyProperty IsHighlightedProperty =
        DependencyProperty.Register(nameof(IsHighlighted), typeof(bool), typeof(ResultRow),
            new PropertyMetadata(false, OnHighlightChanged));

    public bool IsHighlighted
    {
        get => (bool)GetValue(IsHighlightedProperty);
        set => SetValue(IsHighlightedProperty, value);
    }

    private static void OnHighlightChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        var row = (ResultRow)d;
        if ((bool)e.NewValue)
        {
            if (Application.Current.Resources.TryGetValue("SystemAccentColor", out var accent))
                row.ValueBlock.Foreground = new SolidColorBrush((Windows.UI.Color)accent);
        }
        else
        {
            row.ValueBlock.ClearValue(TextBlock.ForegroundProperty);
        }
    }

    public ResultRow() { InitializeComponent(); }
}
