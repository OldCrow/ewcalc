// ViewModels/JammingLocationRadarViewModels.cs
using EwCalc.Helpers;
using EwPresenterNet;
using Microsoft.UI.Dispatching;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace EwCalc.ViewModels;

public sealed class JammingViewModel : INotifyPropertyChanged, IDisposable
{
    private readonly JammingAdapter _adapter = new();
    private readonly DispatcherQueue _dispatcher;

    private string _jsRatio = string.Empty,    _signalAtRx = string.Empty,
                   _jammerAtRx = string.Empty, _optimumBw = string.Empty,
                   _dutyCycle = string.Empty,  _burnthrough = string.Empty;
    private bool _isValid;

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
    // No adapter getter for these two (presenter default is 0.0); tracked here so a
    // restored value can still be reflected in the NumberBox's OneTime-bound default.
    public double DefaultRxGainSignal    => _defaultRxGainSignal;
    public double DefaultRxGainJammer    => _defaultRxGainJammer;

    private readonly double _defaultRxGainSignal;
    private readonly double _defaultRxGainJammer;

    private FieldValidationError _signalErpError       = FieldValidationError.None;
    private FieldValidationError _jammerErpError       = FieldValidationError.None;
    private FieldValidationError _signalDistError      = FieldValidationError.None;
    private FieldValidationError _jammerDistError      = FieldValidationError.None;
    private FieldValidationError _signalHeightError    = FieldValidationError.None;
    private FieldValidationError _jammerHeightError    = FieldValidationError.None;
    private FieldValidationError _rxHeightError        = FieldValidationError.None;
    private FieldValidationError _frequencyError       = FieldValidationError.None;
    private FieldValidationError _rxGainSignalError    = FieldValidationError.None;
    private FieldValidationError _rxGainJammerError    = FieldValidationError.None;
    private FieldValidationError _signalBandwidthError = FieldValidationError.None;
    private FieldValidationError _hopRangeError        = FieldValidationError.None;
    private FieldValidationError _jsThresholdError     = FieldValidationError.None;

    public FieldValidationError SignalErpError       { get => _signalErpError;       private set => Set(ref _signalErpError,       value); }
    public FieldValidationError JammerErpError       { get => _jammerErpError;       private set => Set(ref _jammerErpError,       value); }
    public FieldValidationError SignalDistError      { get => _signalDistError;      private set => Set(ref _signalDistError,      value); }
    public FieldValidationError JammerDistError      { get => _jammerDistError;      private set => Set(ref _jammerDistError,      value); }
    public FieldValidationError SignalHeightError    { get => _signalHeightError;    private set => Set(ref _signalHeightError,    value); }
    public FieldValidationError JammerHeightError    { get => _jammerHeightError;    private set => Set(ref _jammerHeightError,    value); }
    public FieldValidationError RxHeightError        { get => _rxHeightError;        private set => Set(ref _rxHeightError,        value); }
    public FieldValidationError FrequencyError       { get => _frequencyError;       private set => Set(ref _frequencyError,       value); }
    public FieldValidationError RxGainSignalError    { get => _rxGainSignalError;    private set => Set(ref _rxGainSignalError,    value); }
    public FieldValidationError RxGainJammerError    { get => _rxGainJammerError;    private set => Set(ref _rxGainJammerError,    value); }
    public FieldValidationError SignalBandwidthError { get => _signalBandwidthError; private set => Set(ref _signalBandwidthError, value); }
    public FieldValidationError HopRangeError        { get => _hopRangeError;        private set => Set(ref _hopRangeError,        value); }
    public FieldValidationError JsThresholdError     { get => _jsThresholdError;     private set => Set(ref _jsThresholdError,     value); }

