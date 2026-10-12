// ViewModels/RadarJammingViewModel.cs
using EwCalc.Helpers;
using EwPresenterNet;
using Microsoft.UI.Dispatching;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace EwCalc.ViewModels;

public sealed class RadarJammingViewModel : INotifyPropertyChanged, IDisposable
{
    // Mirrors ewpresenter::RadarJammingPresenter::kStandOff.
    private const int StandOffGeometry = 1;

    private readonly RadarJammingAdapter _adapter = new();
    private readonly DispatcherQueue _dispatcher;

    private string _jsRatio = string.Empty,  _jsMargin = string.Empty,
                   _burnthroughRange = string.Empty, _status = string.Empty;
    private bool _isValid, _isStandOff;

    public string JsRatio           { get => _jsRatio;           private set => Set(ref _jsRatio,           value); }
    public string JsMargin          { get => _jsMargin;          private set => Set(ref _jsMargin,          value); }
    public string BurnthroughRange  { get => _burnthroughRange;  private set => Set(ref _burnthroughRange,  value); }
    public string Status            { get => _status;            private set => Set(ref _status,            value); }
    public bool   IsValid           { get => _isValid;           private set => Set(ref _isValid,           value); }

    // Drive the stand-off-only input visibility and the diagram shown in the Geometry expander.
    public bool IsStandOff        { get => _isStandOff;  private set { if (Set(ref _isStandOff, value)) Notify(nameof(IsSelfProtection)); } }
    public bool IsSelfProtection  => !_isStandOff;

    public int    DefaultGeometry     => _adapter.Geometry;
    public double DefaultRadarErp     => _adapter.DefaultRadarErp;
    public double DefaultJammerErp    => _adapter.DefaultJammerErp;
    public double DefaultTargetRcs    => _adapter.DefaultTargetRcs;
    public double DefaultTargetRange  => _adapter.DefaultTargetRange;
    public double DefaultJammerRange  => _adapter.DefaultJammerRange;
    public double DefaultMainbeamGain => _adapter.DefaultMainbeamGain;
    public double DefaultSidelobeGain => _adapter.DefaultSidelobeGain;
    public double DefaultJsThreshold  => _adapter.DefaultJsThreshold;

    private FieldValidationError _geometryError     = FieldValidationError.None;
    private FieldValidationError _radarErpError     = FieldValidationError.None;
    private FieldValidationError _jammerErpError    = FieldValidationError.None;
    private FieldValidationError _targetRcsError    = FieldValidationError.None;
    private FieldValidationError _targetRangeError  = FieldValidationError.None;
    private FieldValidationError _jammerRangeError  = FieldValidationError.None;
    private FieldValidationError _mainbeamGainError = FieldValidationError.None;
    private FieldValidationError _sidelobeGainError = FieldValidationError.None;
    private FieldValidationError _jsThresholdError  = FieldValidationError.None;

    public FieldValidationError GeometryError     { get => _geometryError;     private set => Set(ref _geometryError,     value); }
    public FieldValidationError RadarErpError     { get => _radarErpError;     private set => Set(ref _radarErpError,     value); }
    public FieldValidationError JammerErpError    { get => _jammerErpError;    private set => Set(ref _jammerErpError,    value); }
    public FieldValidationError TargetRcsError    { get => _targetRcsError;    private set => Set(ref _targetRcsError,    value); }
    public FieldValidationError TargetRangeError  { get => _targetRangeError;  private set => Set(ref _targetRangeError,  value); }
    public FieldValidationError JammerRangeError  { get => _jammerRangeError;  private set => Set(ref _jammerRangeError,  value); }
    public FieldValidationError MainbeamGainError { get => _mainbeamGainError; private set => Set(ref _mainbeamGainError, value); }
    public FieldValidationError SidelobeGainError { get => _sidelobeGainError; private set => Set(ref _sidelobeGainError, value); }
    public FieldValidationError JsThresholdError  { get => _jsThresholdError;  private set => Set(ref _jsThresholdError,  value); }

