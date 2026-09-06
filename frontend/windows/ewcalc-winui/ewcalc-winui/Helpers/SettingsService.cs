// Helpers/SettingsService.cs
// Persists user inputs across sessions as a versioned JSON file under
// %LOCALAPPDATA%\ewcalc\settings.json. A plain file keeps persistence behavior
// identical for unpackaged dev builds and MSIX-packaged release builds.
using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Threading;

namespace EwCalc.Helpers;

public sealed class StageSetting
{
    public double NoiseFigureDb { get; set; }
    public double GainDb { get; set; }
}

public sealed class PropagationSettings
{
    public double? Distance { get; set; }
    public double? Frequency { get; set; }
    public double? TxHeight { get; set; }
    public double? RxHeight { get; set; }
    public double? ObstructionHeight { get; set; }
}

public sealed class AntennaSettings
{
    public double? Gain { get; set; }
    public double? AzBeamwidth { get; set; }
    public double? ElBeamwidth { get; set; }
    public double? TxPower { get; set; }
    public double? Frequency { get; set; }
}

public sealed class LinkSettings
{
    public double? TxPower { get; set; }
    public double? TxGain { get; set; }
    public double? RxGain { get; set; }
    public double? Distance { get; set; }
    public double? TxHeight { get; set; }
    public double? RxHeight { get; set; }
    public double? Frequency { get; set; }
    public double? RxSensitivity { get; set; }
}

public sealed class ReceiverSettings
{
    public double? Bandwidth { get; set; }
    public double? NoiseFigure { get; set; }
    public double? RequiredSnr { get; set; }
    public double? SecondOrderIp { get; set; }
    public double? ThirdOrderIp { get; set; }
    public int? AdcBits { get; set; }
    public List<StageSetting>? Stages { get; set; }
}

public sealed class JammingSettings
{
    public double? SignalErp { get; set; }
    public double? JammerErp { get; set; }
    public double? SignalToRxDist { get; set; }
    public double? JammerToRxDist { get; set; }
    public double? SignalTxHeight { get; set; }
    public double? JammerHeight { get; set; }
    public double? RxHeight { get; set; }
    public double? Frequency { get; set; }
    public double? RxGainSignal { get; set; }
    public double? RxGainJammer { get; set; }
    public double? SignalBandwidth { get; set; }
    public double? HopRange { get; set; }
    public double? JsThreshold { get; set; }
}

public sealed class LocationSettings
{
    public double? RmsBearingError { get; set; }
    public double? AoaRange { get; set; }
    public double? RmsTimeError { get; set; }
    public double? Baseline { get; set; }
    public double? SemiMajor { get; set; }
    public double? SemiMinor { get; set; }
}

public sealed class RadarSettings
{
    public double? TxPower { get; set; }
    public double? AntennaGain { get; set; }
    public double? TargetRcs { get; set; }
    public double? Frequency { get; set; }
    public double? SystemLosses { get; set; }
    public double? NoiseFigure { get; set; }
    public double? Bandwidth { get; set; }
    public double? RequiredSnr { get; set; }
    public double? TimeBandwidthProd { get; set; }
    public int? NumPulses { get; set; }
}

public sealed class DetectionSettings
{
    public double? Pd { get; set; }
    public double? PfaExponent { get; set; }
    public int? NumPulses { get; set; }
    public int? SwerlingCase { get; set; }
    public double? Beamwidth { get; set; }
    public double? ScanRate { get; set; }
    public double? Prf { get; set; }
    public double? Bandwidth { get; set; }
}

public sealed class DopplerSettings
{
    public double? Frequency { get; set; }
    public double? RadialSpeed { get; set; }
    public double? Prf { get; set; }
    public double? Bandwidth { get; set; }
    public double? TargetRange { get; set; }
    public double? BeamwidthAz { get; set; }
    public double? BeamwidthEl { get; set; }
}

public sealed class DigitalSettings
{
    public double? Snr { get; set; }
    public double? Bandwidth { get; set; }
    public double? DataRate { get; set; }
    public double? ChipRate { get; set; }
    public double? RequiredEbNo { get; set; }
    public double? ImplementationLoss { get; set; }
}

