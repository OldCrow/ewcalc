// ewcalcApp.swift
import SwiftUI

@main
struct ewcalcApp: App {
    @StateObject private var store = EwCalcStore()

    var body: some Scene {
        WindowGroup("EW Calculator") {
            ContentView()
                .environmentObject(store)
        }
        .defaultSize(width: 960, height: 680)
    }
}
