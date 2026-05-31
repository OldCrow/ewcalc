// Views/PropagationPage.xaml.cs
using EwCalc.ViewModels;
using Microsoft.UI.Xaml.Controls;

namespace EwCalc.Views;

public sealed partial class PropagationPage : Page
{
    public PropagationViewModel ViewModel { get; } = new();

    public PropagationPage() { InitializeComponent(); }

    private void DistanceBox_ValueChanged(NumberBox sender, NumberBoxValueChangedEventArgs e)
    { if (!double.IsNaN(e.NewValue)) ViewModel.SetDistance(e.NewValue); }

    private void FrequencyBox_ValueChanged(NumberBox sender, NumberBoxValueChangedEventArgs e)
    { if (!double.IsNaN(e.NewValue)) ViewModel.SetFrequency(e.NewValue); }

    private void TxHeightBox_ValueChanged(NumberBox sender, NumberBoxValueChangedEventArgs e)
    { if (!double.IsNaN(e.NewValue)) ViewModel.SetTxHeight(e.NewValue); }

    private void RxHeightBox_ValueChanged(NumberBox sender, NumberBoxValueChangedEventArgs e)
    { if (!double.IsNaN(e.NewValue)) ViewModel.SetRxHeight(e.NewValue); }
}
