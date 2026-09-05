// SavedInputs.swift
import Foundation

/// Snapshot of every calculator's user-editable inputs (#20), persisted to
/// UserDefaults as JSON. A single versioned blob was chosen over per-field
/// AppStorage keys or a Codable file under Application Support: it's one
/// atomic read/write, needs no file-system plumbing, and is small enough
/// (a few dozen doubles) that UserDefaults' plist backing is not a concern.
///
/// `formatVersion` exists so a future incompatible change to this shape can
/// be detected; `EwCalcStore.restoreInputs()` currently discards saved data
/// whose version doesn't match rather than attempting a migration, since v1
/// is the first persisted format.
struct SavedInputs: Codable {
    static let currentFormatVersion = 1

    var formatVersion: Int = SavedInputs.currentFormatVersion

    struct Propagation: Codable {
        var distance: Double
        var frequency: Double
        var txHeight: Double
        var rxHeight: Double
        var obstructionHeight: Double
    }

    struct Link: Codable {
        var txPower: Double
        var txGain: Double
        var rxGain: Double
        var distance: Double
        var txHeight: Double
        var rxHeight: Double
        var frequency: Double
        var rxSensitivity: Double
    }

    struct StageInput: Codable {
        var noiseFigureDb: Double
        var gainDb: Double
    }

    struct Receiver: Codable {
        var bandwidth: Double
        var noiseFigure: Double
        var requiredSnr: Double
        var secondOrderIp: Double
        var thirdOrderIp: Double
        var adcBits: Int
        var stages: [StageInput]
    }

    struct Jamming: Codable {
        var signalErp: Double
        var jammerErp: Double
        var signalDist: Double
        var jammerDist: Double
        var signalHeight: Double
        var jammerHeight: Double
        var rxHeight: Double
        var frequency: Double
        var rxGainSignal: Double
        var rxGainJammer: Double
        var signalBandwidth: Double
        var hopRange: Double
        var jsThreshold: Double
    }

    struct Location: Codable {
        var rmsBearingError: Double
        var aoaRange: Double
        var rmsTimeError: Double
        var baseline: Double
        var semiMajor: Double
        var semiMinor: Double
    }

    struct Radar: Codable {
        var txPower: Double
        var antennaGain: Double
        var targetRcs: Double
        var frequency: Double
        var systemLosses: Double
        var noiseFigure: Double
        var bandwidth: Double
        var requiredSnr: Double
        var timeBandwidth: Double
        var numPulses: Int
    }

    struct Detection: Codable {
        var pd: Double
        var pfaExponent: Double
        var numPulses: Int
        var swerlingCase: Int
        var beamwidth: Double
        var scanRate: Double
        var prf: Double
        var bandwidth: Double
    }

    struct Doppler: Codable {
        var frequency: Double
        var radialSpeed: Double
        var prf: Double
        var bandwidth: Double
        var targetRange: Double
        var beamwidthAz: Double
        var beamwidthEl: Double
    }

    struct Digital: Codable {
        var snr: Double
        var bandwidth: Double
        var dataRate: Double
        var chipRate: Double
        var requiredEbNo: Double
        var implementationLoss: Double
    }

    struct Antenna: Codable {
        var gain: Double
        var azBeamwidth: Double
        var elBeamwidth: Double
        var txPower: Double
        var frequency: Double
    }

    var propagation: Propagation
    var link: Link
    var receiver: Receiver
    var jamming: Jamming
    var location: Location
    var radar: Radar
    // Optional: added after format v1 shipped. Synthesized Codable decodes a
    // missing key as nil, so pre-Detection saved blobs still load without a
    // format-version bump; `apply` simply skips it when absent.
    var detection: Detection?
    // Optional: added after format v1 shipped. Synthesized Codable decodes a
    // missing key as nil, so pre-Doppler saved blobs still load without a
    // format-version bump; `apply` simply skips it when absent.
    var doppler: Doppler?
    var digital: Digital
    var antenna: Antenna
}

// ── Per-adapter capture/apply ────────────────────────────────────────────────
// Each sub-struct knows how to read its adapter's current values (`from:`)
// and how to write them back (`apply(to:)`). Keeping this next to the
// adapters means adding a field to a bridge/adapter needs one matching edit
// here rather than plumbing through EwCalcStore.

extension SavedInputs.Propagation {
    init(from a: PropagationAdapter) {
        distance          = a.defaultDistance
        frequency         = a.defaultFrequency
        txHeight          = a.defaultTxHeight
        rxHeight          = a.defaultRxHeight
        obstructionHeight = a.defaultObstructionHeight
    }
    func apply(to a: PropagationAdapter) {
        a.setDistance(distance)
        a.setFrequency(frequency)
        a.setTxHeight(txHeight)
        a.setRxHeight(rxHeight)
        a.setObstructionHeight(obstructionHeight)
    }
}

