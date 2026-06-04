// ViewModels/PropagationViewModel.cs
using EwPresenterNet;
using Microsoft.UI.Dispatching;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace EwCalc.ViewModels;

public sealed class PropagationViewModel : INotifyPropertyChanged
{
    private readonly PropagationAdapter _adapter = new();
    private readonly DispatcherQueue _dispatcher;

    private string _pathLoss = string.Empty, _fspl = string.Empty,
                   _twoRayLoss = string.Empty, _fresnel = string.Empty,
                   _regime = string.Empty,
                   _earthBulge = string.Empty, _horizonRange = string.Empty,
                   _diffractionLoss = string.Empty;
    private bool _isValid = false;

    public string PathLoss        { get => _pathLoss;        private set => Set(ref _pathLoss,        value); }
    public string Fspl            { get => _fspl;            private set => Set(ref _fspl,            value); }
    public string TwoRayLoss      { get => _twoRayLoss;      private set => Set(ref _twoRayLoss,      value); }
    public string Fresnel         { get => _fresnel;         private set => Set(ref _fresnel,         value); }
    public string Regime          { get => _regime;          private set => Set(ref _regime,          value); }
    public string EarthBulge      { get => _earthBulge;      private set => Set(ref _earthBulge,      value); }
    public string HorizonRange    { get => _horizonRange;    private set => Set(ref _horizonRange,    value); }
    public string DiffractionLoss { get => _diffractionLoss; private set => Set(ref _diffractionLoss, value); }
    public bool   IsValid         { get => _isValid;         private set => Set(ref _isValid,         value); }

    private FieldValidationError _distanceError  = FieldValidationError.None;
    private FieldValidationError _frequencyError = FieldValidationError.None;
    private FieldValidationError _txHeightError  = FieldValidationError.None;
    private FieldValidationError _rxHeightError  = FieldValidationError.None;

    public FieldValidationError DistanceError  { get => _distanceError;  private set => Set(ref _distanceError,  value); }
    public FieldValidationError FrequencyError { get => _frequencyError; private set => Set(ref _frequencyError, value); }
    public FieldValidationError TxHeightError  { get => _txHeightError;  private set => Set(ref _txHeightError,  value); }
    public FieldValidationError RxHeightError  { get => _rxHeightError;  private set => Set(ref _rxHeightError,  value); }

    public double DefaultDistance          => _adapter.DefaultDistance;
    public double DefaultFrequency         => _adapter.DefaultFrequency;
    public double DefaultTxHeight          => _adapter.DefaultTxHeight;
    public double DefaultRxHeight          => _adapter.DefaultRxHeight;
    public double DefaultObstructionHeight => _adapter.DefaultObstructionHeight;

    public PropagationViewModel()
    {
        _dispatcher = DispatcherQueue.GetForCurrentThread();
        _adapter.Changed += output => _dispatcher.TryEnqueue(() => ApplyOutput(output));
        ApplyOutput(_adapter.CurrentOutput);
    }

    public void SetDistance         (double km)     => _adapter.SetDistance(km);
    public void SetFrequency        (double mhz)    => _adapter.SetFrequency(mhz);
    public void SetTxHeight         (double meters) => _adapter.SetTxHeight(meters);
    public void SetRxHeight         (double meters) => _adapter.SetRxHeight(meters);
    public void SetObstructionHeight(double meters) => _adapter.SetObstructionHeight(meters);

    private void ApplyOutput(PropagationOutput o)
    {
        PathLoss = o.PathLossStr; Fspl = o.FsplStr; TwoRayLoss = o.TwoRayLossStr;
        Fresnel = o.FresnelZoneStr; Regime = o.RegimeStr;
        EarthBulge = o.EarthBulgeStr; HorizonRange = o.HorizonRangeStr;
        DiffractionLoss = o.DiffractionLossStr;
        IsValid = o.Valid;
        DistanceError = _adapter.DistanceError; FrequencyError = _adapter.FrequencyError;
        TxHeightError = _adapter.TxHeightError; RxHeightError = _adapter.RxHeightError;
    }

    public event PropertyChangedEventHandler? PropertyChanged;
    private void Set<T>(ref T field, T value, [CallerMemberName] string? name = null)
    {
        if (Equals(field, value)) return;
        field = value;
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
    }
}
