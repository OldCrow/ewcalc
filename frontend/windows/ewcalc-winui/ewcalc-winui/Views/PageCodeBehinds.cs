// Views/PageCodeBehinds.cs
// Code-behind for Link, Jamming, Location, Radar, Receiver, Home, Digital, and Reference pages.
using EwCalc.Helpers;
using EwCalc.ViewModels;
using Microsoft.UI.Xaml.Controls;
using Windows.ApplicationModel.DataTransfer;

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

    private void DistanceBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)          { if (!double.IsNaN(e.NewValue)) ViewModel.SetDistance(e.NewValue);          }
    private void FrequencyBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)         { if (!double.IsNaN(e.NewValue)) ViewModel.SetFrequency(e.NewValue);         }
    private void TxHeightBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)          { if (!double.IsNaN(e.NewValue)) ViewModel.SetTxHeight(e.NewValue);          }
    private void RxHeightBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)          { if (!double.IsNaN(e.NewValue)) ViewModel.SetRxHeight(e.NewValue);          }
    private void ObstructionHeightBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e) { if (!double.IsNaN(e.NewValue)) ViewModel.SetObstructionHeight(e.NewValue); }
}

public sealed partial class LinkPage : Page
{
    public LinkViewModel ViewModel { get; } = new();
    public LinkPage() {
        InitializeComponent();
        TxPowerBox      .Setup(-50.0,   200.0);
        TxGainBox       .Setup(-30.0,    60.0);
        FrequencyBox    .Setup(0.1,  100000.0);
        DistanceBox     .Setup(0.01,  10000.0);
        TxHeightBox     .Setup(0.1,  100000.0);
        RxHeightBox     .Setup(0.1,  100000.0);
        RxGainBox       .Setup(-30.0,    60.0);
        RxSensitivityBox.Setup(-200.0,    0.0);
    }

    private void TxPowerBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)       { if (!double.IsNaN(e.NewValue)) ViewModel.SetTxPower(e.NewValue);       }
    private void TxGainBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)        { if (!double.IsNaN(e.NewValue)) ViewModel.SetTxGain(e.NewValue);        }
    private void RxGainBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)        { if (!double.IsNaN(e.NewValue)) ViewModel.SetRxGain(e.NewValue);        }
    private void DistanceBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)      { if (!double.IsNaN(e.NewValue)) ViewModel.SetDistance(e.NewValue);      }
    private void TxHeightBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)      { if (!double.IsNaN(e.NewValue)) ViewModel.SetTxHeight(e.NewValue);      }
    private void RxHeightBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)      { if (!double.IsNaN(e.NewValue)) ViewModel.SetRxHeight(e.NewValue);      }
    private void FrequencyBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)     { if (!double.IsNaN(e.NewValue)) ViewModel.SetFrequency(e.NewValue);     }
    private void RxSensitivityBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e) { if (!double.IsNaN(e.NewValue)) ViewModel.SetRxSensitivity(e.NewValue); }
}

public sealed partial class JammingPage : Page
{
    public JammingViewModel ViewModel { get; } = new();
    public JammingPage() {
        InitializeComponent();
        SignalErpBox   .Setup(-100.0,  200.0);
        JammerErpBox   .Setup(-100.0,  200.0);
        SignalDistBox  .Setup(0.01,  10000.0);
        JammerDistBox  .Setup(0.01,  10000.0);
        SignalHeightBox .Setup(0.1, 100000.0);
        JammerHeightBox .Setup(0.1, 100000.0);
        RxHeightBox    .Setup(0.1,  100000.0);
        FrequencyBox   .Setup(0.1,  100000.0);
        SignalBwBox    .Setup(0.001,  1000.0);
        HopRangeBox    .Setup(0.0,   10000.0);
        RxGainSignalBox.Setup(-30.0,    60.0);
        RxGainJammerBox.Setup(-30.0,    60.0);
        JsThresholdBox .Setup(-30.0,    30.0);
    }

