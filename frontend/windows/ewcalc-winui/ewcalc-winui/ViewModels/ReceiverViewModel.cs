// ViewModels/ReceiverViewModel.cs
using EwPresenterNet;
using Microsoft.UI.Dispatching;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows.Input;

namespace EwCalc.ViewModels;

public sealed class StageItemViewModel : INotifyPropertyChanged
{
    private readonly ReceiverViewModel _owner;
    private double _noiseFigureDb;
    private double _gainDb;

    private int _index;
    public  int  Index      { get => _index; private set { if (_index != value) { _index = value; PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(Index))); PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(IndexLabel))); } } }
    public string IndexLabel => $"S{Index + 1}";

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

public sealed class ReceiverViewModel : INotifyPropertyChanged
{
    private readonly ReceiverAdapter _adapter = new();
    private readonly DispatcherQueue _dispatcher;

    private string _sensitivity = string.Empty, _cascadedNf = string.Empty,
                   _sfdr2 = string.Empty,       _sfdr3 = string.Empty,
                   _digitalDr = string.Empty,   _systemNoiseTemp = string.Empty,
                   _systemNf = string.Empty;
    private bool _isValid = false;

    public string Sensitivity    { get => _sensitivity;    private set => Set(ref _sensitivity,    value); }
    public string CascadedNf     { get => _cascadedNf;     private set => Set(ref _cascadedNf,     value); }
    public string Sfdr2          { get => _sfdr2;          private set => Set(ref _sfdr2,          value); }
    public string Sfdr3          { get => _sfdr3;          private set => Set(ref _sfdr3,          value); }
    public string DigitalDr      { get => _digitalDr;      private set => Set(ref _digitalDr,      value); }
    public string SystemNoiseTemp { get => _systemNoiseTemp; private set => Set(ref _systemNoiseTemp, value); }
    public string SystemNf       { get => _systemNf;       private set => Set(ref _systemNf,       value); }
    public bool   IsValid        { get => _isValid;        private set => Set(ref _isValid,        value); }

    public double DefaultBandwidth     => _adapter.DefaultBandwidth;
    public double DefaultNoiseFigure   => _adapter.DefaultNoiseFigure;
    public double DefaultRequiredSnr   => _adapter.DefaultRequiredSnr;
    public double DefaultSecondOrderIp => _adapter.DefaultSecondOrderIp;
    public double DefaultThirdOrderIp  => _adapter.DefaultThirdOrderIp;
    public int    DefaultAdcBits       => _adapter.DefaultAdcBits;

    public ObservableCollection<StageItemViewModel> Stages { get; } = [];
    public ICommand AddStageCommand { get; }

    public ReceiverViewModel()
    {
        _dispatcher = DispatcherQueue.GetForCurrentThread();
        _adapter.Changed += o => _dispatcher.TryEnqueue(() => ApplyOutput(o));

        var defaultStages = _adapter.GetStages();
        for (int i = 0; i < defaultStages.Length; i++)
            Stages.Add(new StageItemViewModel(this, i, defaultStages[i].NoiseFigureDb, defaultStages[i].GainDb));

        AddStageCommand = new RelayCommand(AddStage);
        ApplyOutput(_adapter.CurrentOutput);
    }

    public void SetBandwidth    (double v) => _adapter.SetBandwidth(v);
    public void SetNoiseFigure  (double v) => _adapter.SetNoiseFigure(v);
    public void SetRequiredSnr  (double v) => _adapter.SetRequiredSnr(v);
    public void SetSecondOrderIp(double v) => _adapter.SetSecondOrderIp(v);
    public void SetThirdOrderIp (double v) => _adapter.SetThirdOrderIp(v);
    public void SetAdcBits      (int v)    => _adapter.SetAdcBits(v);

    internal void PushStages()
    {
        var arr = new StageInput[Stages.Count];
        for (int i = 0; i < Stages.Count; i++)
            arr[i] = new StageInput { NoiseFigureDb = Stages[i].NoiseFigureDb, GainDb = Stages[i].GainDb };
        _adapter.SetStages(arr);
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

    private void ApplyOutput(ReceiverOutput o)
    {
        Sensitivity = o.SensitivityStr; CascadedNf = o.CascadedNfStr;
        Sfdr2 = o.Sfdr2Str; Sfdr3 = o.Sfdr3Str; DigitalDr = o.DigitalDrStr;
        SystemNoiseTemp = o.SystemNoiseTempStr; SystemNf = o.SystemNfStr; IsValid = o.Valid;
    }

    public event PropertyChangedEventHandler? PropertyChanged;

    private void Set<T>(ref T field, T value, [CallerMemberName] string? name = null)
    {
        if (Equals(field, value)) return;
        field = value;
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
    }
}

internal sealed class RelayCommand(Action execute) : ICommand
{
    // CanExecute always returns true; empty accessors satisfy ICommand without
    // creating an unused backing field (suppresses CS0067).
    public event EventHandler? CanExecuteChanged { add { } remove { } }
    public bool CanExecute(object? parameter) => true;
    public void Execute(object? parameter) => execute();
}
