// ViewModels/LinkViewModel.cs
using EwPresenterNet;
using Microsoft.UI.Dispatching;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace EwCalc.ViewModels;

public sealed class LinkViewModel : INotifyPropertyChanged
{
    private readonly LinkAdapter _adapter = new();
    private readonly DispatcherQueue _dispatcher;

    private string _receivedPower = string.Empty, _pathLoss = string.Empty,
                   _linkMargin = string.Empty,    _fresnelZone = string.Empty,
                   _regime = string.Empty,        _effectiveRange = string.Empty;
    private bool _isValid = false;

    public string ReceivedPower  { get => _receivedPower;  private set => Set(ref _receivedPower,  value); }
    public string PathLoss       { get => _pathLoss;       private set => Set(ref _pathLoss,       value); }
    public string LinkMargin     { get => _linkMargin;     private set => Set(ref _linkMargin,     value); }
    public string FresnelZone    { get => _fresnelZone;    private set => Set(ref _fresnelZone,    value); }
    public string Regime         { get => _regime;         private set => Set(ref _regime,         value); }
    public string EffectiveRange { get => _effectiveRange; private set => Set(ref _effectiveRange, value); }
    public bool   IsValid        { get => _isValid;        private set => Set(ref _isValid,        value); }

    public double DefaultTxPower       => _adapter.DefaultTxPower;
    public double DefaultTxGain        => _adapter.DefaultTxGain;
    public double DefaultRxGain        => _adapter.DefaultRxGain;
    public double DefaultDistance      => _adapter.DefaultDistance;
    public double DefaultTxHeight      => _adapter.DefaultTxHeight;
    public double DefaultRxHeight      => _adapter.DefaultRxHeight;
    public double DefaultFrequency     => _adapter.DefaultFrequency;
    public double DefaultRxSensitivity => _adapter.DefaultRxSensitivity;

    public LinkViewModel()
    {
        _dispatcher = DispatcherQueue.GetForCurrentThread();
        _adapter.Changed += o => _dispatcher.TryEnqueue(() => ApplyOutput(o));
        ApplyOutput(_adapter.CurrentOutput);
    }

    public void SetTxPower      (double v) => _adapter.SetTxPower(v);
    public void SetTxGain       (double v) => _adapter.SetTxGain(v);
    public void SetRxGain       (double v) => _adapter.SetRxGain(v);
    public void SetDistance     (double v) => _adapter.SetDistance(v);
    public void SetTxHeight     (double v) => _adapter.SetTxHeight(v);
    public void SetRxHeight     (double v) => _adapter.SetRxHeight(v);
    public void SetFrequency    (double v) => _adapter.SetFrequency(v);
    public void SetRxSensitivity(double v) => _adapter.SetRxSensitivity(v);

    private void ApplyOutput(LinkOutput o)
    {
        ReceivedPower = o.ReceivedPowerStr; PathLoss = o.PathLossStr;
        LinkMargin = o.LinkMarginStr; FresnelZone = o.FresnelZoneStr;
        Regime = o.RegimeStr; EffectiveRange = o.EffectiveRangeStr;
        IsValid = o.Valid;
    }

    public event PropertyChangedEventHandler? PropertyChanged;
    private void Set<T>(ref T f, T v, [CallerMemberName] string? n = null)
    { if (Equals(f, v)) return; f = v; PropertyChanged?.Invoke(this, new(n)); }
}
