// EwCalcStore.swift
import Foundation

/// Top-level store injected as an environment object.
/// Owns all six presenter adapters for the lifetime of the app.
final class EwCalcStore: ObservableObject {
    let propagation = PropagationAdapter()
    let link        = LinkAdapter()
    let receiver    = ReceiverAdapter()
    let jamming     = JammingAdapter()
    let location    = LocationAdapter()
    let radar       = RadarAdapter()
}
