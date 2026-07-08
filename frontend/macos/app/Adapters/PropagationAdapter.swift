// PropagationAdapter.swift
import Foundation

final class PropagationAdapter: ObservableObject {
    private let ref: EwpPropagationRef
    @Published private(set) var output: EwpPropagationOutput

    // Per-field validation state (#16), read from ewpresenter via the bridge's
    // EwpFieldError accessors immediately after each corresponding setter runs.
    @Published private(set) var distanceError:          EwpFieldError
    @Published private(set) var frequencyError:         EwpFieldError
    @Published private(set) var txHeightError:          EwpFieldError
    @Published private(set) var rxHeightError:          EwpFieldError
    @Published private(set) var obstructionHeightError: EwpFieldError

    // Static C-compatible callback; captures nothing from Swift's closure system.
    private static let _cb: EwpPropagationCallback = { out, ctx in
        guard let ctx else { return }
        Unmanaged<PropagationAdapter>.fromOpaque(ctx).takeUnretainedValue().output = out
    }

    init() {
        ref    = ewp_propagation_create()
        output = ewp_propagation_output(ref)
        distanceError          = ewp_propagation_distance_error(ref)
        frequencyError         = ewp_propagation_frequency_error(ref)
        txHeightError          = ewp_propagation_tx_height_error(ref)
        rxHeightError          = ewp_propagation_rx_height_error(ref)
        obstructionHeightError = ewp_propagation_obstruction_height_error(ref)
        // passUnretained: safe only because all adapters are `let` properties of
        // EwCalcStore (@StateObject owned by ewcalcApp) and live until process exit.
        // Do not move adapters to shorter-lived objects without switching to
        // passRetained/takeRetainedValue + clearing the callback in deinit.
        ewp_propagation_set_callback(ref, Self._cb, Unmanaged.passUnretained(self).toOpaque())
    }

    deinit { ewp_propagation_destroy(ref) }

    func setDistance(_ km: Double) {
        ewp_propagation_set_distance(ref, km)
        distanceError = ewp_propagation_distance_error(ref)
    }
    func setFrequency(_ mhz: Double) {
        ewp_propagation_set_frequency(ref, mhz)
        frequencyError = ewp_propagation_frequency_error(ref)
    }
    func setTxHeight(_ m: Double) {
        ewp_propagation_set_tx_height(ref, m)
        txHeightError = ewp_propagation_tx_height_error(ref)
    }
    func setRxHeight(_ m: Double) {
        ewp_propagation_set_rx_height(ref, m)
        rxHeightError = ewp_propagation_rx_height_error(ref)
    }
    func setObstructionHeight(_ m: Double) {
        ewp_propagation_set_obstruction_height(ref, m)
        obstructionHeightError = ewp_propagation_obstruction_height_error(ref)
    }

    var defaultDistance:          Double { ewp_propagation_distance(ref) }
    var defaultFrequency:         Double { ewp_propagation_frequency(ref) }
    var defaultTxHeight:          Double { ewp_propagation_tx_height(ref) }
    var defaultRxHeight:          Double { ewp_propagation_rx_height(ref) }
    var defaultObstructionHeight: Double { ewp_propagation_obstruction_height(ref) }
}