    public JammingViewModel()
    {
        _dispatcher = DispatcherQueue.GetForCurrentThread();

        var saved = SettingsService.Current.Jamming;
        _defaultRxGainSignal = saved.RxGainSignal ?? 0.0;
        _defaultRxGainJammer = saved.RxGainJammer ?? 0.0;
        if (saved.SignalErp       is double se)  _adapter.SetSignalErp(se);
        if (saved.JammerErp       is double je)  _adapter.SetJammerErp(je);
        if (saved.SignalToRxDist  is double sd)  _adapter.SetSignalToRxDist(sd);
        if (saved.JammerToRxDist  is double jd)  _adapter.SetJammerToRxDist(jd);
        if (saved.SignalTxHeight  is double sh)  _adapter.SetSignalTxHeight(sh);
        if (saved.JammerHeight    is double jh)  _adapter.SetJammerHeight(jh);
        if (saved.RxHeight        is double rh)  _adapter.SetRxHeight(rh);
        if (saved.Frequency       is double f)   _adapter.SetFrequency(f);
        if (saved.SignalBandwidth is double sbw) _adapter.SetSignalBandwidth(sbw);
        if (saved.HopRange        is double hr)  _adapter.SetHopRange(hr);
        if (saved.JsThreshold     is double jt)  _adapter.SetJsThreshold(jt);
        _adapter.SetRxGainSignal(_defaultRxGainSignal);
        _adapter.SetRxGainJammer(_defaultRxGainJammer);

        _adapter.Changed += o => _dispatcher.TryEnqueue(() => ApplyOutput(o));
        ApplyOutput(_adapter.CurrentOutput);
    }

    public void SetSignalErp(double v)
    { _adapter.SetSignalErp(v); SettingsService.Current.Jamming.SignalErp = v; SettingsService.Save(); }
    public void SetJammerErp(double v)
    { _adapter.SetJammerErp(v); SettingsService.Current.Jamming.JammerErp = v; SettingsService.Save(); }
    public void SetSignalToRxDist(double v)
    { _adapter.SetSignalToRxDist(v); SettingsService.Current.Jamming.SignalToRxDist = v; SettingsService.Save(); }
    public void SetJammerToRxDist(double v)
    { _adapter.SetJammerToRxDist(v); SettingsService.Current.Jamming.JammerToRxDist = v; SettingsService.Save(); }
    public void SetSignalTxHeight(double v)
    { _adapter.SetSignalTxHeight(v); SettingsService.Current.Jamming.SignalTxHeight = v; SettingsService.Save(); }
    public void SetJammerHeight(double v)
    { _adapter.SetJammerHeight(v); SettingsService.Current.Jamming.JammerHeight = v; SettingsService.Save(); }
    public void SetRxHeight(double v)
    { _adapter.SetRxHeight(v); SettingsService.Current.Jamming.RxHeight = v; SettingsService.Save(); }
    public void SetFrequency(double v)
    { _adapter.SetFrequency(v); SettingsService.Current.Jamming.Frequency = v; SettingsService.Save(); }
    public void SetRxGainSignal(double v)
    { _adapter.SetRxGainSignal(v); SettingsService.Current.Jamming.RxGainSignal = v; SettingsService.Save(); }
    public void SetRxGainJammer(double v)
    { _adapter.SetRxGainJammer(v); SettingsService.Current.Jamming.RxGainJammer = v; SettingsService.Save(); }
    public void SetSignalBandwidth(double v)
    { _adapter.SetSignalBandwidth(v); SettingsService.Current.Jamming.SignalBandwidth = v; SettingsService.Save(); }
    public void SetHopRange(double v)
    { _adapter.SetHopRange(v); SettingsService.Current.Jamming.HopRange = v; SettingsService.Save(); }
    public void SetJsThreshold(double v)
    { _adapter.SetJsThreshold(v); SettingsService.Current.Jamming.JsThreshold = v; SettingsService.Save(); }

    public string BuildResultsText() => string.Join("\n", new[]
    {
        $"J/S ratio: {JsRatio}",
        $"Signal at Rx: {SignalAtRx}",
        $"Jammer at Rx: {JammerAtRx}",
        $"Burnthrough: {Burnthrough}",
        $"Optimum BW: {OptimumBw}",
        $"Duty cycle: {DutyCycle}",
    });

    private void ApplyOutput(JammingOutput o)
    {
        JsRatio = o.JsRatioStr; SignalAtRx = o.SignalAtRxStr; JammerAtRx = o.JammerAtRxStr;
        OptimumBw = o.OptimumBwStr; DutyCycle = o.DutyCycleStr;
        Burnthrough = o.BurnthroughRangeStr; IsValid = o.Valid;
        SignalErpError = _adapter.SignalErpError; JammerErpError = _adapter.JammerErpError;
        SignalDistError = _adapter.SignalDistError; JammerDistError = _adapter.JammerDistError;
        SignalHeightError = _adapter.SignalHeightError; JammerHeightError = _adapter.JammerHeightError;
        RxHeightError = _adapter.RxHeightError; FrequencyError = _adapter.FrequencyError;
        RxGainSignalError = _adapter.RxGainSignalError; RxGainJammerError = _adapter.RxGainJammerError;
        SignalBandwidthError = _adapter.SignalBandwidthError; HopRangeError = _adapter.HopRangeError;
        JsThresholdError = _adapter.JsThresholdError;
    }

