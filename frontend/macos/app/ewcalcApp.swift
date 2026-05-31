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
        .defaultSize(width: 720, height: 600)
    }
}
