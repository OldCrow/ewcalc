// Views/PageCodeBehinds.cs
// Code-behind for Link, Jamming, Location, Radar, Detection, Doppler, Receiver, Home, Digital, and Antenna pages.
// (Reference has its own file — Views/ReferencePage.xaml.cs — see #73.)
using EwCalc.Helpers;
using EwCalc.ViewModels;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Navigation;

namespace EwCalc.Views;

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

    // Frame.Navigate creates a fresh page (and ViewModel) each time this page is
    // navigated to, so the outgoing instance's native adapter must be disposed
    // deterministically here rather than relying on its finalizer.
    protected override void OnNavigatedFrom(NavigationEventArgs e)
    {
        ViewModel.Dispose();
        base.OnNavigatedFrom(e);
    }

    private void TxPowerBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)       { if (!double.IsNaN(e.NewValue)) ViewModel.SetTxPower(e.NewValue);       }
    private void TxGainBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)        { if (!double.IsNaN(e.NewValue)) ViewModel.SetTxGain(e.NewValue);        }
    private void RxGainBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)        { if (!double.IsNaN(e.NewValue)) ViewModel.SetRxGain(e.NewValue);        }
    private void DistanceBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)      { if (!double.IsNaN(e.NewValue)) ViewModel.SetDistance(e.NewValue);      }
    private void TxHeightBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)      { if (!double.IsNaN(e.NewValue)) ViewModel.SetTxHeight(e.NewValue);      }
    private void RxHeightBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)      { if (!double.IsNaN(e.NewValue)) ViewModel.SetRxHeight(e.NewValue);      }
    private void FrequencyBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)     { if (!double.IsNaN(e.NewValue)) ViewModel.SetFrequency(e.NewValue);     }
    private void RxSensitivityBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e) { if (!double.IsNaN(e.NewValue)) ViewModel.SetRxSensitivity(e.NewValue); }

    private void CopyResultsButton_Click(object sender, Microsoft.UI.Xaml.RoutedEventArgs e)
        => ClipboardHelper.CopyText(ViewModel.BuildResultsText());
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

    protected override void OnNavigatedFrom(NavigationEventArgs e)
    {
        ViewModel.Dispose();
        base.OnNavigatedFrom(e);
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

    private void CopyResultsButton_Click(object sender, Microsoft.UI.Xaml.RoutedEventArgs e)
        => ClipboardHelper.CopyText(ViewModel.BuildResultsText());
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

    protected override void OnNavigatedFrom(NavigationEventArgs e)
    {
        ViewModel.Dispose();
        base.OnNavigatedFrom(e);
    }

    private void BearingErrorBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)  { if (!double.IsNaN(e.NewValue)) ViewModel.SetRmsBearingError(e.NewValue); }
    private void AoaRangeBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)      { if (!double.IsNaN(e.NewValue)) ViewModel.SetAoaRange(e.NewValue);        }
    private void RmsTimeErrorBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)  { if (!double.IsNaN(e.NewValue)) ViewModel.SetRmsTimeError(e.NewValue);    }
    private void BaselineBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)      { if (!double.IsNaN(e.NewValue)) ViewModel.SetBaseline(e.NewValue);        }
    private void SemiMajorBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)     { if (!double.IsNaN(e.NewValue)) ViewModel.SetSemiMajor(e.NewValue);       }
    private void SemiMinorBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)     { if (!double.IsNaN(e.NewValue)) ViewModel.SetSemiMinor(e.NewValue);       }

    private void CopyResultsButton_Click(object sender, Microsoft.UI.Xaml.RoutedEventArgs e)
        => ClipboardHelper.CopyText(ViewModel.BuildResultsText());
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

    protected override void OnNavigatedFrom(NavigationEventArgs e)
    {
        ViewModel.Dispose();
        base.OnNavigatedFrom(e);
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

    private void CopyResultsButton_Click(object sender, Microsoft.UI.Xaml.RoutedEventArgs e)
        => ClipboardHelper.CopyText(ViewModel.BuildResultsText());
}

