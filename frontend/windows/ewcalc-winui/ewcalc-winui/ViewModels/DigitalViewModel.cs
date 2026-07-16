// ViewModels/DigitalViewModel.cs
using EwCalc.Helpers;
using EwPresenterNet;
using Microsoft.UI.Dispatching;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace EwCalc.ViewModels;

public sealed class DigitalViewModel : INotifyPropertyChanged, IDisposable
{
    private readonly DigitalAdapter _adapter = new();
    private readonly DispatcherQueue _dispatcher;

    private string _ebNo = string.Empty,        _requiredSnrForEbNo = string.Empty,
                   _processGain = string.Empty,  _jammingMargin = string.Empty,
                   _requiredJs = string.Empty;
    private bool _isValid;

    public string EbNo               { get => _ebNo;               private set => Set(ref _ebNo,               value); }
    public string RequiredSnrForEbNo { get => _requiredSnrForEbNo; private set => Set(ref _requiredSnrForEbNo, value); }
    public string ProcessGain        { get => _processGain;        private set => Set(ref _processGain,        value); }
    public string JammingMargin { get => _jammingMargin; private set => Set(ref _jammingMargin, value); }
    public string RequiredJs    { get => _requiredJs;    private set => Set(ref _requiredJs,    value); }
    public bool   IsValid       { get => _isValid;       private set => Set(ref _isValid,       value); }

    public double DefaultSnr                => _adapter.DefaultSnr;
    public double DefaultBandwidth          => _adapter.DefaultBandwidth;
    public double DefaultDataRate           => _adapter.DefaultDataRate;
    public double DefaultChipRate           => _adapter.DefaultChipRate;
    public double DefaultRequiredEbNo       => _adapter.DefaultRequiredEbNo;
    public double DefaultImplementationLoss => _adapter.DefaultImplementationLoss;

    private FieldValidationError _snrError                = FieldValidationError.None;
    private FieldValidationError _bandwidthError           = FieldValidationError.None;
    private FieldValidationError _dataRateError            = FieldValidationError.None;
    private FieldValidationError _chipRateError            = FieldValidationError.None;
    private FieldValidationError _requiredEbNoError        = FieldValidationError.None;
    private FieldValidationError _implementationLossError  = FieldValidationError.None;

    public FieldValidationError SnrError                { get => _snrError;               private set => Set(ref _snrError,               value); }
    public FieldValidationError BandwidthError          { get => _bandwidthError;         private set => Set(ref _bandwidthError,         value); }
    public FieldValidationError DataRateError           { get => _dataRateError;          private set => Set(ref _dataRateError,          value); }
    public FieldValidationError ChipRateError           { get => _chipRateError;          private set => Set(ref _chipRateError,          value); }
    public FieldValidationError RequiredEbNoError       { get => _requiredEbNoError;      private set => Set(ref _requiredEbNoError,      value); }
    public FieldValidationError ImplementationLossError { get => _implementationLossError; private set => Set(ref _implementationLossError, value); }

    public DigitalViewModel()
    {
        _dispatcher = DispatcherQueue.GetForCurrentThread();

        var saved = SettingsService.Current.Digital;
        if (saved.Snr                is double s)   _adapter.SetSnr(s);
        if (saved.Bandwidth          is double bw)  _adapter.SetBandwidth(bw);
        if (saved.DataRate           is double dr)  _adapter.SetDataRate(dr);
        if (saved.ChipRate           is double cr)  _adapter.SetChipRate(cr);
        if (saved.RequiredEbNo       is double eb)  _adapter.SetRequiredEbNo(eb);
        if (saved.ImplementationLoss is double il)  _adapter.SetImplementationLoss(il);

        _adapter.Changed += o => _dispatcher.TryEnqueue(() => ApplyOutput(o));
        ApplyOutput(_adapter.CurrentOutput);
    }

    public void SetSnr(double v)
    { _adapter.SetSnr(v); SettingsService.Current.Digital.Snr = v; SettingsService.Save(); }
    public void SetBandwidth(double v)
    { _adapter.SetBandwidth(v); SettingsService.Current.Digital.Bandwidth = v; SettingsService.Save(); }
    public void SetDataRate(double v)
    { _adapter.SetDataRate(v); SettingsService.Current.Digital.DataRate = v; SettingsService.Save(); }
    public void SetChipRate(double v)
    { _adapter.SetChipRate(v); SettingsService.Current.Digital.ChipRate = v; SettingsService.Save(); }
    public void SetRequiredEbNo(double v)
    { _adapter.SetRequiredEbNo(v); SettingsService.Current.Digital.RequiredEbNo = v; SettingsService.Save(); }
    public void SetImplementationLoss(double v)
    { _adapter.SetImplementationLoss(v); SettingsService.Current.Digital.ImplementationLoss = v; SettingsService.Save(); }

    public string BuildResultsText() => string.Join("\n", new[]
    {
        $"Eb/N0: {EbNo}",
        $"Required SNR: {RequiredSnrForEbNo}",
        $"Process gain: {ProcessGain}",
        $"Jamming margin: {JammingMargin}",
        $"Required J/S: {RequiredJs}",
    });

    private void ApplyOutput(DigitalOutput o)
    {
        EbNo = o.EbNoStr; RequiredSnrForEbNo = o.RequiredSnrForEbNoStr;
        ProcessGain = o.ProcessGainStr; JammingMargin = o.JammingMarginStr;
        RequiredJs = o.RequiredJsStr; IsValid = o.Valid;
        SnrError = _adapter.SnrError; BandwidthError = _adapter.BandwidthError;
        DataRateError = _adapter.DataRateError; ChipRateError = _adapter.ChipRateError;
        RequiredEbNoError = _adapter.RequiredEbNoError; ImplementationLossError = _adapter.ImplementationLossError;
    }

    public event PropertyChangedEventHandler? PropertyChanged;
    private void Set<T>(ref T f, T v, [CallerMemberName] string? n = null)
    { if (Equals(f, v)) return; f = v; PropertyChanged?.Invoke(this, new(n)); }

    public void Dispose() => _adapter.Dispose();
}
