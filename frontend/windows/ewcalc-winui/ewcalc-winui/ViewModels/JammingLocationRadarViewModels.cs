// ViewModels/JammingLocationRadarViewModels.cs
using EwPresenterNet;
using Microsoft.UI.Dispatching;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace EwCalc.ViewModels;

public sealed class JammingViewModel : INotifyPropertyChanged
{
    private readonly JammingAdapter _adapter = new();
    private readonly DispatcherQueue _dispatcher;

    private string _jsRatio = string.Empty,    _signalAtRx = string.Empty,
                   _jammerAtRx = string.Empty, _optimumBw = string.Empty,
                   _dutyCycle = string.Empty,  _burnthrough = string.Empty;
    private bool _isValid = false;

    public string JsRatio     { get => _jsRatio;     private set => Set(ref _jsRatio,     value); }
    public string SignalAtRx  { get => _signalAtRx;  private set => Set(ref _signalAtRx,  value); }
    public string JammerAtRx  { get => _jammerAtRx;  private set => Set(ref _jammerAtRx,  value); }
    public string OptimumBw   { get => _optimumBw;   private set => Set(ref _optimumBw,   value); }
    public string DutyCycle   { get => _dutyCycle;   private set => Set(ref _dutyCycle,   value); }
    public string Burnthrough { get => _burnthrough; private set => Set(ref _burnthrough, value); }
    public bool   IsValid     { get => _isValid;     private set => Set(ref _isValid,     value); }

    public double DefaultSignalErp       => _adapter.DefaultSignalErp;
    public double DefaultJammerErp       => _adapter.DefaultJammerErp;
    public double DefaultSignalToRxDist  => _adapter.DefaultSignalToRxDist;
    public double DefaultJammerToRxDist  => _adapter.DefaultJammerToRxDist;
    public double DefaultSignalTxHeight  => _adapter.DefaultSignalTxHeight;
    public double DefaultJammerHeight    => _adapter.DefaultJammerHeight;
    public double DefaultRxHeight        => _adapter.DefaultRxHeight;
    public double DefaultFrequency       => _adapter.DefaultFrequency;
    public double DefaultSignalBandwidth => _adapter.DefaultSignalBandwidth;
    public double DefaultHopRange        => _adapter.DefaultHopRange;
    public double DefaultJsThreshold     => _adapter.DefaultJsThreshold;
    // Presenter default is 0.0 for both; no adapter property getter needed.
    public double DefaultRxGainSignal    => 0.0;
    public double DefaultRxGainJammer    => 0.0;

    public JammingViewModel()
    {
        _dispatcher = DispatcherQueue.GetForCurrentThread();
        _adapter.Changed += o => _dispatcher.TryEnqueue(() => ApplyOutput(o));
        ApplyOutput(_adapter.CurrentOutput);
    }

    public void SetSignalErp      (double v) => _adapter.SetSignalErp(v);
    public void SetJammerErp      (double v) => _adapter.SetJammerErp(v);
    public void SetSignalToRxDist (double v) => _adapter.SetSignalToRxDist(v);
    public void SetJammerToRxDist (double v) => _adapter.SetJammerToRxDist(v);
    public void SetSignalTxHeight (double v) => _adapter.SetSignalTxHeight(v);
    public void SetJammerHeight   (double v) => _adapter.SetJammerHeight(v);
    public void SetRxHeight       (double v) => _adapter.SetRxHeight(v);
    public void SetFrequency      (double v) => _adapter.SetFrequency(v);
    public void SetRxGainSignal   (double v) => _adapter.SetRxGainSignal(v);
    public void SetRxGainJammer   (double v) => _adapter.SetRxGainJammer(v);
    public void SetSignalBandwidth(double v) => _adapter.SetSignalBandwidth(v);
    public void SetHopRange       (double v) => _adapter.SetHopRange(v);
    public void SetJsThreshold    (double v) => _adapter.SetJsThreshold(v);

    private void ApplyOutput(JammingOutput o)
    {
        JsRatio = o.JsRatioStr; SignalAtRx = o.SignalAtRxStr; JammerAtRx = o.JammerAtRxStr;
        OptimumBw = o.OptimumBwStr; DutyCycle = o.DutyCycleStr;
        Burnthrough = o.BurnthroughRangeStr; IsValid = o.Valid;
    }

    public event PropertyChangedEventHandler? PropertyChanged;
    private void Set<T>(ref T f, T v, [CallerMemberName] string? n = null)
    { if (Equals(f, v)) return; f = v; PropertyChanged?.Invoke(this, new(n)); }
}

public sealed class LocationViewModel : INotifyPropertyChanged
{
    private readonly LocationAdapter _adapter = new();
    private readonly DispatcherQueue _dispatcher;

    private string _cepAoa = string.Empty, _cepEep = string.Empty, _cepTdoa = string.Empty;
    private bool   _isValid = false;

    public string CepAoa  { get => _cepAoa;  private set => Set(ref _cepAoa,  value); }
    public string CepEep  { get => _cepEep;  private set => Set(ref _cepEep,  value); }
    public string CepTdoa { get => _cepTdoa; private set => Set(ref _cepTdoa, value); }
    public bool   IsValid { get => _isValid; private set => Set(ref _isValid, value); }