extension SavedInputs.Link {
    init(from a: LinkAdapter) {
        txPower       = a.defaultTxPower
        txGain        = a.defaultTxGain
        rxGain        = a.defaultRxGain
        distance      = a.defaultDistance
        txHeight      = a.defaultTxHeight
        rxHeight      = a.defaultRxHeight
        frequency     = a.defaultFrequency
        rxSensitivity = a.defaultRxSensitivity
    }
    func apply(to a: LinkAdapter) {
        a.setTxPower(txPower)
        a.setTxGain(txGain)
        a.setRxGain(rxGain)
        a.setDistance(distance)
        a.setTxHeight(txHeight)
        a.setRxHeight(rxHeight)
        a.setFrequency(frequency)
        a.setRxSensitivity(rxSensitivity)
    }
}

extension SavedInputs.Receiver {
    init(from a: ReceiverAdapter) {
        bandwidth     = a.defaultBandwidth
        noiseFigure   = a.defaultNoiseFigure
        requiredSnr   = a.defaultRequiredSnr
        secondOrderIp = a.defaultSecondOrder
        thirdOrderIp  = a.defaultThirdOrder
        adcBits       = a.defaultAdcBits
        stages        = a.defaultStages.map {
            SavedInputs.StageInput(noiseFigureDb: $0.noise_figure_db, gainDb: $0.gain_db)
        }
    }
    func apply(to a: ReceiverAdapter) {
        a.setBandwidth(bandwidth)
        a.setNoiseFigure(noiseFigure)
        a.setRequiredSnr(requiredSnr)
        a.setSecondOrderIp(secondOrderIp)
        a.setThirdOrderIp(thirdOrderIp)
        a.setAdcBits(adcBits)
        a.setStages(stages.map { EwpStageInput(noise_figure_db: $0.noiseFigureDb, gain_db: $0.gainDb) })
    }
}

extension SavedInputs.Jamming {
    init(from a: JammingAdapter) {
        signalErp       = a.defaultSignalErp
        jammerErp       = a.defaultJammerErp
        signalDist      = a.defaultSignalDist
        jammerDist      = a.defaultJammerDist
        signalHeight    = a.defaultSignalHeight
        jammerHeight    = a.defaultJammerHeight
        rxHeight        = a.defaultRxHeight
        frequency       = a.defaultFrequency
        rxGainSignal    = a.defaultRxGainSignal
        rxGainJammer    = a.defaultRxGainJammer
        signalBandwidth = a.defaultSignalBandwidth
        hopRange        = a.defaultHopRange
        jsThreshold     = a.defaultJsThreshold
    }
    func apply(to a: JammingAdapter) {
        a.setSignalErp(signalErp)
        a.setJammerErp(jammerErp)
        a.setSignalDist(signalDist)
        a.setJammerDist(jammerDist)
        a.setSignalHeight(signalHeight)
        a.setJammerHeight(jammerHeight)
        a.setRxHeight(rxHeight)
        a.setFrequency(frequency)
        a.setRxGainSignal(rxGainSignal)
        a.setRxGainJammer(rxGainJammer)
        a.setSignalBandwidth(signalBandwidth)
        a.setHopRange(hopRange)
        a.setJsThreshold(jsThreshold)
    }
}

extension SavedInputs.Location {
    init(from a: LocationAdapter) {
        rmsBearingError = a.defaultRmsBearingError
        aoaRange        = a.defaultAoaRange
        rmsTimeError    = a.defaultRmsTimeError
        baseline        = a.defaultBaseline
        semiMajor       = a.defaultSemiMajor
        semiMinor       = a.defaultSemiMinor
    }
    func apply(to a: LocationAdapter) {
        a.setRmsBearingError(rmsBearingError)
        a.setAoaRange(aoaRange)
        a.setRmsTimeError(rmsTimeError)
        a.setBaseline(baseline)
        a.setSemiMajor(semiMajor)
        a.setSemiMinor(semiMinor)
    }
}

