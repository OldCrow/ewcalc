// LinkAdapter.swift
import Foundation

final class LinkAdapter: ObservableObject {
    private let ref: EwpLinkRef
    @Published private(set) var output: EwpLinkOutput

    private static let _cb: EwpLinkCallback = { out, ctx in
        guard let ctx else { return }
        Unmanaged<LinkAdapter>.fromOpaque(ctx).takeUnretainedValue().output = out
    }

    init() {
        ref    = ewp_link_create()
        output = ewp_link_output(ref)
        ewp_link_set_callback(ref, Self._cb, Unmanaged.passUnretained(self).toOpaque())
    }

    deinit { ewp_link_destroy(ref) }

    func setTxPower(_ dbm: Double)       { ewp_link_set_tx_power(ref, dbm) }
    func setTxGain(_ db: Double)          { ewp_link_set_tx_gain(ref, db) }
    func setRxGain(_ db: Double)          { ewp_link_set_rx_gain(ref, db) }
    func setDistance(_ km: Double)        { ewp_link_set_distance(ref, km) }
    func setTxHeight(_ m: Double)         { ewp_link_set_tx_height(ref, m) }
    func setRxHeight(_ m: Double)         { ewp_link_set_rx_height(ref, m) }
    func setFrequency(_ mhz: Double)      { ewp_link_set_frequency(ref, mhz) }
    func setRxSensitivity(_ dbm: Double)  { ewp_link_set_rx_sensitivity(ref, dbm) }

    var defaultTxPower:       Double { ewp_link_tx_power(ref) }
    var defaultTxGain:        Double { ewp_link_tx_gain(ref) }
    var defaultRxGain:        Double { ewp_link_rx_gain(ref) }
    var defaultDistance:      Double { ewp_link_distance(ref) }
    var defaultTxHeight:      Double { ewp_link_tx_height(ref) }
    var defaultRxHeight:      Double { ewp_link_rx_height(ref) }
    var defaultFrequency:     Double { ewp_link_frequency(ref) }
    var defaultRxSensitivity: Double { ewp_link_rx_sensitivity(ref) }
}