    public event PropertyChangedEventHandler? PropertyChanged;
    private void Set<T>(ref T f, T v, [CallerMemberName] string? n = null)
    { if (Equals(f, v)) return; f = v; PropertyChanged?.Invoke(this, new(n)); }

    public void Dispose() => _adapter.Dispose();
}

public sealed class LocationViewModel : INotifyPropertyChanged, IDisposable
{
    private readonly LocationAdapter _adapter = new();
    private readonly DispatcherQueue _dispatcher;

    private string _cepAoa = string.Empty, _cepEep = string.Empty, _cepTdoa = string.Empty;
    private bool   _isValid;

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

    private FieldValidationError _rmsBearingError = FieldValidationError.None;
    private FieldValidationError _aoaRangeError    = FieldValidationError.None;
    private FieldValidationError _rmsTimeError     = FieldValidationError.None;
    private FieldValidationError _baselineError    = FieldValidationError.None;
    private FieldValidationError _semiMajorError   = FieldValidationError.None;
    private FieldValidationError _semiMinorError   = FieldValidationError.None;

    public FieldValidationError RmsBearingError { get => _rmsBearingError; private set => Set(ref _rmsBearingError, value); }
    public FieldValidationError AoaRangeError   { get => _aoaRangeError;   private set => Set(ref _aoaRangeError,   value); }
    public FieldValidationError RmsTimeError    { get => _rmsTimeError;    private set => Set(ref _rmsTimeError,    value); }
    public FieldValidationError BaselineError   { get => _baselineError;   private set => Set(ref _baselineError,   value); }
    public FieldValidationError SemiMajorError  { get => _semiMajorError;  private set => Set(ref _semiMajorError,  value); }
    /// <summary>The more specific of the adapter's own bounds check and the semi-minor >
    /// semi-major cross-field check (EepAxisError), since the NumberBox has only one
    /// BorderBrush to bind.</summary>
    public FieldValidationError SemiMinorError  { get => _semiMinorError;  private set => Set(ref _semiMinorError,  value); }

    public LocationViewModel()
    {
        _dispatcher = DispatcherQueue.GetForCurrentThread();

        var saved = SettingsService.Current.Location;
        if (saved.RmsBearingError is double be) _adapter.SetRmsBearingError(be);
        if (saved.AoaRange        is double ar) _adapter.SetAoaRange(ar);
        if (saved.RmsTimeError    is double te) _adapter.SetRmsTimeError(te);
        if (saved.Baseline        is double bl) _adapter.SetBaseline(bl);
        if (saved.SemiMajor       is double sj) _adapter.SetSemiMajor(sj);
        if (saved.SemiMinor       is double sn) _adapter.SetSemiMinor(sn);

        _adapter.Changed += o => _dispatcher.TryEnqueue(() => ApplyOutput(o));
        ApplyOutput(_adapter.CurrentOutput);
    }

    public void SetRmsBearingError(double v)
    { _adapter.SetRmsBearingError(v); SettingsService.Current.Location.RmsBearingError = v; SettingsService.Save(); }
    public void SetAoaRange(double v)
    { _adapter.SetAoaRange(v); SettingsService.Current.Location.AoaRange = v; SettingsService.Save(); }
    public void SetSemiMajor(double v)
    { _adapter.SetSemiMajor(v); SettingsService.Current.Location.SemiMajor = v; SettingsService.Save(); }
    public void SetSemiMinor(double v)
    { _adapter.SetSemiMinor(v); SettingsService.Current.Location.SemiMinor = v; SettingsService.Save(); }
    public void SetRmsTimeError(double v)
    { _adapter.SetRmsTimeError(v); SettingsService.Current.Location.RmsTimeError = v; SettingsService.Save(); }
    public void SetBaseline(double v)
    { _adapter.SetBaseline(v); SettingsService.Current.Location.Baseline = v; SettingsService.Save(); }

    public string BuildResultsText() => string.Join("\n", new[]
    {
        $"CEP (AOA method): {CepAoa}",
        $"CEP (TDOA): {CepTdoa}",
        $"CEP (EEP method): {CepEep}",
    });