extension SavedInputs.Radar {
    init(from a: RadarAdapter) {
        txPower       = a.defaultTxPower
        antennaGain   = a.defaultAntennaGain
        targetRcs     = a.defaultTargetRcs
        frequency     = a.defaultFrequency
        systemLosses  = a.defaultSystemLosses
        noiseFigure   = a.defaultNoiseFigure
        bandwidth     = a.defaultBandwidth
        requiredSnr   = a.defaultRequiredSnr
        timeBandwidth = a.defaultTimeBandwidth
        numPulses     = a.defaultNumPulses
    }
    func apply(to a: RadarAdapter) {
        a.setTxPower(txPower)
        a.setAntennaGain(antennaGain)
        a.setTargetRcs(targetRcs)
        a.setFrequency(frequency)
        a.setSystemLosses(systemLosses)
        a.setNoiseFigure(noiseFigure)
        a.setBandwidth(bandwidth)
        a.setRequiredSnr(requiredSnr)
        a.setTimeBandwidth(timeBandwidth)
        a.setNumPulses(numPulses)
    }
}

extension SavedInputs.Detection {
    init(from a: DetectionAdapter) {
        pd           = a.defaultPd
        pfaExponent  = a.defaultPfaExponent
        numPulses    = a.defaultNumPulses
        swerlingCase = a.defaultSwerlingCase
        beamwidth    = a.defaultBeamwidth
        scanRate     = a.defaultScanRate
        prf          = a.defaultPrf
        bandwidth    = a.defaultBandwidth
    }
    func apply(to a: DetectionAdapter) {
        a.setPd(pd)
        a.setPfaExponent(pfaExponent)
        a.setNumPulses(numPulses)
        a.setSwerlingCase(swerlingCase)
        a.setBeamwidth(beamwidth)
        a.setScanRate(scanRate)
        a.setPrf(prf)
        a.setBandwidth(bandwidth)
    }
}

extension SavedInputs.Doppler {
    init(from a: DopplerAdapter) {
        frequency   = a.defaultFrequency
        radialSpeed = a.defaultRadialSpeed
        prf         = a.defaultPrf
        bandwidth   = a.defaultBandwidth
        targetRange = a.defaultTargetRange
        beamwidthAz = a.defaultBeamwidthAz
        beamwidthEl = a.defaultBeamwidthEl
    }
    func apply(to a: DopplerAdapter) {
        a.setFrequency(frequency)
        a.setRadialSpeed(radialSpeed)
        a.setPrf(prf)
        a.setBandwidth(bandwidth)
        a.setTargetRange(targetRange)
        a.setBeamwidthAz(beamwidthAz)
        a.setBeamwidthEl(beamwidthEl)
    }
}

extension SavedInputs.Digital {
    init(from a: DigitalAdapter) {
        snr                = a.defaultSnr
        bandwidth          = a.defaultBandwidth
        dataRate           = a.defaultDataRate
        chipRate           = a.defaultChipRate
        requiredEbNo       = a.defaultRequiredEbNo
        implementationLoss = a.defaultImplementationLoss
    }
    func apply(to a: DigitalAdapter) {
        a.setSnr(snr)
        a.setBandwidth(bandwidth)
        a.setDataRate(dataRate)
        a.setChipRate(chipRate)
        a.setRequiredEbNo(requiredEbNo)
        a.setImplementationLoss(implementationLoss)
    }
}

extension SavedInputs.Antenna {
    init(from a: AntennaAdapter) {
        gain        = a.defaultGain
        azBeamwidth = a.defaultAzBeamwidth
        elBeamwidth = a.defaultElBeamwidth
        txPower     = a.defaultTxPower
        frequency   = a.defaultFrequency
    }
    func apply(to a: AntennaAdapter) {
        a.setGain(gain)
        a.setAzBeamwidth(azBeamwidth)
        a.setElBeamwidth(elBeamwidth)
        a.setTxPower(txPower)
        a.setFrequency(frequency)
    }
}

// ── Whole-store capture/apply ────────────────────────────────────────────────

extension SavedInputs {
    init(capturing store: EwCalcStore) {
        formatVersion = Self.currentFormatVersion
        propagation   = .init(from: store.propagation)
        link          = .init(from: store.link)
        receiver      = .init(from: store.receiver)
        jamming       = .init(from: store.jamming)
        location      = .init(from: store.location)
        radar         = .init(from: store.radar)
        detection     = .init(from: store.detection)
        doppler       = .init(from: store.doppler)
        digital       = .init(from: store.digital)
        antenna       = .init(from: store.antenna)
    }

    func apply(to store: EwCalcStore) {
        propagation.apply(to: store.propagation)
        link.apply(to: store.link)
        receiver.apply(to: store.receiver)
        jamming.apply(to: store.jamming)
        location.apply(to: store.location)
        radar.apply(to: store.radar)
        detection?.apply(to: store.detection)
        doppler?.apply(to: store.doppler)
        digital.apply(to: store.digital)
        antenna.apply(to: store.antenna)
    }
}
