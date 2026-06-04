// PropagationAdapter.swift
import Foundation

final class PropagationAdapter: ObservableObject {
    private let ref: EwpPropagationRef
    @Published private(set) var output: EwpPropagationOutput

    // Static C-compatible callback; captures nothing from Swift's closure system.
    private static let _cb: EwpPropagationCallback = { out, ctx in
        guard let ctx else { return }
        Unmanaged<PropagationAdapter>.fromOpaque(ctx).takeUnretainedValue().output = out
    }

    init() {
        ref    = ewp_propagation_create()
        output = ewp_propagation_output(ref)
        ewp_propagation_set_callback(ref, Self._cb, Unmanaged.passUnretained(self).toOpaque())
    }

    deinit { ewp_propagation_destroy(ref) }

    func setDistance(_ km: Double)             { ewp_propagation_set_distance(ref, km) }
    func setFrequency(_ mhz: Double)           { ewp_propagation_set_frequency(ref, mhz) }
    func setTxHeight(_ m: Double)              { ewp_propagation_set_tx_height(ref, m) }
    func setRxHeight(_ m: Double)              { ewp_propagation_set_rx_height(ref, m) }
    func setObstructionHeight(_ m: Double)     { ewp_propagation_set_obstruction_height(ref, m) }

    var defaultDistance:          Double { ewp_propagation_distance(ref) }
    var defaultFrequency:         Double { ewp_propagation_frequency(ref) }
    var defaultTxHeight:          Double { ewp_propagation_tx_height(ref) }
    var defaultRxHeight:          Double { ewp_propagation_rx_height(ref) }
    var defaultObstructionHeight: Double { ewp_propagation_obstruction_height(ref) }
}
