// ViewModels/ReceiverViewModel.cs
using EwCalc.Helpers;
using EwPresenterNet;
using Microsoft.UI.Dispatching;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Windows.Input;

namespace EwCalc.ViewModels;

public sealed class StageItemViewModel : INotifyPropertyChanged
{
    private readonly ReceiverViewModel _owner;
    private double _noiseFigureDb;
    private double _gainDb;

    private int _index;
    public  int  Index      { get => _index; private set { if (_index != value) { _index = value; RaiseIndexDependentChanges(); } } }
    public string IndexLabel => $"S{Index + 1}";

    // Per-stage automation names for the dynamically templated NumberBoxes/Button in
    // ReceiverPage.xaml (#19) — these can't be static XAML AutomationProperties.Name
    // values since each row's index isn't known until the ItemsRepeater materializes it.
    public string NfAutomationName     => $"{IndexLabel} noise figure (dB)";
    public string GainAutomationName   => $"{IndexLabel} gain (dB)";
    public string RemoveAutomationName => $"Remove stage {IndexLabel}";

    private void RaiseIndexDependentChanges()
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(Index)));
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(IndexLabel)));
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(NfAutomationName)));
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(GainAutomationName)));
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(RemoveAutomationName)));
    }

    /// Update the display index after a sibling stage is added or removed.
    internal void UpdateIndex(int newIndex) => Index = newIndex;

    public double NoiseFigureDb
    {
        get => _noiseFigureDb;
        set { if (Set(ref _noiseFigureDb, value)) _owner.PushStages(); }
    }

    public double GainDb
    {
        get => _gainDb;
        set { if (Set(ref _gainDb, value)) _owner.PushStages(); }
    }

    public ICommand RemoveCommand { get; }

    public StageItemViewModel(ReceiverViewModel owner, int index, double nf, double gain)
    {
        _owner = owner; _index = index; _noiseFigureDb = nf; _gainDb = gain;
        RemoveCommand = new RelayCommand(() => owner.RemoveStage(this));
    }

    public event PropertyChangedEventHandler? PropertyChanged;

    private bool Set<T>(ref T field, T value, [CallerMemberName] string? name = null)
    {
        if (Equals(field, value)) return false;
        field = value;
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
        return true;
    }
}

public sealed class ReceiverViewModel : INotifyPropertyChanged, IDisposable
{
    private readonly ReceiverAdapter _adapter = new();
    private readonly DispatcherQueue _dispatcher;

    private string _sensitivity = string.Empty, _cascadedNf = string.Empty,
                   _sfdr2 = string.Empty,       _sfdr3 = string.Empty,
                   _digitalDr = string.Empty,   _digitalSqnr = string.Empty,
                   _systemNoiseTemp = string.Empty,
                   _systemNf = string.Empty;
    private bool _isValid;

    public string Sensitivity    { get => _sensitivity;    private set => Set(ref _sensitivity,    value); }
    public string CascadedNf     { get => _cascadedNf;     private set => Set(ref _cascadedNf,     value); }
    public string Sfdr2          { get => _sfdr2;          private set => Set(ref _sfdr2,          value); }
    public string Sfdr3          { get => _sfdr3;          private set => Set(ref _sfdr3,          value); }
    public string DigitalDr      { get => _digitalDr;      private set => Set(ref _digitalDr,      value); }
    public string DigitalSqnr    { get => _digitalSqnr;    private set => Set(ref _digitalSqnr,    value); }
    public string SystemNoiseTemp { get => _systemNoiseTemp; private set => Set(ref _systemNoiseTemp, value); }
    public string SystemNf       { get => _systemNf;       private set => Set(ref _systemNf,       value); }
    public bool   IsValid        { get => _isValid;        private set => Set(ref _isValid,        value); }

    public double DefaultBandwidth     => _adapter.DefaultBandwidth;
    public double DefaultNoiseFigure   => _adapter.DefaultNoiseFigure;
    public double DefaultRequiredSnr   => _adapter.DefaultRequiredSnr;
    public double DefaultSecondOrderIp => _adapter.DefaultSecondOrderIp;
    public double DefaultThirdOrderIp  => _adapter.DefaultThirdOrderIp;
    public int    DefaultAdcBits       => _adapter.DefaultAdcBits;

    private FieldValidationError _bandwidthError     = FieldValidationError.None;
    private FieldValidationError _noiseFigureError   = FieldValidationError.None;
    private FieldValidationError _requiredSnrError   = FieldValidationError.None;
    private FieldValidationError _secondOrderIpError = FieldValidationError.None;
    private FieldValidationError _thirdOrderIpError  = FieldValidationError.None;
    private FieldValidationError _adcBitsError       = FieldValidationError.None;
    private FieldValidationError _stageNfError       = FieldValidationError.None;

    public FieldValidationError BandwidthError     { get => _bandwidthError;     private set => Set(ref _bandwidthError,     value); }
    public FieldValidationError NoiseFigureError   { get => _noiseFigureError;   private set => Set(ref _noiseFigureError,   value); }
    public FieldValidationError RequiredSnrError   { get => _requiredSnrError;   private set => Set(ref _requiredSnrError,   value); }
    public FieldValidationError SecondOrderIpError { get => _secondOrderIpError; private set => Set(ref _secondOrderIpError, value); }
    public FieldValidationError ThirdOrderIpError  { get => _thirdOrderIpError;  private set => Set(ref _thirdOrderIpError,  value); }
    public FieldValidationError AdcBitsError       { get => _adcBitsError;       private set => Set(ref _adcBitsError,       value); }
    /// <summary>Aggregate error for the Friis stage chain; surfaced on the "NOISE CHAIN"
    /// section border since individual stage rows aren't independently validated.</summary>
    public FieldValidationError StageNfError       { get => _stageNfError;       private set => Set(ref _stageNfError,       value); }

