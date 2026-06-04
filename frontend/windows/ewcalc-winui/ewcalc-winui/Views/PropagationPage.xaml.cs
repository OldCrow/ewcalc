// Views/PropagationPage.xaml.cs
using EwCalc.Helpers;
using EwCalc.ViewModels;
using Microsoft.UI.Xaml.Controls;

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
}