    public RadarJammingViewModel()
    {
        _dispatcher = DispatcherQueue.GetForCurrentThread();

        var saved = SettingsService.Current.RadarJamming;
        if (saved.Geometry     is int g)     _adapter.SetGeometry(g);
        if (saved.RadarErp     is double re) _adapter.SetRadarErp(re);
        if (saved.JammerErp    is double je) _adapter.SetJammerErp(je);
        if (saved.TargetRcs    is double rc) _adapter.SetTargetRcs(rc);
        if (saved.TargetRange  is double tr) _adapter.SetTargetRange(tr);
        if (saved.JammerRange  is double jr) _adapter.SetJammerRange(jr);
        if (saved.MainbeamGain is double mg) _adapter.SetMainbeamGain(mg);
        if (saved.SidelobeGain is double sg) _adapter.SetSidelobeGain(sg);
        if (saved.JsThreshold  is double jt) _adapter.SetJsThreshold(jt);

        _adapter.Changed += o => _dispatcher.TryEnqueue(() => ApplyOutput(o));
        ApplyOutput(_adapter.CurrentOutput);
    }

    public void SetGeometry(int v)
    { _adapter.SetGeometry(v); SettingsService.Current.RadarJamming.Geometry = v; SettingsService.Save(); }
    public void SetRadarErp(double v)
    { _adapter.SetRadarErp(v); SettingsService.Current.RadarJamming.RadarErp = v; SettingsService.Save(); }
    public void SetJammerErp(double v)
    { _adapter.SetJammerErp(v); SettingsService.Current.RadarJamming.JammerErp = v; SettingsService.Save(); }
    public void SetTargetRcs(double v)
    { _adapter.SetTargetRcs(v); SettingsService.Current.RadarJamming.TargetRcs = v; SettingsService.Save(); }
    public void SetTargetRange(double v)
    { _adapter.SetTargetRange(v); SettingsService.Current.RadarJamming.TargetRange = v; SettingsService.Save(); }
    public void SetJammerRange(double v)
    { _adapter.SetJammerRange(v); SettingsService.Current.RadarJamming.JammerRange = v; SettingsService.Save(); }
    public void SetMainbeamGain(double v)
    { _adapter.SetMainbeamGain(v); SettingsService.Current.RadarJamming.MainbeamGain = v; SettingsService.Save(); }
    public void SetSidelobeGain(double v)
    { _adapter.SetSidelobeGain(v); SettingsService.Current.RadarJamming.SidelobeGain = v; SettingsService.Save(); }
    public void SetJsThreshold(double v)
    { _adapter.SetJsThreshold(v); SettingsService.Current.RadarJamming.JsThreshold = v; SettingsService.Save(); }

    public string BuildResultsText() => string.Join("\n", new[]
    {
        $"J/S: {JsRatio}",
        $"J/S margin: {JsMargin}",
        $"Burnthrough range: {BurnthroughRange}",
        $"Status: {Status}",
    });

    private void ApplyOutput(RadarJammingOutput o)
    {
        JsRatio = o.JsRatioStr; JsMargin = o.JsMarginStr;
        BurnthroughRange = o.BurnthroughRangeStr; Status = o.StatusStr; IsValid = o.Valid;
        IsStandOff = _adapter.Geometry == StandOffGeometry;
        GeometryError = _adapter.GeometryError; RadarErpError = _adapter.RadarErpError;
        JammerErpError = _adapter.JammerErpError; TargetRcsError = _adapter.TargetRcsError;
        TargetRangeError = _adapter.TargetRangeError; JammerRangeError = _adapter.JammerRangeError;
        MainbeamGainError = _adapter.MainbeamGainError; SidelobeGainError = _adapter.SidelobeGainError;
        JsThresholdError = _adapter.JsThresholdError;
    }

    public event PropertyChangedEventHandler? PropertyChanged;

    private void Notify(string name) => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));

    private bool Set<T>(ref T field, T value, [CallerMemberName] string? name = null)
    {
        if (Equals(field, value)) return false;
        field = value;
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
        return true;
    }

    public void Dispose() => _adapter.Dispose();
}
