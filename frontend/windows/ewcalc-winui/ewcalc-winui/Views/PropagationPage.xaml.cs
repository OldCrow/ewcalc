// Views/PropagationPage.xaml.cs
using EwCalc.Helpers;
using EwCalc.ViewModels;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Navigation;

namespace EwCalc.Views;

public sealed partial class PropagationPage : Page
{
    public PropagationViewModel ViewModel { get; } = new();

    public PropagationPage() {
        InitializeComponent();
        DistanceBox         .Setup(0.01,  10000.0);
        FrequencyBox        .Setup(0.1,  100000.0);
        TxHeightBox         .Setup(0.1,  100000.0);
        RxHeightBox         .Setup(0.1,  100000.0);
        ObstructionHeightBox.Setup(0.0,   10000.0);
    }

    // Frame.Navigate creates a fresh page (and ViewModel) each time this page is
    // navigated to, so the outgoing instance's native adapter must be disposed
    // deterministically here rather than relying on its finalizer.
    protected override void OnNavigatedFrom(NavigationEventArgs e)
    {
        ViewModel.Dispose();
        base.OnNavigatedFrom(e);
    }

    private void DistanceBox_ValueChanged(NumberBox sender, NumberBoxValueChangedEventArgs e)
    { if (!double.IsNaN(e.NewValue)) ViewModel.SetDistance(e.NewValue); }

    private void FrequencyBox_ValueChanged(NumberBox sender, NumberBoxValueChangedEventArgs e)
    { if (!double.IsNaN(e.NewValue)) ViewModel.SetFrequency(e.NewValue); }

    private void TxHeightBox_ValueChanged(NumberBox sender, NumberBoxValueChangedEventArgs e)
    { if (!double.IsNaN(e.NewValue)) ViewModel.SetTxHeight(e.NewValue); }

    private void RxHeightBox_ValueChanged(NumberBox sender, NumberBoxValueChangedEventArgs e)
    { if (!double.IsNaN(e.NewValue)) ViewModel.SetRxHeight(e.NewValue); }

    private void ObstructionHeightBox_ValueChanged(NumberBox sender, NumberBoxValueChangedEventArgs e)
    { if (!double.IsNaN(e.NewValue)) ViewModel.SetObstructionHeight(e.NewValue); }

    private void CopyResultsButton_Click(object sender, Microsoft.UI.Xaml.RoutedEventArgs e)
        => ClipboardHelper.CopyText(ViewModel.BuildResultsText());
}