public sealed partial class DetectionPage : Page
{
    public DetectionViewModel ViewModel { get; } = new();
    public DetectionPage() {
        InitializeComponent();
        PdBox          .Setup(0.1,       0.99);
        PfaExponentBox .Setup(-9.0,      -3.0);
        NumPulsesBox   .Setup(1.0,      100.0);
        SwerlingCaseBox.Setup(0.0,        4.0);
        BeamwidthBox   .Setup(0.1,       45.0);
        ScanRateBox    .Setup(1.0,      720.0);
        PrfBox         .Setup(10.0, 1000000.0);
        BandwidthBox   .Setup(0.001,  10000.0);
    }

    protected override void OnNavigatedFrom(NavigationEventArgs e)
    {
        ViewModel.Dispose();
        base.OnNavigatedFrom(e);
    }

    private void PdBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)           { if (!double.IsNaN(e.NewValue)) ViewModel.SetPd(e.NewValue);                }
    private void PfaExponentBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)  { if (!double.IsNaN(e.NewValue)) ViewModel.SetPfaExponent(e.NewValue);       }
    private void NumPulsesBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)    { if (!double.IsNaN(e.NewValue)) ViewModel.SetNumPulses((int)e.NewValue);    }
    private void SwerlingCaseBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e) { if (!double.IsNaN(e.NewValue)) ViewModel.SetSwerlingCase((int)e.NewValue); }
    private void BeamwidthBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)    { if (!double.IsNaN(e.NewValue)) ViewModel.SetBeamwidth(e.NewValue);         }
    private void ScanRateBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)     { if (!double.IsNaN(e.NewValue)) ViewModel.SetScanRate(e.NewValue);          }
    private void PrfBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)          { if (!double.IsNaN(e.NewValue)) ViewModel.SetPrf(e.NewValue);               }
    private void BandwidthBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)    { if (!double.IsNaN(e.NewValue)) ViewModel.SetBandwidth(e.NewValue);         }

    private void CopyResultsButton_Click(object sender, Microsoft.UI.Xaml.RoutedEventArgs e)
        => ClipboardHelper.CopyText(ViewModel.BuildResultsText());
}

public sealed partial class DopplerPage : Page
{
    public DopplerViewModel ViewModel { get; } = new();
    public DopplerPage() {
        InitializeComponent();
        FrequencyBox   .Setup(1.0,   100000.0);
        RadialSpeedBox .Setup(-3000.0, 3000.0);
        PrfBox         .Setup(10.0, 1000000.0);
        BandwidthBox   .Setup(0.001,  10000.0);
        TargetRangeBox .Setup(0.1,     5000.0);
        BeamwidthAzBox .Setup(0.1,       45.0);
        BeamwidthElBox .Setup(0.1,       45.0);
    }

    protected override void OnNavigatedFrom(NavigationEventArgs e)
    {
        ViewModel.Dispose();
        base.OnNavigatedFrom(e);
    }

