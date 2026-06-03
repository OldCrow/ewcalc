// ViewModels/DigitalViewModel.cs
using EwPresenterNet;
using Microsoft.UI.Dispatching;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace EwCalc.ViewModels;

public sealed class DigitalViewModel : INotifyPropertyChanged
{
    private readonly DigitalAdapter _adapter = new();
    private readonly DispatcherQueue _dispatcher;

    private string _ebNo = string.Empty,        _processGain = string.Empty,
                   _jammingMargin = string.Empty, _requiredJs = string.Empty;
    private bool _isValid = false;

    public string EbNo          { get => _ebNo;          private set => Set(ref _ebNo,          value); }
    public string ProcessGain   { get => _processGain;   private set => Set(ref _processGain,   value); }
    public string JammingMargin { get => _jammingMargin; private set => Set(ref _jammingMargin, value); }
    public string RequiredJs    { get => _requiredJs;    private set => Set(ref _requiredJs,    value); }
    public bool   IsValid       { get => _isValid;       private set => Set(ref _isValid,       value); }

    public double DefaultSnr                => _adapter.DefaultSnr;
    public double DefaultBandwidth          => _adapter.DefaultBandwidth;
    public double DefaultDataRate           => _adapter.DefaultDataRate;
    public double DefaultChipRate           => _adapter.DefaultChipRate;
    public double DefaultRequiredEbNo       => _adapter.DefaultRequiredEbNo;
    public double DefaultImplementationLoss => _adapter.DefaultImplementationLoss;

    public DigitalViewModel()
    {
        _dispatcher = DispatcherQueue.GetForCurrentThread();
        _adapter.Changed += o => _dispatcher.TryEnqueue(() => ApplyOutput(o));
        ApplyOutput(_adapter.CurrentOutput);
    }

    public void SetSnr               (double v) => _adapter.SetSnr(v);
    public void SetBandwidth         (double v) => _adapter.SetBandwidth(v);
    public void SetDataRate          (double v) => _adapter.SetDataRate(v);
    public void SetChipRate          (double v) => _adapter.SetChipRate(v);
    public void SetRequiredEbNo      (double v) => _adapter.SetRequiredEbNo(v);
    public void SetImplementationLoss(double v) => _adapter.SetImplementationLoss(v);

    private void ApplyOutput(DigitalOutput o)
    {
        EbNo = o.EbNoStr; ProcessGain = o.ProcessGainStr;
        JammingMargin = o.JammingMarginStr; RequiredJs = o.RequiredJsStr; IsValid = o.Valid;
    }

    public event PropertyChangedEventHandler? PropertyChanged;
    private void Set<T>(ref T f, T v, [CallerMemberName] string? n = null)
    { if (Equals(f, v)) return; f = v; PropertyChanged?.Invoke(this, new(n)); }
}