    private void ApplyOutput(LocationOutput o)
    {
        CepAoa = o.CepAoaStr; CepEep = o.CepEepStr; CepTdoa = o.CepTdoaStr; IsValid = o.Valid;
        RmsBearingError = _adapter.RmsBearingError; AoaRangeError = _adapter.AoaRangeError;
        RmsTimeError = _adapter.RmsTimeError; BaselineError = _adapter.BaselineError;
        SemiMajorError = _adapter.SemiMajorError;
        var eepAxisError = _adapter.EepAxisError;
        SemiMinorError = eepAxisError != FieldValidationError.None ? eepAxisError : _adapter.SemiMinorError;
    }

    public event PropertyChangedEventHandler? PropertyChanged;
    private void Set<T>(ref T f, T v, [CallerMemberName] string? n = null)
    { if (Equals(f, v)) return; f = v; PropertyChanged?.Invoke(this, new(n)); }

    public void Dispose() => _adapter.Dispose();
}

public sealed class RadarViewModel : INotifyPropertyChanged, IDisposable
{
    private readonly RadarAdapter _adapter = new();
    private readonly DispatcherQueue _dispatcher;

    private string _maxRange = string.Empty, _twoWayLoss = string.Empty,
                   _pcGain = string.Empty,   _cohGain = string.Empty,
                   _lpiAdvantage = string.Empty, _targetRcs = string.Empty;
    private bool _isValid;

    public string MaxRange      { get => _maxRange;      private set => Set(ref _maxRange,      value); }
    public string TwoWayLoss    { get => _twoWayLoss;    private set => Set(ref _twoWayLoss,    value); }
    public string PcGain        { get => _pcGain;        private set => Set(ref _pcGain,        value); }
    public string CohGain       { get => _cohGain;       private set => Set(ref _cohGain,       value); }
    public string LpiAdvantage  { get => _lpiAdvantage;  private set => Set(ref _lpiAdvantage,  value); }
    public string TargetRcs     { get => _targetRcs;     private set => Set(ref _targetRcs,     value); }
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

    private FieldValidationError _txPowerError           = FieldValidationError.None;
    private FieldValidationError _antennaGainError       = FieldValidationError.None;
    private FieldValidationError _targetRcsError         = FieldValidationError.None;
    private FieldValidationError _frequencyError         = FieldValidationError.None;
    private FieldValidationError _systemLossesError      = FieldValidationError.None;
    private FieldValidationError _noiseFigureError       = FieldValidationError.None;
    private FieldValidationError _bandwidthError         = FieldValidationError.None;
    private FieldValidationError _requiredSnrError       = FieldValidationError.None;
    private FieldValidationError _timeBandwidthProdError = FieldValidationError.None;
    private FieldValidationError _numPulsesError         = FieldValidationError.None;

    public FieldValidationError TxPowerError           { get => _txPowerError;           private set => Set(ref _txPowerError,           value); }
    public FieldValidationError AntennaGainError       { get => _antennaGainError;       private set => Set(ref _antennaGainError,       value); }
    public FieldValidationError TargetRcsError         { get => _targetRcsError;         private set => Set(ref _targetRcsError,         value); }
    public FieldValidationError FrequencyError         { get => _frequencyError;         private set => Set(ref _frequencyError,         value); }
    public FieldValidationError SystemLossesError      { get => _systemLossesError;      private set => Set(ref _systemLossesError,      value); }
    public FieldValidationError NoiseFigureError       { get => _noiseFigureError;       private set => Set(ref _noiseFigureError,       value); }
    public FieldValidationError BandwidthError         { get => _bandwidthError;         private set => Set(ref _bandwidthError,         value); }
    public FieldValidationError RequiredSnrError       { get => _requiredSnrError;       private set => Set(ref _requiredSnrError,       value); }
    public FieldValidationError TimeBandwidthProdError { get => _timeBandwidthProdError; private set => Set(ref _timeBandwidthProdError, value); }
    public FieldValidationError NumPulsesError         { get => _numPulsesError;         private set => Set(ref _numPulsesError,         value); }