    private void FrequencyBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)   { if (!double.IsNaN(e.NewValue)) ViewModel.SetFrequency(e.NewValue);   }
    private void RadialSpeedBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e) { if (!double.IsNaN(e.NewValue)) ViewModel.SetRadialSpeed(e.NewValue); }
    private void PrfBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)         { if (!double.IsNaN(e.NewValue)) ViewModel.SetPrf(e.NewValue);         }
    private void BandwidthBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)   { if (!double.IsNaN(e.NewValue)) ViewModel.SetBandwidth(e.NewValue);   }
    private void TargetRangeBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e) { if (!double.IsNaN(e.NewValue)) ViewModel.SetTargetRange(e.NewValue); }
    private void BeamwidthAzBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e) { if (!double.IsNaN(e.NewValue)) ViewModel.SetBeamwidthAz(e.NewValue); }
    private void BeamwidthElBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e) { if (!double.IsNaN(e.NewValue)) ViewModel.SetBeamwidthEl(e.NewValue); }

    private void CopyResultsButton_Click(object sender, Microsoft.UI.Xaml.RoutedEventArgs e)
        => ClipboardHelper.CopyText(ViewModel.BuildResultsText());
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

    protected override void OnNavigatedFrom(NavigationEventArgs e)
    {
        ViewModel.Dispose();
        base.OnNavigatedFrom(e);
    }

    private void BandwidthBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)   { if (!double.IsNaN(e.NewValue)) ViewModel.SetBandwidth(e.NewValue);     }
    private void NoiseFigureBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e) { if (!double.IsNaN(e.NewValue)) ViewModel.SetNoiseFigure(e.NewValue);   }
    private void SnrBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)         { if (!double.IsNaN(e.NewValue)) ViewModel.SetRequiredSnr(e.NewValue);   }
    private void Ip2Box_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)         { if (!double.IsNaN(e.NewValue)) ViewModel.SetSecondOrderIp(e.NewValue); }
    private void Ip3Box_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)         { if (!double.IsNaN(e.NewValue)) ViewModel.SetThirdOrderIp(e.NewValue);  }
    private void AdcBitsBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)     { if (!double.IsNaN(e.NewValue)) ViewModel.SetAdcBits((int)e.NewValue);  }

    private void CopyResultsButton_Click(object sender, Microsoft.UI.Xaml.RoutedEventArgs e)
        => ClipboardHelper.CopyText(ViewModel.BuildResultsText());
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

    protected override void OnNavigatedFrom(NavigationEventArgs e)
    {
        ViewModel.Dispose();
        base.OnNavigatedFrom(e);
    }

    private void DataRateBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)     { if (!double.IsNaN(e.NewValue)) ViewModel.SetDataRate(e.NewValue);           }
    private void BandwidthBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)    { if (!double.IsNaN(e.NewValue)) ViewModel.SetBandwidth(e.NewValue);          }
    private void SnrBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)          { if (!double.IsNaN(e.NewValue)) ViewModel.SetSnr(e.NewValue);                }
    private void ChipRateBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)     { if (!double.IsNaN(e.NewValue)) ViewModel.SetChipRate(e.NewValue);           }
    private void RequiredEbNoBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e) { if (!double.IsNaN(e.NewValue)) ViewModel.SetRequiredEbNo(e.NewValue);       }
    private void ImplLossBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)     { if (!double.IsNaN(e.NewValue)) ViewModel.SetImplementationLoss(e.NewValue); }

    private void CopyResultsButton_Click(object sender, Microsoft.UI.Xaml.RoutedEventArgs e)
        => ClipboardHelper.CopyText(ViewModel.BuildResultsText());
}

public sealed partial class AntennaPage : Page
{
    public AntennaViewModel ViewModel { get; } = new();
    public AntennaPage() {
        InitializeComponent();
        // Lower bound matches AntennaPresenter::set_gain's -6.35 dBi validation
        // floor (below that, beamwidth_from_gain() exceeds its valid domain and
        // returns beamwidth > 360° — see audit #1 / libew antenna.h). NumberBox's
        // XAML Minimum is set here in code, not XAML, because WinUI parses
        // NumberBox.Minimum as float32 (see PageHelper.cs), which would round
        // -6.35 imprecisely; Setup() applies the exact double-precision bound
        // and overrides whatever the XAML declares.
        GainBox       .Setup(-6.35,   60.0);
        AzBeamwidthBox.Setup(0.1,    360.0);
        ElBeamwidthBox.Setup(0.1,    360.0);
        TxPowerBox    .Setup(-30.0,  100.0);
        FrequencyBox  .Setup(0.1, 100000.0);
    }

    protected override void OnNavigatedFrom(NavigationEventArgs e)
    {
        ViewModel.Dispose();
        base.OnNavigatedFrom(e);
    }

    private void GainBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)        { if (!double.IsNaN(e.NewValue)) ViewModel.SetGain(e.NewValue);        }
    private void AzBeamwidthBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e) { if (!double.IsNaN(e.NewValue)) ViewModel.SetAzBeamwidth(e.NewValue); }
    private void ElBeamwidthBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e) { if (!double.IsNaN(e.NewValue)) ViewModel.SetElBeamwidth(e.NewValue); }
    private void TxPowerBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)     { if (!double.IsNaN(e.NewValue)) ViewModel.SetTxPower(e.NewValue);     }
    private void FrequencyBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)   { if (!double.IsNaN(e.NewValue)) ViewModel.SetFrequency(e.NewValue);   }

    private void CopyResultsButton_Click(object sender, Microsoft.UI.Xaml.RoutedEventArgs e)
        => ClipboardHelper.CopyText(ViewModel.BuildResultsText());
}

// ReferencePage moved to its own code-behind file (#73): Views/ReferencePage.xaml.cs.
// It renders ewpresenter::refdata content via the RefData interop wrapper instead of
// the ~240 lines of hand-unrolled rows and per-row CopyXxxN_Click handlers this file
// used to carry.
