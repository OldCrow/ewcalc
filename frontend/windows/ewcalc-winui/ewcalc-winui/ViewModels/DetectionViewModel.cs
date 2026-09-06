// ViewModels/DetectionViewModel.cs
using EwCalc.Helpers;
using EwPresenterNet;
using Microsoft.UI.Dispatching;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace EwCalc.ViewModels;

public sealed class DetectionViewModel : INotifyPropertyChanged, IDisposable
{
    private readonly DetectionAdapter _adapter = new();
    private readonly DispatcherQueue _dispatcher;

    private string _requiredSnr = string.Empty, _requiredSnrAlbersheim = string.Empty,
                   _fluctuationLoss = string.Empty, _dwellTime = string.Empty,
                   _hitsPerScan = string.Empty,     _far = string.Empty;
    private bool _isValid;

    public string RequiredSnr           { get => _requiredSnr;           private set => Set(ref _requiredSnr,           value); }
    public string RequiredSnrAlbersheim { get => _requiredSnrAlbersheim; private set => Set(ref _requiredSnrAlbersheim, value); }
    public string FluctuationLoss       { get => _fluctuationLoss;       private set => Set(ref _fluctuationLoss,       value); }
    public string DwellTime             { get => _dwellTime;             private set => Set(ref _dwellTime,             value); }
    public string HitsPerScan           { get => _hitsPerScan;           private set => Set(ref _hitsPerScan,           value); }
    public string Far                   { get => _far;                   private set => Set(ref _far,                   value); }
    public bool   IsValid               { get => _isValid;               private set => Set(ref _isValid,               value); }

    public double DefaultPd           => _adapter.DefaultPd;
    public double DefaultPfaExponent  => _adapter.DefaultPfaExponent;
    public int    DefaultNumPulses    => _adapter.DefaultNumPulses;
    public int    DefaultSwerlingCase => _adapter.DefaultSwerlingCase;
    public double DefaultBeamwidth    => _adapter.DefaultBeamwidth;
    public double DefaultScanRate     => _adapter.DefaultScanRate;
    public double DefaultPrf          => _adapter.DefaultPrf;
    public double DefaultBandwidth    => _adapter.DefaultBandwidth;

    private FieldValidationError _pdError           = FieldValidationError.None;
    private FieldValidationError _pfaExponentError  = FieldValidationError.None;
    private FieldValidationError _numPulsesError    = FieldValidationError.None;
    private FieldValidationError _swerlingCaseError = FieldValidationError.None;
    private FieldValidationError _beamwidthError    = FieldValidationError.None;
    private FieldValidationError _scanRateError     = FieldValidationError.None;
    private FieldValidationError _prfError          = FieldValidationError.None;
    private FieldValidationError _bandwidthError    = FieldValidationError.None;

    public FieldValidationError PdError           { get => _pdError;           private set => Set(ref _pdError,           value); }
    public FieldValidationError PfaExponentError  { get => _pfaExponentError;  private set => Set(ref _pfaExponentError,  value); }
    public FieldValidationError NumPulsesError    { get => _numPulsesError;    private set => Set(ref _numPulsesError,    value); }
    public FieldValidationError SwerlingCaseError { get => _swerlingCaseError; private set => Set(ref _swerlingCaseError, value); }
    public FieldValidationError BeamwidthError    { get => _beamwidthError;    private set => Set(ref _beamwidthError,    value); }
    public FieldValidationError ScanRateError     { get => _scanRateError;     private set => Set(ref _scanRateError,     value); }
    public FieldValidationError PrfError          { get => _prfError;          private set => Set(ref _prfError,          value); }
    public FieldValidationError BandwidthError    { get => _bandwidthError;    private set => Set(ref _bandwidthError,    value); }

    public DetectionViewModel()
    {
        _dispatcher = DispatcherQueue.GetForCurrentThread();

        var saved = SettingsService.Current.Detection;
        if (saved.Pd           is double pd)  _adapter.SetPd(pd);
        if (saved.PfaExponent  is double pfa) _adapter.SetPfaExponent(pfa);
        if (saved.NumPulses    is int np)     _adapter.SetNumPulses(np);
        if (saved.SwerlingCase is int sw)     _adapter.SetSwerlingCase(sw);
        if (saved.Beamwidth    is double bw)  _adapter.SetBeamwidth(bw);
        if (saved.ScanRate     is double sr)  _adapter.SetScanRate(sr);
        if (saved.Prf          is double prf) _adapter.SetPrf(prf);
        if (saved.Bandwidth    is double nbw) _adapter.SetBandwidth(nbw);

        _adapter.Changed += o => _dispatcher.TryEnqueue(() => ApplyOutput(o));
        ApplyOutput(_adapter.CurrentOutput);
    }

    public void SetPd(double v)
    { _adapter.SetPd(v); SettingsService.Current.Detection.Pd = v; SettingsService.Save(); }
    public void SetPfaExponent(double v)
    { _adapter.SetPfaExponent(v); SettingsService.Current.Detection.PfaExponent = v; SettingsService.Save(); }
    public void SetNumPulses(int v)
    { _adapter.SetNumPulses(v); SettingsService.Current.Detection.NumPulses = v; SettingsService.Save(); }
    public void SetSwerlingCase(int v)
    { _adapter.SetSwerlingCase(v); SettingsService.Current.Detection.SwerlingCase = v; SettingsService.Save(); }
    public void SetBeamwidth(double v)
    { _adapter.SetBeamwidth(v); SettingsService.Current.Detection.Beamwidth = v; SettingsService.Save(); }
    public void SetScanRate(double v)
    { _adapter.SetScanRate(v); SettingsService.Current.Detection.ScanRate = v; SettingsService.Save(); }
    public void SetPrf(double v)
    { _adapter.SetPrf(v); SettingsService.Current.Detection.Prf = v; SettingsService.Save(); }
    public void SetBandwidth(double v)
    { _adapter.SetBandwidth(v); SettingsService.Current.Detection.Bandwidth = v; SettingsService.Save(); }

    public string BuildResultsText() => string.Join("\n", new[]
    {
        $"Required SNR (Shnidman): {RequiredSnr}",
        $"Required SNR (Albersheim, Sw0): {RequiredSnrAlbersheim}",
        $"Fluctuation loss: {FluctuationLoss}",
        $"Dwell time: {DwellTime}",
        $"Hits per scan: {HitsPerScan}",
        $"False-alarm rate: {Far}",
    });

    private void ApplyOutput(DetectionOutput o)
    {
        RequiredSnr = o.RequiredSnrStr; RequiredSnrAlbersheim = o.RequiredSnrAlbersheimStr;
        FluctuationLoss = o.FluctuationLossStr; DwellTime = o.DwellTimeStr;
        HitsPerScan = o.HitsPerScanStr; Far = o.FarStr; IsValid = o.Valid;
        PdError = _adapter.PdError; PfaExponentError = _adapter.PfaExponentError;
        NumPulsesError = _adapter.NumPulsesError; SwerlingCaseError = _adapter.SwerlingCaseError;
        BeamwidthError = _adapter.BeamwidthError; ScanRateError = _adapter.ScanRateError;
        PrfError = _adapter.PrfError; BandwidthError = _adapter.BandwidthError;
    }

    public event PropertyChangedEventHandler? PropertyChanged;

    private void Set<T>(ref T field, T value, [CallerMemberName] string? name = null)
    {
        if (Equals(field, value)) return;
        field = value;
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
    }

    public void Dispose() => _adapter.Dispose();
}
