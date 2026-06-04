// ViewModels/AntennaViewModel.cs
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

    public double DefaultGain        => _adapter.DefaultGain;
    public double DefaultAzBeamwidth => _adapter.DefaultAzBeamwidth;
    public double DefaultElBeamwidth => _adapter.DefaultElBeamwidth;
    public double DefaultTxPower     => _adapter.DefaultTxPower;
    public double DefaultFrequency   => _adapter.DefaultFrequency;

    public AntennaViewModel()
    {
        _dispatcher = DispatcherQueue.GetForCurrentThread();
        _adapter.Changed += o => _dispatcher.TryEnqueue(() => ApplyOutput(o));
        ApplyOutput(_adapter.CurrentOutput);
    }

    public void SetGain        (double v) => _adapter.SetGain(v);
    public void SetAzBeamwidth (double v) => _adapter.SetAzBeamwidth(v);
    public void SetElBeamwidth (double v) => _adapter.SetElBeamwidth(v);
    public void SetTxPower     (double v) => _adapter.SetTxPower(v);
    public void SetFrequency   (double v) => _adapter.SetFrequency(v);

    private void ApplyOutput(AntennaOutput o)
    {
        Erp = o.ErpStr; BeamwidthFromGain = o.BeamwidthFromGainStr;
        GainFromBeamwidth = o.GainFromBeamwidthStr; Wavelength = o.WavelengthStr;
        IsValid = o.Valid;
    }

    public event PropertyChangedEventHandler? PropertyChanged;
    private void Set<T>(ref T f, T v, [CallerMemberName] string? n = null)
    { if (Equals(f, v)) return; f = v; PropertyChanged?.Invoke(this, new(n)); }
}
