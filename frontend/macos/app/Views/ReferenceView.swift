// ReferenceView.swift
import SwiftUI

// ── Data model ────────────────────────────────────────────────────────────────

private struct RefEntry {
    let label: String
    let value: String       ///< Full display string (annotated)
    let copyValue: String?  ///< Numeric value only; nil = no copy button
}

private struct RefSection {
    let title: String
    let entries: [RefEntry]
}

// Helper for terse construction
private func entry(_ label: String, _ value: String, copy cv: String? = nil) -> RefEntry {
    RefEntry(label: label, value: value, copyValue: cv)
}

private let referenceData: [RefSection] = [

    // Gain values: copy just the number (dBi) for direct field entry
    RefSection(title: "Antenna Gain", entries: [
        entry("Isotropic (reference)",               "0.0 dBi",              copy: "0.0"),
        entry("Short whip / rubber duck",            "0 dBi  (typical)",     copy: "0"),
        entry("Quarter-wave whip, ground-plane mount","2 dBi  (typical)",    copy: "2"),
        entry("Half-wave dipole",                    "2.15 dBi  (0 dBd)",   copy: "2.15"),
        entry("2-element Yagi",                      "≈ 7 dBi",              copy: "7"),
        entry("3-element Yagi",                      "≈ 8.5 dBi",            copy: "8.5"),
        entry("5-element Yagi",                      "≈ 10.5 dBi",           copy: "10.5"),
        entry("10-element Yagi",                     "≈ 14 dBi",             copy: "14"),
    ]),

    // Sidelobe levels are dBc offsets relative to main lobe;
    // no copy button for ranges (no single right value)
    RefSection(title: "Antenna Sidelobe Levels (re main lobe)", entries: [
        entry("Uniform aperture — 1st SLL",          "−13 dBc",             copy: "-13"),
        entry("Taylor weighted — 1st SLL",           "−25 dBc",             copy: "-25"),
        entry("Low-sidelobe array — 1st SLL",        "−35 dBc",             copy: "-35"),
        entry("Typical back lobe",                   "−25 to −35 dBc"),
    ]),

    // Thermal noise: copy the numeric dBm value
    RefSection(title: "Thermal Noise Floor  (kT, 290 K)", entries: [
        entry("1 Hz bandwidth",                      "−174.0 dBm",           copy: "-174.0"),
        entry("1 kHz bandwidth",                     "−144.0 dBm",           copy: "-144.0"),
        entry("1 MHz bandwidth",                     "−114.0 dBm",           copy: "-114.0"),
        entry("10 MHz bandwidth",                    "−104.0 dBm",           copy: "-104.0"),
        entry("100 MHz bandwidth",                   "−94.0 dBm",            copy: "-94.0"),
        entry("1 GHz bandwidth",                     "−84.0 dBm",            copy: "-84.0"),
    ]),

    // RCS: wide ranges get no copy button; specific targets get a representative value
    RefSection(title: "Radar Cross Section (Typical)", entries: [
        entry("Large aircraft (broadside)",          "+15 dBsm  (rep.)",    copy: "15"),
        entry("Fighter (broadside)",                 "+7 dBsm  (rep.)",     copy: "7"),
        entry("Fighter (nose-on, conventional)",     "0 dBsm  (≈ 1 m²)",    copy: "0"),
        entry("LO fighter (nose-on)",                "−15 dBsm  (rep.)",    copy: "-15"),
        entry("Cruise missile",                      "−7 dBsm  (rep.)",     copy: "-7"),
        entry("Bird",                                "−15 dBsm  (rep.)",    copy: "-15"),
        entry("Ship (small, ∼1 000 t)",             "+25 dBsm  (rep.)",    copy: "25"),
        entry("Ship (large, >10 000 t)",             "+45 dBsm  (rep.)",    copy: "45"),
    ]),

    // Eb/N₀: all single values, copy just the number
    RefSection(title: "Eb/N₀ Requirements (AWGN)", entries: [
        entry("BPSK / QPSK,  BER 10⁻³",            "6.8 dB",               copy: "6.8"),
        entry("BPSK / QPSK,  BER 10⁻⁵",            "9.6 dB",               copy: "9.6"),
        entry("BPSK / QPSK,  BER 10⁻⁶",            "10.5 dB",              copy: "10.5"),
        entry("Non-coh. FSK, BER 10⁻³",            "≈ 13.5 dB",            copy: "13.5"),
        entry("Non-coh. FSK, BER 10⁻⁵",            "≈ 17.0 dB",            copy: "17.0"),
    ]),
]

// ── RefRow ────────────────────────────────────────────────────────────────────

/// A reference row with an optional copy-to-clipboard button.
/// Rows with a nil copyValue show no button (value is a range or annotation).
/// Rows with a copyValue show a clipboard icon; clicking it copies only the
/// numeric string and flashes a green checkmark for confirmation.
private struct RefRow: View {
    let entry: RefEntry
    @State private var copied = false

    var body: some View {
        LabeledContent(entry.label) {
            HStack(spacing: 4) {
                Text(entry.value)
                    .monospaced()
                    .frame(maxWidth: .infinity, alignment: .trailing)
                if let cv = entry.copyValue {
                    Button {
                        NSPasteboard.general.clearContents()
                        NSPasteboard.general.setString(cv, forType: .string)
                        copied = true
                        DispatchQueue.main.asyncAfter(deadline: .now() + 1.5) {
                            copied = false
                        }
                    } label: {
                        Image(systemName: copied ? "checkmark.circle.fill" : "doc.on.doc")
                            .foregroundStyle(copied ? .green : .secondary)
                            .imageScale(.small)
                    }
                    .buttonStyle(.borderless)
                    .frame(width: 22)
                    .help("Copy \(cv)")
                } else {
                    // Fixed-width placeholder — matches the copy button width so
                    // value text stays right-aligned in all rows
                    Color.clear.frame(width: 22)
                }
            }
            .frame(height: 16) // consistent row height
        }
    }
}

// ── ReferenceView ─────────────────────────────────────────────────────────────

struct ReferenceView: View {
    var body: some View {
        Form {
            ForEach(referenceData, id: \.title) { section in
                Section(section.title) {
                    ForEach(section.entries, id: \.label) { entry in
                        RefRow(entry: entry)
                    }
                }
            }
        }
        .formStyle(.grouped)
        .navigationTitle("Reference")
    }
}
