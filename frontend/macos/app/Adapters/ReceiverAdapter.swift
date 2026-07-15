// ReceiverAdapter.swift
import Foundation

final class ReceiverAdapter: ObservableObject {
    private let ref: EwpReceiverRef
    @Published private(set) var output: EwpReceiverOutput

    // Per-field validation state (#16). stageNfError is a single aggregate
    // across the stage chain (the bridge exposes one accessor, not one per
    // stage), refreshed whenever the stage chain changes.
    @Published private(set) var bandwidthError:    EwpFieldError
    @Published private(set) var noiseFigureError:  EwpFieldError
    @Published private(set) var requiredSnrError:  EwpFieldError
    @Published private(set) var secondOrderError:  EwpFieldError
    @Published private(set) var thirdOrderError:   EwpFieldError
    @Published private(set) var adcBitsError:      EwpFieldError
    @Published private(set) var stageNfError:      EwpFieldError

    private static let _cb: EwpReceiverCallback = { out, ctx in
        guard let ctx else { return }
        Unmanaged<ReceiverAdapter>.fromOpaque(ctx).takeUnretainedValue().output = out
    }

    init() {
        ref    = ewp_receiver_create()
        output = ewp_receiver_output(ref)
        bandwidthError   = ewp_receiver_bandwidth_error(ref)
        noiseFigureError = ewp_receiver_noise_figure_error(ref)
        requiredSnrError = ewp_receiver_required_snr_error(ref)
        secondOrderError = ewp_receiver_second_order_ip_error(ref)
        thirdOrderError  = ewp_receiver_third_order_ip_error(ref)
        adcBitsError     = ewp_receiver_adc_bits_error(ref)
        stageNfError     = ewp_receiver_stage_nf_error(ref)
        // passUnretained: safe only because all adapters are `let` properties of
        // EwCalcStore (@StateObject owned by ewcalcApp) and live until process exit.
        // Do not move adapters to shorter-lived objects without switching to
        // passRetained/takeRetainedValue + clearing the callback in deinit.
        ewp_receiver_set_callback(ref, Self._cb, Unmanaged.passUnretained(self).toOpaque())
    }

    deinit { ewp_receiver_destroy(ref) }

    func setBandwidth(_ mhz: Double) {
        ewp_receiver_set_bandwidth(ref, mhz)
        bandwidthError = ewp_receiver_bandwidth_error(ref)
    }
    func setNoiseFigure(_ db: Double) {
        ewp_receiver_set_noise_figure(ref, db)
        noiseFigureError = ewp_receiver_noise_figure_error(ref)
    }
    func setRequiredSnr(_ db: Double) {
        ewp_receiver_set_required_snr(ref, db)
        requiredSnrError = ewp_receiver_required_snr_error(ref)
    }
    func setSecondOrderIp(_ dbm: Double) {
        ewp_receiver_set_second_order_ip(ref, dbm)
        secondOrderError = ewp_receiver_second_order_ip_error(ref)
    }
    func setThirdOrderIp(_ dbm: Double) {
        ewp_receiver_set_third_order_ip(ref, dbm)
        thirdOrderError = ewp_receiver_third_order_ip_error(ref)
    }
    func setAdcBits(_ bits: Int) {
        ewp_receiver_set_adc_bits(ref, Int32(bits))
        adcBitsError = ewp_receiver_adc_bits_error(ref)
    }

    func setStages(_ stages: [EwpStageInput]) {
        stages.withUnsafeBufferPointer { buf in
            ewp_receiver_set_stages(ref, buf.baseAddress, Int32(buf.count))
        }
        stageNfError = ewp_receiver_stage_nf_error(ref)
    }

    var defaultBandwidth:    Double { ewp_receiver_bandwidth(ref) }
    var defaultNoiseFigure:  Double { ewp_receiver_noise_figure(ref) }
    var defaultRequiredSnr:  Double { ewp_receiver_required_snr(ref) }
    var defaultSecondOrder:  Double { ewp_receiver_second_order_ip(ref) }
    var defaultThirdOrder:   Double { ewp_receiver_third_order_ip(ref) }
    var defaultAdcBits:      Int { Int(ewp_receiver_adc_bits(ref)) }

    var defaultStages: [EwpStageInput] {
        (0..<Int(ewp_receiver_stage_count(ref))).map { ewp_receiver_stage(ref, Int32($0)) }
    }
}