    private void SignalErpBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)     { if (!double.IsNaN(e.NewValue)) ViewModel.SetSignalErp(e.NewValue);       }
    private void JammerErpBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)     { if (!double.IsNaN(e.NewValue)) ViewModel.SetJammerErp(e.NewValue);       }
    private void SignalDistBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)    { if (!double.IsNaN(e.NewValue)) ViewModel.SetSignalToRxDist(e.NewValue);  }
    private void JammerDistBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)    { if (!double.IsNaN(e.NewValue)) ViewModel.SetJammerToRxDist(e.NewValue);  }
    private void SignalHeightBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)  { if (!double.IsNaN(e.NewValue)) ViewModel.SetSignalTxHeight(e.NewValue);  }
    private void JammerHeightBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)  { if (!double.IsNaN(e.NewValue)) ViewModel.SetJammerHeight(e.NewValue);    }
    private void RxHeightBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)      { if (!double.IsNaN(e.NewValue)) ViewModel.SetRxHeight(e.NewValue);        }
    private void FrequencyBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)     { if (!double.IsNaN(e.NewValue)) ViewModel.SetFrequency(e.NewValue);       }
    private void SignalBwBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)       { if (!double.IsNaN(e.NewValue)) ViewModel.SetSignalBandwidth(e.NewValue); }
    private void HopRangeBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)       { if (!double.IsNaN(e.NewValue)) ViewModel.SetHopRange(e.NewValue);        }
    private void RxGainSignalBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)  { if (!double.IsNaN(e.NewValue)) ViewModel.SetRxGainSignal(e.NewValue);   }
    private void RxGainJammerBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)  { if (!double.IsNaN(e.NewValue)) ViewModel.SetRxGainJammer(e.NewValue);   }
    private void JsThresholdBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)   { if (!double.IsNaN(e.NewValue)) ViewModel.SetJsThreshold(e.NewValue);    }
}

public sealed partial class LocationPage : Page
{
    public LocationViewModel ViewModel { get; } = new();
    public LocationPage() {
        InitializeComponent();
        BearingErrorBox.Setup(0.01,    45.0);
        AoaRangeBox    .Setup(0.1,  10000.0);
        RmsTimeErrorBox.Setup(0.001, 100000.0);
        BaselineBox    .Setup(0.1,  10000.0);
        SemiMajorBox   .Setup(0.001, 1000.0);
        SemiMinorBox   .Setup(0.001, 1000.0);
    }

    private void BearingErrorBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)  { if (!double.IsNaN(e.NewValue)) ViewModel.SetRmsBearingError(e.NewValue); }
    private void AoaRangeBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)      { if (!double.IsNaN(e.NewValue)) ViewModel.SetAoaRange(e.NewValue);        }
    private void RmsTimeErrorBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)  { if (!double.IsNaN(e.NewValue)) ViewModel.SetRmsTimeError(e.NewValue);    }
    private void BaselineBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)      { if (!double.IsNaN(e.NewValue)) ViewModel.SetBaseline(e.NewValue);        }
    private void SemiMajorBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)     { if (!double.IsNaN(e.NewValue)) ViewModel.SetSemiMajor(e.NewValue);       }
    private void SemiMinorBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)     { if (!double.IsNaN(e.NewValue)) ViewModel.SetSemiMinor(e.NewValue);       }
}

public sealed partial class RadarPage : Page
{
    public RadarViewModel ViewModel { get; } = new();
    public RadarPage() {
        InitializeComponent();
        TxPowerBox    .Setup(-50.0,   200.0);
        GainBox       .Setup(-30.0,    60.0);
        FrequencyBox  .Setup(1.0,  100000.0);
        RcsBox        .Setup(-40.0,    60.0);
        LossBox       .Setup(0.0,      30.0);
        NfBox         .Setup(0.0,      30.0);
        BwBox         .Setup(0.001,  10000.0);
        SnrBox        .Setup(-10.0,    50.0);
        TbProductBox  .Setup(1.0,   1000000.0);
        NumPulsesBox  .Setup(1.0,   100000.0);
    }

    private void TxPowerBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)    { if (!double.IsNaN(e.NewValue)) ViewModel.SetTxPower(e.NewValue);           }
    private void GainBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)       { if (!double.IsNaN(e.NewValue)) ViewModel.SetAntennaGain(e.NewValue);       }
    private void FrequencyBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)  { if (!double.IsNaN(e.NewValue)) ViewModel.SetFrequency(e.NewValue);         }
    private void RcsBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)        { if (!double.IsNaN(e.NewValue)) ViewModel.SetTargetRcs(e.NewValue);         }
    private void LossBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)       { if (!double.IsNaN(e.NewValue)) ViewModel.SetSystemLosses(e.NewValue);      }
    private void NfBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)         { if (!double.IsNaN(e.NewValue)) ViewModel.SetNoiseFigure(e.NewValue);       }
    private void BwBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)         { if (!double.IsNaN(e.NewValue)) ViewModel.SetBandwidth(e.NewValue);         }
    private void SnrBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)        { if (!double.IsNaN(e.NewValue)) ViewModel.SetRequiredSnr(e.NewValue);       }
    private void TbProductBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)  { if (!double.IsNaN(e.NewValue)) ViewModel.SetTimeBandwidthProd(e.NewValue); }
    private void NumPulsesBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)  { if (!double.IsNaN(e.NewValue)) ViewModel.SetNumPulses((int)e.NewValue);    }
}

