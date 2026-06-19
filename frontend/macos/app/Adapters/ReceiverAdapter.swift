// ReceiverAdapter.swift
import Foundation

final class ReceiverAdapter: ObservableObject {
    private let ref: EwpReceiverRef
    @Published private(set) var output: EwpReceiverOutput

    private static let _cb: EwpReceiverCallback = { out, ctx in
        guard let ctx else { return }
        Unmanaged<ReceiverAdapter>.fromOpaque(ctx).takeUnretainedValue().output = out
    }

    init() {
        ref    = ewp_receiver_create()
        output = ewp_receiver_output(ref)
        // passUnretained: safe only because all adapters are `let` properties of
        // EwCalcStore (@StateObject owned by ewcalcApp) and live until process exit.
        // Do not move adapters to shorter-lived objects without switching to
        // passRetained/takeRetainedValue + clearing the callback in deinit.
        ewp_receiver_set_callback(ref, Self._cb, Unmanaged.passUnretained(self).toOpaque())
    }

    deinit { ewp_receiver_destroy(ref) }

    func setBandwidth(_ mhz: Double)      { ewp_receiver_set_bandwidth(ref, mhz) }
    func setNoiseFigure(_ db: Double)     { ewp_receiver_set_noise_figure(ref, db) }
    func setRequiredSnr(_ db: Double)     { ewp_receiver_set_required_snr(ref, db) }
    func setSecondOrderIp(_ dbm: Double)  { ewp_receiver_set_second_order_ip(ref, dbm) }
    func setThirdOrderIp(_ dbm: Double)   { ewp_receiver_set_third_order_ip(ref, dbm) }
    func setAdcBits(_ bits: Int)          { ewp_receiver_set_adc_bits(ref, Int32(bits)) }

    func setStages(_ stages: [EwpStageInput]) {
        stages.withUnsafeBufferPointer { buf in
            ewp_receiver_set_stages(ref, buf.baseAddress, Int32(buf.count))
        }
    }

    var defaultBandwidth:    Double { ewp_receiver_bandwidth(ref) }
    var defaultNoiseFigure:  Double { ewp_receiver_noise_figure(ref) }
    var defaultRequiredSnr:  Double { ewp_receiver_required_snr(ref) }
    var defaultSecondOrder:  Double { ewp_receiver_second_order_ip(ref) }
    var defaultThirdOrder:   Double { ewp_receiver_third_order_ip(ref) }
    var defaultAdcBits:      Int    { Int(ewp_receiver_adc_bits(ref)) }

    var defaultStages: [EwpStageInput] {
        (0..<Int(ewp_receiver_stage_count(ref))).map { ewp_receiver_stage(ref, Int32($0)) }
    }
}
