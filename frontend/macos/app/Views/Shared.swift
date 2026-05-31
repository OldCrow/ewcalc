// Shared.swift
import SwiftUI

// ── C-string helper ───────────────────────────────────────────────────────────

/// Converts a fixed-size C char array (imported as a Swift tuple) to a String.
/// Uses the bytes up to the first null terminator.
func cStr<T>(_ tuple: T) -> String {
    withUnsafeBytes(of: tuple) { buf in
        let bytes = buf.prefix(while: { $0 != 0 })
        return String(bytes: bytes, encoding: .utf8) ?? ""
    }
}

// ── InputRow ──────────────────────────────────────────────────────────────────

/// A LabeledContent row combining a TextField and a Stepper.
/// Calls `onChange` whenever the value is edited.  Uses the macOS 13-compatible
/// single-parameter `.onChange` variant.
struct InputRow: View {
    let label:    String
    let unit:     String
    @Binding var value: Double
    let range:    ClosedRange<Double>
    let step:     Double
    let decimals: Int
    let onChange: (Double) -> Void

    init(
        _ label: String,
        unit: String = "",
        value: Binding<Double>,
        in range: ClosedRange<Double>,
        step: Double = 1.0,
        decimals: Int = 1,
        onChange: @escaping (Double) -> Void
    ) {
        self.label    = label
        self.unit     = unit
        _value        = value
        self.range    = range
        self.step     = step
        self.decimals = decimals
        self.onChange = onChange
    }

    var body: some View {
        LabeledContent(label) {
            HStack(spacing: 4) {
                TextField(
                    "",
                    value: $value,
                    format: .number.precision(.fractionLength(decimals))
                )
                .frame(width: 100)
                .textFieldStyle(.roundedBorder)
                .multilineTextAlignment(.trailing)

                Stepper("", value: $value, in: range, step: step)
                    .labelsHidden()

                // Always render the unit slot so all rows align regardless
                // of whether they carry a unit label or not.
                Text(unit)
                    .foregroundStyle(.secondary)
                    .frame(width: 38, alignment: .leading)
            }
        }
        .onChange(of: value) { onChange($0) }
    }
}

// ── ResultRow ─────────────────────────────────────────────────────────────────

/// A read-only LabeledContent row showing a formatted result.
struct ResultRow: View {
    let label: String
    let value: String

    init(_ label: String, _ value: String) {
        self.label = label
        self.value = value
    }

    var body: some View {
        LabeledContent(label) {
            Text(value.isEmpty ? "\u{2014}" : value) // em-dash for empty
                .textSelection(.enabled)
                .foregroundStyle(value.isEmpty ? .secondary : .primary)
                .frame(maxWidth: .infinity, alignment: .trailing)
                .monospaced()
        }
    }
}