public sealed class AppSettings
{
    /// <summary>Bumped whenever the shape of this class changes incompatibly.
    /// A mismatched version on load is treated as "no saved settings".</summary>
    public int Version { get; set; } = SettingsService.CurrentVersion;

    public PropagationSettings Propagation { get; set; } = new();
    public AntennaSettings     Antenna     { get; set; } = new();
    public LinkSettings        Link        { get; set; } = new();
    public ReceiverSettings    Receiver    { get; set; } = new();
    public JammingSettings     Jamming     { get; set; } = new();
    public LocationSettings    Location    { get; set; } = new();
    public RadarSettings       Radar       { get; set; } = new();
    public DetectionSettings   Detection   { get; set; } = new();
    public DopplerSettings     Doppler     { get; set; } = new();
    public DigitalSettings     Digital     { get; set; } = new();
}

/// <summary>
/// Loads/saves <see cref="AppSettings"/> to a JSON file in %LOCALAPPDATA%\ewcalc.
/// Writes are debounced (rapid NumberBox drags coalesce into one disk write) and always
/// flushed synchronously on app exit via <see cref="Flush"/>.
/// </summary>
public static class SettingsService
{
    public const int CurrentVersion = 1;

    private static readonly string DirectoryPath =
        Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "ewcalc");
    private static readonly string FilePath = Path.Combine(DirectoryPath, "settings.json");
    private static readonly TimeSpan SaveDebounce = TimeSpan.FromMilliseconds(300);

    private static readonly JsonSerializerOptions Options = new()
    {
        WriteIndented = true,
        DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull,
    };

    private static readonly object Lock = new();
    private static Timer? _saveTimer;

    public static AppSettings Current { get; private set; } = LoadFromDisk();

    /// <summary>Schedules a debounced write of <see cref="Current"/> to disk.</summary>
    public static void Save()
    {
        lock (Lock)
        {
            _saveTimer?.Dispose();
            _saveTimer = new Timer(_ => SaveImmediate(), null, SaveDebounce, Timeout.InfiniteTimeSpan);
        }
    }

    /// <summary>Cancels any pending debounced write and persists immediately. Call on app exit.</summary>
    public static void Flush()
    {
        lock (Lock)
        {
            _saveTimer?.Dispose();
            _saveTimer = null;
        }
        SaveImmediate();
    }

    /// <summary>Clears saved inputs both in memory and on disk. Callers should re-create any
    /// already-open pages/view models so they pick up presenter defaults again.</summary>
    public static void ResetToDefaults()
    {
        lock (Lock)
        {
            _saveTimer?.Dispose();
            _saveTimer = null;
            Current = new AppSettings();
            try
            {
                if (File.Exists(FilePath)) File.Delete(FilePath);
            }
            catch (IOException) { /* best-effort; ignore */ }
            catch (UnauthorizedAccessException) { /* best-effort; ignore */ }
        }
    }

    private static void SaveImmediate()
    {
        lock (Lock)
        {
            try
            {
                Directory.CreateDirectory(DirectoryPath);
                var json = JsonSerializer.Serialize(Current, Options);
                var tmpPath = FilePath + ".tmp";
                File.WriteAllText(tmpPath, json);
                File.Copy(tmpPath, FilePath, overwrite: true);
                File.Delete(tmpPath);
            }
            catch (IOException) { /* best-effort persistence; never let disk errors crash the UI */ }
            catch (UnauthorizedAccessException) { /* best-effort persistence */ }
        }
    }

    private static AppSettings LoadFromDisk()
    {
        try
        {
            if (!File.Exists(FilePath)) return new AppSettings();
            var json = File.ReadAllText(FilePath);
            var loaded = JsonSerializer.Deserialize<AppSettings>(json, Options);
            if (loaded is null || loaded.Version != CurrentVersion) return new AppSettings();
            return loaded;
        }
        catch (IOException) { return new AppSettings(); }
        catch (UnauthorizedAccessException) { return new AppSettings(); }
        catch (JsonException) { return new AppSettings(); }
    }
}