    public ObservableCollection<StageItemViewModel> Stages { get; } = [];
    public ICommand AddStageCommand { get; }

    public ReceiverViewModel()
    {
        _dispatcher = DispatcherQueue.GetForCurrentThread();

        var saved = SettingsService.Current.Receiver;
        if (saved.Bandwidth     is double bw) _adapter.SetBandwidth(bw);
        if (saved.NoiseFigure   is double nf) _adapter.SetNoiseFigure(nf);
        if (saved.RequiredSnr   is double sn) _adapter.SetRequiredSnr(sn);
        if (saved.SecondOrderIp is double ip2) _adapter.SetSecondOrderIp(ip2);
        if (saved.ThirdOrderIp  is double ip3) _adapter.SetThirdOrderIp(ip3);
        if (saved.AdcBits       is int bits)   _adapter.SetAdcBits(bits);
        if (saved.Stages is { Count: > 0 } savedStages)
            _adapter.SetStages(savedStages.Select(s => new StageInput { NoiseFigureDb = s.NoiseFigureDb, GainDb = s.GainDb }).ToArray());

        _adapter.Changed += o => _dispatcher.TryEnqueue(() => ApplyOutput(o));

        var defaultStages = _adapter.GetStages();
        for (int i = 0; i < defaultStages.Length; i++)
            Stages.Add(new StageItemViewModel(this, i, defaultStages[i].NoiseFigureDb, defaultStages[i].GainDb));

        AddStageCommand = new RelayCommand(AddStage);
        ApplyOutput(_adapter.CurrentOutput);
    }

    public void SetBandwidth(double v)
    { _adapter.SetBandwidth(v); SettingsService.Current.Receiver.Bandwidth = v; SettingsService.Save(); }
    public void SetNoiseFigure(double v)
    { _adapter.SetNoiseFigure(v); SettingsService.Current.Receiver.NoiseFigure = v; SettingsService.Save(); }
    public void SetRequiredSnr(double v)
    { _adapter.SetRequiredSnr(v); SettingsService.Current.Receiver.RequiredSnr = v; SettingsService.Save(); }
    public void SetSecondOrderIp(double v)
    { _adapter.SetSecondOrderIp(v); SettingsService.Current.Receiver.SecondOrderIp = v; SettingsService.Save(); }
    public void SetThirdOrderIp(double v)
    { _adapter.SetThirdOrderIp(v); SettingsService.Current.Receiver.ThirdOrderIp = v; SettingsService.Save(); }
    public void SetAdcBits(int v)
    { _adapter.SetAdcBits(v); SettingsService.Current.Receiver.AdcBits = v; SettingsService.Save(); }

    internal void PushStages()
    {
        var arr = new StageInput[Stages.Count];
        for (int i = 0; i < Stages.Count; i++)
            arr[i] = new StageInput { NoiseFigureDb = Stages[i].NoiseFigureDb, GainDb = Stages[i].GainDb };
        _adapter.SetStages(arr);
        SettingsService.Current.Receiver.Stages =
            Stages.Select(s => new StageSetting { NoiseFigureDb = s.NoiseFigureDb, GainDb = s.GainDb }).ToList();
        SettingsService.Save();
    }

    internal void RemoveStage(StageItemViewModel stage)
    {
        if (Stages.Count <= 1) return;
        Stages.Remove(stage);
        ReindexStages();
        PushStages();
    }

    private void AddStage()
    {
        Stages.Add(new StageItemViewModel(this, Stages.Count, 3.0, 0.0));
        // No reindex needed on add — new item already has the correct index.
        PushStages();
    }

    private void ReindexStages()
    {
        for (int i = 0; i < Stages.Count; i++)
            Stages[i].UpdateIndex(i);
    }

    public string BuildResultsText() => string.Join("\n", new[]
    {
        $"Sensitivity: {Sensitivity}",
        $"Cascaded NF: {CascadedNf}",
        $"Sys. noise temp: {SystemNoiseTemp}",
        $"Sys. NF equiv.: {SystemNf}",
        $"SFDR (2nd): {Sfdr2}",
        $"SFDR (3rd): {Sfdr3}",
        $"Digital DR: {DigitalDr}",
        $"SQNR: {DigitalSqnr}",
    });

    private void ApplyOutput(ReceiverOutput o)
    {
        Sensitivity = o.SensitivityStr; CascadedNf = o.CascadedNfStr;
        Sfdr2 = o.Sfdr2Str; Sfdr3 = o.Sfdr3Str; DigitalDr = o.DigitalDrStr;
        DigitalSqnr = o.DigitalSqnrStr;
        SystemNoiseTemp = o.SystemNoiseTempStr; SystemNf = o.SystemNfStr; IsValid = o.Valid;
        BandwidthError = _adapter.BandwidthError; NoiseFigureError = _adapter.NoiseFigureError;
        RequiredSnrError = _adapter.RequiredSnrError; SecondOrderIpError = _adapter.SecondOrderIpError;
        ThirdOrderIpError = _adapter.ThirdOrderIpError; AdcBitsError = _adapter.AdcBitsError;
        StageNfError = _adapter.StageNfError;
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

internal sealed class RelayCommand(Action execute) : ICommand
{
    // CanExecute always returns true; empty accessors satisfy ICommand without
    // creating an unused backing field (suppresses CS0067).
    public event EventHandler? CanExecuteChanged { add { } remove { } }
    public bool CanExecute(object? parameter) => true;
    public void Execute(object? parameter) => execute();
}
