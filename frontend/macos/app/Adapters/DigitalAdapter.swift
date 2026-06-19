// DigitalAdapter.swift
import Foundation

final class DigitalAdapter: ObservableObject {
    private let ref: EwpDigitalRef
    @Published private(set) var output: EwpDigitalOutput

    private static let _cb: EwpDigitalCallback = { out, ctx in
        guard let ctx else { return }
        Unmanaged<DigitalAdapter>.fromOpaque(ctx).takeUnretainedValue().output = out
    }

    init() {
        ref    = ewp_digital_create()
        output = ewp_digital_output(ref)
        // passUnretained: safe only because all adapters are `let` properties of
        // EwCalcStore (@StateObject owned by ewcalcApp) and live until process exit.
        // Do not move adapters to shorter-lived objects without switching to
        // passRetained/takeRetainedValue + clearing the callback in deinit.
        ewp_digital_set_callback(ref, Self._cb, Unmanaged.passUnretained(self).toOpaque())
    }

    deinit { ewp_digital_destroy(ref) }

    func setSnr(_ db: Double)                { ewp_digital_set_snr(ref, db) }
    func setBandwidth(_ mhz: Double)         { ewp_digital_set_bandwidth(ref, mhz) }
    func setDataRate(_ mbps: Double)         { ewp_digital_set_data_rate(ref, mbps) }
    func setChipRate(_ mcps: Double)         { ewp_digital_set_chip_rate(ref, mcps) }
    func setRequiredEbNo(_ db: Double)       { ewp_digital_set_required_eb_no(ref, db) }
    func setImplementationLoss(_ db: Double) { ewp_digital_set_implementation_loss(ref, db) }

    var defaultSnr:                Double { ewp_digital_snr(ref) }
    var defaultBandwidth:          Double { ewp_digital_bandwidth(ref) }
    var defaultDataRate:           Double { ewp_digital_data_rate(ref) }
    var defaultChipRate:           Double { ewp_digital_chip_rate(ref) }
    var defaultRequiredEbNo:       Double { ewp_digital_required_eb_no(ref) }
    var defaultImplementationLoss: Double { ewp_digital_implementation_loss(ref) }
}
