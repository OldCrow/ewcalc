// ViewModels/DopplerViewModel.cs
using EwCalc.Helpers;
using EwPresenterNet;
using Microsoft.UI.Dispatching;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace EwCalc.ViewModels;

public sealed class DopplerViewModel : INotifyPropertyChanged, IDisposable
{
    private readonly DopplerAdapter _adapter = new();
    private readonly DispatcherQueue _dispatcher;

    private string _dopplerShift = string.Empty,    _unambiguousRange = string.Empty,
                   _blindSpeed = string.Empty,      _unambiguousVelocity = string.Empty,
                   _rangeResolution = string.Empty, _crossRangeAz = string.Empty,
                   _crossRangeEl = string.Empty;
    private bool _isValid;

    public string DopplerShift        { get => _dopplerShift;        private set => Set(ref _dopplerShift,        value); }
    public string UnambiguousRange    { get => _unambiguousRange;    private set => Set(ref _unambiguousRange,    value); }
    public string BlindSpeed          { get => _blindSpeed;          private set => Set(ref _blindSpeed,          value); }
    public string UnambiguousVelocity { get => _unambiguousVelocity; private set => Set(ref _unambiguousVelocity, value); }
    public string RangeResolution     { get => _rangeResolution;     private set => Set(ref _rangeResolution,     value); }
    public string CrossRangeAz        { get => _crossRangeAz;        private set => Set(ref _crossRangeAz,        value); }
    public string CrossRangeEl        { get => _crossRangeEl;        private set => Set(ref _crossRangeEl,        value); }
    public bool   IsValid             { get => _isValid;             private set => Set(ref _isValid,             value); }

    public double DefaultFrequency   => _adapter.DefaultFrequency;
    public double DefaultRadialSpeed => _adapter.DefaultRadialSpeed;
    public double DefaultPrf         => _adapter.DefaultPrf;
    public double DefaultBandwidth   => _adapter.DefaultBandwidth;
    public double DefaultTargetRange => _adapter.DefaultTargetRange;
    public double DefaultBeamwidthAz => _adapter.DefaultBeamwidthAz;
    public double DefaultBeamwidthEl => _adapter.DefaultBeamwidthEl;

    private FieldValidationError _frequencyError   = FieldValidationError.None;
    private FieldValidationError _radialSpeedError = FieldValidationError.None;
    private FieldValidationError _prfError         = FieldValidationError.None;
    private FieldValidationError _bandwidthError   = FieldValidationError.None;
    private FieldValidationError _targetRangeError = FieldValidationError.None;
    private FieldValidationError _beamwidthAzError = FieldValidationError.None;
    private FieldValidationError _beamwidthElError = FieldValidationError.None;

    public FieldValidationError FrequencyError   { get => _frequencyError;   private set => Set(ref _frequencyError,   value); }
    public FieldValidationError RadialSpeedError { get => _radialSpeedError; private set => Set(ref _radialSpeedError, value); }
    public FieldValidationError PrfError         { get => _prfError;         private set => Set(ref _prfError,         value); }
    public FieldValidationError BandwidthError   { get => _bandwidthError;   private set => Set(ref _bandwidthError,   value); }
    public FieldValidationError TargetRangeError { get => _targetRangeError; private set => Set(ref _targetRangeError, value); }
    public FieldValidationError BeamwidthAzError { get => _beamwidthAzError; private set => Set(ref _beamwidthAzError, value); }
    public FieldValidationError BeamwidthElError { get => _beamwidthElError; private set => Set(ref _beamwidthElError, value); }

    public DopplerViewModel()
    {
        _dispatcher = DispatcherQueue.GetForCurrentThread();

        var saved = SettingsService.Current.Doppler;
        if (saved.Frequency   is double f)   _adapter.SetFrequency(f);
        if (saved.RadialSpeed is double v)   _adapter.SetRadialSpeed(v);
        if (saved.Prf         is double prf) _adapter.SetPrf(prf);
        if (saved.Bandwidth   is double bw)  _adapter.SetBandwidth(bw);
        if (saved.TargetRange is double r)   _adapter.SetTargetRange(r);
        if (saved.BeamwidthAz is double az)  _adapter.SetBeamwidthAz(az);
        if (saved.BeamwidthEl is double el)  _adapter.SetBeamwidthEl(el);

        _adapter.Changed += o => _dispatcher.TryEnqueue(() => ApplyOutput(o));
        ApplyOutput(_adapter.CurrentOutput);
    }

    public void SetFrequency(double v)
    { _adapter.SetFrequency(v); SettingsService.Current.Doppler.Frequency = v; SettingsService.Save(); }
    public void SetRadialSpeed(double v)
    { _adapter.SetRadialSpeed(v); SettingsService.Current.Doppler.RadialSpeed = v; SettingsService.Save(); }
    public void SetPrf(double v)
    { _adapter.SetPrf(v); SettingsService.Current.Doppler.Prf = v; SettingsService.Save(); }
    public void SetBandwidth(double v)
    { _adapter.SetBandwidth(v); SettingsService.Current.Doppler.Bandwidth = v; SettingsService.Save(); }
    public void SetTargetRange(double v)
    { _adapter.SetTargetRange(v); SettingsService.Current.Doppler.TargetRange = v; SettingsService.Save(); }
    public void SetBeamwidthAz(double v)
    { _adapter.SetBeamwidthAz(v); SettingsService.Current.Doppler.BeamwidthAz = v; SettingsService.Save(); }
    public void SetBeamwidthEl(double v)
    { _adapter.SetBeamwidthEl(v); SettingsService.Current.Doppler.BeamwidthEl = v; SettingsService.Save(); }

    public string BuildResultsText() => string.Join("\n", new[]
    {
        $"Doppler shift: {DopplerShift}",
        $"Unambiguous range: {UnambiguousRange}",
        $"First blind speed: {BlindSpeed}",
        $"Unambiguous velocity (±): {UnambiguousVelocity}",
        $"Range resolution: {RangeResolution}",
        $"Cross-range (az): {CrossRangeAz}",
        $"Cross-range (el): {CrossRangeEl}",
    });

    private void ApplyOutput(DopplerOutput o)
    {
        DopplerShift = o.DopplerShiftStr; UnambiguousRange = o.UnambiguousRangeStr;
        BlindSpeed = o.BlindSpeedStr; UnambiguousVelocity = o.UnambiguousVelocityStr;
        RangeResolution = o.RangeResolutionStr; CrossRangeAz = o.CrossRangeAzStr;
        CrossRangeEl = o.CrossRangeElStr; IsValid = o.Valid;
        FrequencyError = _adapter.FrequencyError; RadialSpeedError = _adapter.RadialSpeedError;
        PrfError = _adapter.PrfError; BandwidthError = _adapter.BandwidthError;
        TargetRangeError = _adapter.TargetRangeError; BeamwidthAzError = _adapter.BeamwidthAzError;
        BeamwidthElError = _adapter.BeamwidthElError;
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
