// ViewModels/LinkViewModel.cs
using EwCalc.Helpers;
using EwPresenterNet;
using Microsoft.UI.Dispatching;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace EwCalc.ViewModels;

public sealed class LinkViewModel : INotifyPropertyChanged, IDisposable
{
    private readonly LinkAdapter _adapter = new();
    private readonly DispatcherQueue _dispatcher;

    private string _receivedPower = string.Empty, _pathLoss = string.Empty,
                   _linkMargin = string.Empty,    _fresnelZone = string.Empty,
                   _regime = string.Empty,        _effectiveRange = string.Empty,
                   _rangeRegime = string.Empty;
    private bool _isValid;

    public string ReceivedPower  { get => _receivedPower;  private set => Set(ref _receivedPower,  value); }
    public string PathLoss       { get => _pathLoss;       private set => Set(ref _pathLoss,       value); }
    public string LinkMargin     { get => _linkMargin;     private set => Set(ref _linkMargin,     value); }
    public string FresnelZone    { get => _fresnelZone;    private set => Set(ref _fresnelZone,    value); }
    public string Regime         { get => _regime;         private set => Set(ref _regime,         value); }
    public string EffectiveRange { get => _effectiveRange; private set => Set(ref _effectiveRange, value); }
    public string RangeRegime    { get => _rangeRegime;    private set => Set(ref _rangeRegime,    value); }
    public bool   IsValid        { get => _isValid;        private set => Set(ref _isValid,        value); }

    public double DefaultTxPower       => _adapter.DefaultTxPower;
    public double DefaultTxGain        => _adapter.DefaultTxGain;
    public double DefaultRxGain        => _adapter.DefaultRxGain;
    public double DefaultDistance      => _adapter.DefaultDistance;
    public double DefaultTxHeight      => _adapter.DefaultTxHeight;
    public double DefaultRxHeight      => _adapter.DefaultRxHeight;
    public double DefaultFrequency     => _adapter.DefaultFrequency;
    public double DefaultRxSensitivity => _adapter.DefaultRxSensitivity;

    private FieldValidationError _txPowerError       = FieldValidationError.None;
    private FieldValidationError _txGainError        = FieldValidationError.None;
    private FieldValidationError _rxGainError        = FieldValidationError.None;
    private FieldValidationError _distanceError      = FieldValidationError.None;
    private FieldValidationError _txHeightError      = FieldValidationError.None;
    private FieldValidationError _rxHeightError      = FieldValidationError.None;
    private FieldValidationError _frequencyError     = FieldValidationError.None;
    private FieldValidationError _rxSensitivityError = FieldValidationError.None;

    public FieldValidationError TxPowerError       { get => _txPowerError;       private set => Set(ref _txPowerError,       value); }
    public FieldValidationError TxGainError        { get => _txGainError;        private set => Set(ref _txGainError,        value); }
    public FieldValidationError RxGainError        { get => _rxGainError;        private set => Set(ref _rxGainError,        value); }
    public FieldValidationError DistanceError      { get => _distanceError;      private set => Set(ref _distanceError,      value); }
    public FieldValidationError TxHeightError      { get => _txHeightError;      private set => Set(ref _txHeightError,      value); }
    public FieldValidationError RxHeightError      { get => _rxHeightError;      private set => Set(ref _rxHeightError,      value); }
    public FieldValidationError FrequencyError     { get => _frequencyError;     private set => Set(ref _frequencyError,     value); }
    public FieldValidationError RxSensitivityError { get => _rxSensitivityError; private set => Set(ref _rxSensitivityError, value); }

    public LinkViewModel()
    {
        _dispatcher = DispatcherQueue.GetForCurrentThread();

        var saved = SettingsService.Current.Link;
        if (saved.TxPower       is double tp) _adapter.SetTxPower(tp);
        if (saved.TxGain        is double tg) _adapter.SetTxGain(tg);
        if (saved.RxGain        is double rg) _adapter.SetRxGain(rg);
        if (saved.Distance      is double d)  _adapter.SetDistance(d);
        if (saved.TxHeight      is double th) _adapter.SetTxHeight(th);
        if (saved.RxHeight      is double rh) _adapter.SetRxHeight(rh);
        if (saved.Frequency     is double f)  _adapter.SetFrequency(f);
        if (saved.RxSensitivity is double rs) _adapter.SetRxSensitivity(rs);

        _adapter.Changed += o => _dispatcher.TryEnqueue(() => ApplyOutput(o));
        ApplyOutput(_adapter.CurrentOutput);
    }

    public void SetTxPower(double v)
    { _adapter.SetTxPower(v); SettingsService.Current.Link.TxPower = v; SettingsService.Save(); }
    public void SetTxGain(double v)
    { _adapter.SetTxGain(v); SettingsService.Current.Link.TxGain = v; SettingsService.Save(); }
    public void SetRxGain(double v)
    { _adapter.SetRxGain(v); SettingsService.Current.Link.RxGain = v; SettingsService.Save(); }
    public void SetDistance(double v)
    { _adapter.SetDistance(v); SettingsService.Current.Link.Distance = v; SettingsService.Save(); }
    public void SetTxHeight(double v)
    { _adapter.SetTxHeight(v); SettingsService.Current.Link.TxHeight = v; SettingsService.Save(); }
    public void SetRxHeight(double v)
    { _adapter.SetRxHeight(v); SettingsService.Current.Link.RxHeight = v; SettingsService.Save(); }
    public void SetFrequency(double v)
    { _adapter.SetFrequency(v); SettingsService.Current.Link.Frequency = v; SettingsService.Save(); }
    public void SetRxSensitivity(double v)
    { _adapter.SetRxSensitivity(v); SettingsService.Current.Link.RxSensitivity = v; SettingsService.Save(); }

    public string BuildResultsText() => string.Join("\n", new[]
    {
        $"Received power: {ReceivedPower}",
        $"Path loss: {PathLoss}",
        $"Link margin: {LinkMargin}",
        $"Fresnel zone: {FresnelZone}",
        $"Regime: {Regime}",
        $"Effective range: {EffectiveRange}",
        $"Range regime: {RangeRegime}",
    });

    private void ApplyOutput(LinkOutput o)
    {
        ReceivedPower = o.ReceivedPowerStr; PathLoss = o.PathLossStr;
        LinkMargin = o.LinkMarginStr; FresnelZone = o.FresnelZoneStr;
        Regime = o.RegimeStr; EffectiveRange = o.EffectiveRangeStr;
        RangeRegime = o.RangeRegimeStr; IsValid = o.Valid;
        TxPowerError = _adapter.TxPowerError; TxGainError = _adapter.TxGainError;
        RxGainError = _adapter.RxGainError; DistanceError = _adapter.DistanceError;
        TxHeightError = _adapter.TxHeightError; RxHeightError = _adapter.RxHeightError;
        FrequencyError = _adapter.FrequencyError; RxSensitivityError = _adapter.RxSensitivityError;
    }

    public event PropertyChangedEventHandler? PropertyChanged;
    private void Set<T>(ref T f, T v, [CallerMemberName] string? n = null)
    { if (Equals(f, v)) return; f = v; PropertyChanged?.Invoke(this, new(n)); }

    public void Dispose() => _adapter.Dispose();
}
