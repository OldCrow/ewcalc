// ViewModels/AntennaViewModel.cs
using EwCalc.Helpers;
using EwPresenterNet;
using Microsoft.UI.Dispatching;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace EwCalc.ViewModels;

public sealed class AntennaViewModel : INotifyPropertyChanged
{
    private readonly AntennaAdapter _adapter = new();
    private readonly DispatcherQueue _dispatcher;

    private string _erp = string.Empty,               _beamwidthFromGain = string.Empty,
                   _gainFromBeamwidth = string.Empty, _wavelength = string.Empty;
    private bool _isValid = false;

    public string Erp               { get => _erp;               private set => Set(ref _erp,               value); }
    public string BeamwidthFromGain { get => _beamwidthFromGain; private set => Set(ref _beamwidthFromGain, value); }
    public string GainFromBeamwidth { get => _gainFromBeamwidth; private set => Set(ref _gainFromBeamwidth, value); }
    public string Wavelength        { get => _wavelength;        private set => Set(ref _wavelength,        value); }
    public bool   IsValid           { get => _isValid;           private set => Set(ref _isValid,           value); }

    private FieldValidationError _gainError        = FieldValidationError.None;
    private FieldValidationError _azBeamwidthError = FieldValidationError.None;
    private FieldValidationError _elBeamwidthError = FieldValidationError.None;
    private FieldValidationError _txPowerError     = FieldValidationError.None;
    private FieldValidationError _frequencyError   = FieldValidationError.None;

    public FieldValidationError GainError        { get => _gainError;        private set => Set(ref _gainError,        value); }
    public FieldValidationError AzBeamwidthError { get => _azBeamwidthError; private set => Set(ref _azBeamwidthError, value); }
    public FieldValidationError ElBeamwidthError { get => _elBeamwidthError; private set => Set(ref _elBeamwidthError, value); }
    public FieldValidationError TxPowerError     { get => _txPowerError;     private set => Set(ref _txPowerError,     value); }
    public FieldValidationError FrequencyError   { get => _frequencyError;   private set => Set(ref _frequencyError,   value); }

    public double DefaultGain        => _adapter.DefaultGain;
    public double DefaultAzBeamwidth => _adapter.DefaultAzBeamwidth;
    public double DefaultElBeamwidth => _adapter.DefaultElBeamwidth;
    public double DefaultTxPower     => _adapter.DefaultTxPower;
    public double DefaultFrequency   => _adapter.DefaultFrequency;

    public AntennaViewModel()
    {
        _dispatcher = DispatcherQueue.GetForCurrentThread();

        var saved = SettingsService.Current.Antenna;
        if (saved.Gain        is double g)  _adapter.SetGain(g);
        if (saved.AzBeamwidth is double az) _adapter.SetAzBeamwidth(az);
        if (saved.ElBeamwidth is double el) _adapter.SetElBeamwidth(el);
        if (saved.TxPower     is double tp) _adapter.SetTxPower(tp);
        if (saved.Frequency   is double f)  _adapter.SetFrequency(f);

        _adapter.Changed += o => _dispatcher.TryEnqueue(() => ApplyOutput(o));
        ApplyOutput(_adapter.CurrentOutput);
    }

    public void SetGain(double v)
    { _adapter.SetGain(v); SettingsService.Current.Antenna.Gain = v; SettingsService.Save(); }
    public void SetAzBeamwidth(double v)
    { _adapter.SetAzBeamwidth(v); SettingsService.Current.Antenna.AzBeamwidth = v; SettingsService.Save(); }
    public void SetElBeamwidth(double v)
    { _adapter.SetElBeamwidth(v); SettingsService.Current.Antenna.ElBeamwidth = v; SettingsService.Save(); }
    public void SetTxPower(double v)
    { _adapter.SetTxPower(v); SettingsService.Current.Antenna.TxPower = v; SettingsService.Save(); }
    public void SetFrequency(double v)
    { _adapter.SetFrequency(v); SettingsService.Current.Antenna.Frequency = v; SettingsService.Save(); }

    public string BuildResultsText() => string.Join("\n", new[]
    {
        $"ERP: {Erp}",
        $"Beamwidth from gain: {BeamwidthFromGain}",
        $"Gain from beamwidth: {GainFromBeamwidth}",
        $"Wavelength: {Wavelength}",
    });

    private void ApplyOutput(AntennaOutput o)
    {
        Erp = o.ErpStr; BeamwidthFromGain = o.BeamwidthFromGainStr;
        GainFromBeamwidth = o.GainFromBeamwidthStr; Wavelength = o.WavelengthStr;
        IsValid = o.Valid;
        GainError = _adapter.GainError; AzBeamwidthError = _adapter.AzBeamwidthError;
        ElBeamwidthError = _adapter.ElBeamwidthError; TxPowerError = _adapter.TxPowerError;
        FrequencyError = _adapter.FrequencyError;
    }

    public event PropertyChangedEventHandler? PropertyChanged;
    private void Set<T>(ref T f, T v, [CallerMemberName] string? n = null)
    { if (Equals(f, v)) return; f = v; PropertyChanged?.Invoke(this, new(n)); }
}