    public double DefaultRmsBearingError => _adapter.DefaultRmsBearingError;
    public double DefaultAoaRange        => _adapter.DefaultAoaRange;
    public double DefaultSemiMajor       => _adapter.DefaultSemiMajor;
    public double DefaultSemiMinor       => _adapter.DefaultSemiMinor;
    public double DefaultRmsTimeError    => _adapter.DefaultRmsTimeError;
    public double DefaultBaseline        => _adapter.DefaultBaseline;

    public LocationViewModel()
    {
        _dispatcher = DispatcherQueue.GetForCurrentThread();
        _adapter.Changed += o => _dispatcher.TryEnqueue(() => ApplyOutput(o));
        ApplyOutput(_adapter.CurrentOutput);
    }

    public void SetRmsBearingError(double v) => _adapter.SetRmsBearingError(v);
    public void SetAoaRange       (double v) => _adapter.SetAoaRange(v);
    public void SetSemiMajor      (double v) => _adapter.SetSemiMajor(v);
    public void SetSemiMinor      (double v) => _adapter.SetSemiMinor(v);
    public void SetRmsTimeError   (double v) => _adapter.SetRmsTimeError(v);
    public void SetBaseline       (double v) => _adapter.SetBaseline(v);

    private void ApplyOutput(LocationOutput o)
    { CepAoa = o.CepAoaStr; CepEep = o.CepEepStr; CepTdoa = o.CepTdoaStr; IsValid = o.Valid; }

    public event PropertyChangedEventHandler? PropertyChanged;
    private void Set<T>(ref T f, T v, [CallerMemberName] string? n = null)
    { if (Equals(f, v)) return; f = v; PropertyChanged?.Invoke(this, new(n)); }
}

public sealed class RadarViewModel : INotifyPropertyChanged
{
    private readonly RadarAdapter _adapter = new();
    private readonly DispatcherQueue _dispatcher;

    private string _maxRange = string.Empty, _twoWayLoss = string.Empty,
                   _pcGain = string.Empty,   _cohGain = string.Empty,
                   _lpiAdvantage = string.Empty;
    private bool _isValid = false;

    public string MaxRange      { get => _maxRange;      private set => Set(ref _maxRange,      value); }
    public string TwoWayLoss    { get => _twoWayLoss;    private set => Set(ref _twoWayLoss,    value); }
    public string PcGain        { get => _pcGain;        private set => Set(ref _pcGain,        value); }
    public string CohGain       { get => _cohGain;       private set => Set(ref _cohGain,       value); }
    public string LpiAdvantage  { get => _lpiAdvantage;  private set => Set(ref _lpiAdvantage,  value); }
    public bool   IsValid       { get => _isValid;       private set => Set(ref _isValid,       value); }

    public double DefaultTxPower           => _adapter.DefaultTxPower;
    public double DefaultAntennaGain       => _adapter.DefaultAntennaGain;
    public double DefaultTargetRcs         => _adapter.DefaultTargetRcs;
    public double DefaultFrequency         => _adapter.DefaultFrequency;
    public double DefaultSystemLosses      => _adapter.DefaultSystemLosses;
    public double DefaultNoiseFigure       => _adapter.DefaultNoiseFigure;
    public double DefaultBandwidth         => _adapter.DefaultBandwidth;
    public double DefaultRequiredSnr       => _adapter.DefaultRequiredSnr;
    public double DefaultTimeBandwidthProd => _adapter.DefaultTimeBandwidthProd;
    public int    DefaultNumPulses         => _adapter.DefaultNumPulses;

    public RadarViewModel()
    {
        _dispatcher = DispatcherQueue.GetForCurrentThread();
        _adapter.Changed += o => _dispatcher.TryEnqueue(() => ApplyOutput(o));
        ApplyOutput(_adapter.CurrentOutput);
    }

    public void SetTxPower          (double v) => _adapter.SetTxPower(v);
    public void SetAntennaGain      (double v) => _adapter.SetAntennaGain(v);
    public void SetTargetRcs        (double v) => _adapter.SetTargetRcs(v);
    public void SetFrequency        (double v) => _adapter.SetFrequency(v);
    public void SetSystemLosses     (double v) => _adapter.SetSystemLosses(v);
    public void SetNoiseFigure      (double v) => _adapter.SetNoiseFigure(v);
    public void SetBandwidth        (double v) => _adapter.SetBandwidth(v);
    public void SetRequiredSnr      (double v) => _adapter.SetRequiredSnr(v);
    public void SetTimeBandwidthProd(double v) => _adapter.SetTimeBandwidthProd(v);
    public void SetNumPulses        (int v)    => _adapter.SetNumPulses(v);

    private void ApplyOutput(RadarOutput o)
    {
        MaxRange = o.MaxRangeStr; TwoWayLoss = o.TwoWayLossStr;
        PcGain = o.PulseCompressionGainStr; CohGain = o.CoherentIntegrationGainStr;
        LpiAdvantage = o.LpiAdvantageStr; IsValid = o.Valid;
    }

    public event PropertyChangedEventHandler? PropertyChanged;
    private void Set<T>(ref T f, T v, [CallerMemberName] string? n = null)
    { if (Equals(f, v)) return; f = v; PropertyChanged?.Invoke(this, new(n)); }
}
