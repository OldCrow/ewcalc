// Views/PageCodeBehinds.cs
// Code-behind for Link, Jamming, Location, Radar, Receiver, and Home pages.
using EwCalc.ViewModels;
using Microsoft.UI.Xaml.Controls;

namespace EwCalc.Views;

public sealed partial class LinkPage : Page
{
    public LinkViewModel ViewModel { get; } = new();
    public LinkPage() { InitializeComponent(); }

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
    public JammingPage() { InitializeComponent(); }

    private void SignalErpBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)     { if (!double.IsNaN(e.NewValue)) ViewModel.SetSignalErp(e.NewValue);       }
    private void JammerErpBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)     { if (!double.IsNaN(e.NewValue)) ViewModel.SetJammerErp(e.NewValue);       }
    private void SignalDistBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)    { if (!double.IsNaN(e.NewValue)) ViewModel.SetSignalToRxDist(e.NewValue);  }
    private void JammerDistBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)    { if (!double.IsNaN(e.NewValue)) ViewModel.SetJammerToRxDist(e.NewValue);  }
    private void SignalHeightBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)  { if (!double.IsNaN(e.NewValue)) ViewModel.SetSignalTxHeight(e.NewValue);  }
    private void JammerHeightBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)  { if (!double.IsNaN(e.NewValue)) ViewModel.SetJammerHeight(e.NewValue);    }
    private void RxHeightBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)      { if (!double.IsNaN(e.NewValue)) ViewModel.SetRxHeight(e.NewValue);        }
    private void FrequencyBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)     { if (!double.IsNaN(e.NewValue)) ViewModel.SetFrequency(e.NewValue);       }
    private void SignalBwBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)      { if (!double.IsNaN(e.NewValue)) ViewModel.SetSignalBandwidth(e.NewValue); }
    private void HopRangeBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)      { if (!double.IsNaN(e.NewValue)) ViewModel.SetHopRange(e.NewValue);        }
}

public sealed partial class LocationPage : Page
{
    public LocationViewModel ViewModel { get; } = new();
    public LocationPage() { InitializeComponent(); }

    private void BearingErrorBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)  { if (!double.IsNaN(e.NewValue)) ViewModel.SetRmsBearingError(e.NewValue); }
    private void AoaRangeBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)      { if (!double.IsNaN(e.NewValue)) ViewModel.SetAoaRange(e.NewValue);        }
    private void SemiMajorBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)     { if (!double.IsNaN(e.NewValue)) ViewModel.SetSemiMajor(e.NewValue);       }
    private void SemiMinorBox_ValueChanged(NumberBox s, NumberBoxValueChangedEventArgs e)     { if (!double.IsNaN(e.NewValue)) ViewModel.SetSemiMinor(e.NewValue);       }
}

public sealed partial class RadarPage : Page
{
    public RadarViewModel ViewModel { get; } = new();
    public RadarPage() { InitializeComponent(); }

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
    public ReceiverPage() { InitializeComponent(); }

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