    public RadarViewModel()
    {
        _dispatcher = DispatcherQueue.GetForCurrentThread();

        var saved = SettingsService.Current.Radar;
        if (saved.TxPower           is double tp)  _adapter.SetTxPower(tp);
        if (saved.AntennaGain       is double ag)  _adapter.SetAntennaGain(ag);
        if (saved.TargetRcs         is double rcs) _adapter.SetTargetRcs(rcs);
        if (saved.Frequency         is double f)   _adapter.SetFrequency(f);
        if (saved.SystemLosses      is double sl)  _adapter.SetSystemLosses(sl);
        if (saved.NoiseFigure       is double nf)  _adapter.SetNoiseFigure(nf);
        if (saved.Bandwidth         is double bw)  _adapter.SetBandwidth(bw);
        if (saved.RequiredSnr       is double sn)  _adapter.SetRequiredSnr(sn);
        if (saved.TimeBandwidthProd is double tb)  _adapter.SetTimeBandwidthProd(tb);
        if (saved.NumPulses         is int np)     _adapter.SetNumPulses(np);

        _adapter.Changed += o => _dispatcher.TryEnqueue(() => ApplyOutput(o));
        ApplyOutput(_adapter.CurrentOutput);
    }

    public void SetTxPower(double v)
    { _adapter.SetTxPower(v); SettingsService.Current.Radar.TxPower = v; SettingsService.Save(); }
    public void SetAntennaGain(double v)
    { _adapter.SetAntennaGain(v); SettingsService.Current.Radar.AntennaGain = v; SettingsService.Save(); }
    public void SetTargetRcs(double v)
    { _adapter.SetTargetRcs(v); SettingsService.Current.Radar.TargetRcs = v; SettingsService.Save(); }
    public void SetFrequency(double v)
    { _adapter.SetFrequency(v); SettingsService.Current.Radar.Frequency = v; SettingsService.Save(); }
    public void SetSystemLosses(double v)
    { _adapter.SetSystemLosses(v); SettingsService.Current.Radar.SystemLosses = v; SettingsService.Save(); }
    public void SetNoiseFigure(double v)
    { _adapter.SetNoiseFigure(v); SettingsService.Current.Radar.NoiseFigure = v; SettingsService.Save(); }
    public void SetBandwidth(double v)
    { _adapter.SetBandwidth(v); SettingsService.Current.Radar.Bandwidth = v; SettingsService.Save(); }
    public void SetRequiredSnr(double v)
    { _adapter.SetRequiredSnr(v); SettingsService.Current.Radar.RequiredSnr = v; SettingsService.Save(); }
    public void SetTimeBandwidthProd(double v)
    { _adapter.SetTimeBandwidthProd(v); SettingsService.Current.Radar.TimeBandwidthProd = v; SettingsService.Save(); }
    public void SetNumPulses(int v)
    { _adapter.SetNumPulses(v); SettingsService.Current.Radar.NumPulses = v; SettingsService.Save(); }

    public string BuildResultsText() => string.Join("\n", new[]
    {
        $"Max range: {MaxRange}",
        $"Two-way loss: {TwoWayLoss}",
        $"Target RCS: {TargetRcs}",
        $"Pulse compression gain: {PcGain}",
        $"Coherent integration gain: {CohGain}",
        $"LPI advantage: {LpiAdvantage}",
    });

    private void ApplyOutput(RadarOutput o)
    {
        MaxRange = o.MaxRangeStr; TwoWayLoss = o.TwoWayLossStr;
        PcGain = o.PulseCompressionGainStr; CohGain = o.CoherentIntegrationGainStr;
        LpiAdvantage = o.LpiAdvantageStr; TargetRcs = o.TargetRcsStr; IsValid = o.Valid;
        TxPowerError = _adapter.TxPowerError; AntennaGainError = _adapter.AntennaGainError;
        TargetRcsError = _adapter.TargetRcsError; FrequencyError = _adapter.FrequencyError;
        SystemLossesError = _adapter.SystemLossesError; NoiseFigureError = _adapter.NoiseFigureError;
        BandwidthError = _adapter.BandwidthError; RequiredSnrError = _adapter.RequiredSnrError;
        TimeBandwidthProdError = _adapter.TimeBandwidthProdError; NumPulsesError = _adapter.NumPulsesError;
    }

    public event PropertyChangedEventHandler? PropertyChanged;
    private void Set<T>(ref T f, T v, [CallerMemberName] string? n = null)
    { if (Equals(f, v)) return; f = v; PropertyChanged?.Invoke(this, new(n)); }

    public void Dispose() => _adapter.Dispose();
}