public sealed partial class ReceiverPage : Page
{
    public ReceiverViewModel ViewModel { get; } = new();
    public ReceiverPage() {
        InitializeComponent();
        BandwidthBox  .Setup(0.001,  10000.0);
        NoiseFigureBox.Setup(0.0,       30.0);
        SnrBox        .Setup(-20.0,     50.0);
        Ip2Box        .Setup(-50.0,    100.0);
        Ip3Box        .Setup(-50.0,    100.0);
        AdcBitsBox    .Setup(1.0,       64.0);
    }

    private void BandwidthBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)   { if (!double.IsNaN(e.NewValue)) ViewModel.SetBandwidth(e.NewValue);     }
    private void NoiseFigureBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e) { if (!double.IsNaN(e.NewValue)) ViewModel.SetNoiseFigure(e.NewValue);   }
    private void SnrBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)         { if (!double.IsNaN(e.NewValue)) ViewModel.SetRequiredSnr(e.NewValue);   }
    private void Ip2Box_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)         { if (!double.IsNaN(e.NewValue)) ViewModel.SetSecondOrderIp(e.NewValue); }
    private void Ip3Box_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)         { if (!double.IsNaN(e.NewValue)) ViewModel.SetThirdOrderIp(e.NewValue);  }
    private void AdcBitsBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)     { if (!double.IsNaN(e.NewValue)) ViewModel.SetAdcBits((int)e.NewValue);  }
}

public sealed partial class HomePage : Page
{
    public HomePage() { InitializeComponent(); }
}

public sealed partial class DigitalPage : Page
{
    public DigitalViewModel ViewModel { get; } = new();
    public DigitalPage() {
        InitializeComponent();
        DataRateBox    .Setup(0.0001, 10000.0);
        BandwidthBox   .Setup(0.001,  10000.0);
        SnrBox         .Setup(-30.0,     60.0);
        ChipRateBox    .Setup(0.0001, 10000.0);
        RequiredEbNoBox.Setup(-10.0,     30.0);
        ImplLossBox    .Setup(0.0,       10.0);
    }

    private void DataRateBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)     { if (!double.IsNaN(e.NewValue)) ViewModel.SetDataRate(e.NewValue);           }
    private void BandwidthBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)    { if (!double.IsNaN(e.NewValue)) ViewModel.SetBandwidth(e.NewValue);          }
    private void SnrBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)          { if (!double.IsNaN(e.NewValue)) ViewModel.SetSnr(e.NewValue);                }
    private void ChipRateBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)     { if (!double.IsNaN(e.NewValue)) ViewModel.SetChipRate(e.NewValue);           }
    private void RequiredEbNoBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e) { if (!double.IsNaN(e.NewValue)) ViewModel.SetRequiredEbNo(e.NewValue);       }
    private void ImplLossBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)     { if (!double.IsNaN(e.NewValue)) ViewModel.SetImplementationLoss(e.NewValue); }
}

public sealed partial class ReferencePage : Page
{
    public ReferencePage() { InitializeComponent(); }

    private static void CopyText(string value) {
        var pkg = new DataPackage();
        pkg.SetText(value);
        Clipboard.SetContent(pkg);
    }

    // Antenna Gain
    private void CopyAntGain0_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("0.0"); }
    private void CopyAntGain1_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("0"); }
    private void CopyAntGain2_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("2"); }
    private void CopyAntGain3_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("2.15"); }
    private void CopyAntGain4_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("7"); }
    private void CopyAntGain5_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("8.5"); }
    private void CopyAntGain6_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("10.5"); }
    private void CopyAntGain7_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("14"); }

    // Antenna Sidelobe Levels
    private void CopySll0_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("-13"); }
    private void CopySll1_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("-25"); }
    private void CopySll2_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("-35"); }

    // Thermal Noise Floor
    private void CopyNoise0_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("-174.0"); }
    private void CopyNoise1_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("-144.0"); }
    private void CopyNoise2_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("-114.0"); }
    private void CopyNoise3_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("-104.0"); }
    private void CopyNoise4_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("-94.0"); }
    private void CopyNoise5_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("-84.0"); }

    // Radar Cross Section
    private void CopyRcs0_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("15"); }
    private void CopyRcs1_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("7"); }
    private void CopyRcs2_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("0"); }
    private void CopyRcs3_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("-15"); }
    private void CopyRcs4_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("-7"); }
    private void CopyRcs5_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("-15"); }
    private void CopyRcs6_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("25"); }
    private void CopyRcs7_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("45"); }

    // Eb/N₀ Requirements
    private void CopyEbNo0_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("6.8"); }
    private void CopyEbNo1_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("9.6"); }
    private void CopyEbNo2_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("10.5"); }
    private void CopyEbNo3_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("13.5"); }
    private void CopyEbNo4_Click(object s, Microsoft.UI.Xaml.RoutedEventArgs e) { CopyText("17.0"); }
}
