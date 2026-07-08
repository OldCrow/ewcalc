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

// ── EwpFieldError helpers (#16) ─────────────────────────────────────────────────

// EwpFieldError's exact Swift-imported case names depend on the Clang importer's
// enum-extensibility inference for the plain C enum in ewcalc_bridge.h. To stay
// resilient to that (struct-style vs. true-enum import), all comparisons here go
// through `rawValue` rather than named cases.
extension EwpFieldError {
    /// EWP_FIELD_OK == 0 (see ewcalc_bridge.h). The Clang importer generates a
    /// non-failable `init(rawValue:)` for this C enum (it imports as a
    /// RawRepresentable wrapper, not a case-checked Swift `enum`), so no `!` is
    /// needed here.
    static let ok = EwpFieldError(rawValue: 0)

    var isError: Bool { rawValue != 0 }

    /// Human-readable validation message, or "" when there is no error.
    var message: String {
        switch rawValue {
        case 0:  return ""
        case 1:  return "Value is below the minimum"
        case 2:  return "Value exceeds the maximum"
        case 3:  return "Value must be positive"
        case 4:  return "Value must not be negative"
        case 5:  return "Value must be a finite number"
        default: return "Invalid value"
        }
    }
}

// ── InputRow ──────────────────────────────────────────────────────────────────

/// A LabeledContent row combining a TextField and a Stepper.
/// Calls `onChange` whenever the value is edited.  Uses the macOS 13-compatible
/// single-parameter `.onChange` variant.
///
/// Out-of-range handling (#17): values are clamped to `range` as soon as they
/// are committed (on every `.onChange`), so a value typed outside the bounds
/// is silently snapped to the nearest bound before it ever reaches the
/// presenter. This was chosen over leaving out-of-range values in place with
/// only an inline error, since `range` already mirrors the presenter's hard
/// min/max for every field and there is no valid reason to let the field sit
/// out of bounds. The `error` parameter (from ewpresenter's per-field
/// `EwpFieldError`) is still surfaced visually for validation that clamping
/// cannot express, e.g. cross-field constraints such as semi-minor > semi-major
/// in LocationView, where there's no single "correct" value to snap to.
struct InputRow: View {
    let label:    String
    let unit:     String
    @Binding var value: Double
    let range:    ClosedRange<Double>
    let step:     Double
    let decimals: Int
    let error:    EwpFieldError
    let onChange: (Double) -> Void
    let help:     String?

    init(
        _ label: String,
        unit: String = "",
        value: Binding<Double>,
        in range: ClosedRange<Double>,
        step: Double = 1.0,
        decimals: Int = 1,
        error: EwpFieldError = .ok,
        help: String? = nil,
        onChange: @escaping (Double) -> Void
    ) {
        self.label    = label
        self.unit     = unit
        _value        = value
        self.range    = range
        self.step     = step
        self.decimals = decimals
        self.error    = error
        self.help     = help
        self.onChange = onChange
    }

    private var combinedHelp: String {
        guard error.isError else { return help ?? "" }
        let base = help ?? ""
        return base.isEmpty ? error.message : "\(base)\n⚠️ \(error.message)"
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
                .overlay(
                    RoundedRectangle(cornerRadius: 5)
                        .stroke(error.isError ? Color.red : Color.clear, lineWidth: 1.5)
                )
                .accessibilityLabel(unit.isEmpty ? label : "\(label), \(unit)")
                .accessibilityValue(error.isError ? "\(value.formatted()), \(error.message)" : value.formatted())

                Stepper("", value: $value, in: range, step: step)
                    .labelsHidden()
                    .accessibilityLabel("\(label) stepper")

                // Always render the unit slot so all rows align regardless
                // of whether they carry a unit label or not.
                Text(unit)
                    .foregroundStyle(.secondary)
                    .frame(width: 38, alignment: .leading)
                    .accessibilityHidden(true)

                if error.isError {
                    Image(systemName: "exclamationmark.triangle.fill")
                        .foregroundStyle(.red)
                        .imageScale(.small)
                        .accessibilityHidden(true)
                }
            }
        }
        .onChange(of: value) { newValue in
            let clamped = min(max(newValue, range.lowerBound), range.upperBound)
            if clamped != newValue { value = clamped }
            onChange(clamped)
        }
        .help(combinedHelp)
    }
}

// ── ResultRow ─────────────────────────────────────────────────────────────────

/// A read-only LabeledContent row showing a formatted result.
struct ResultRow: View {
    let label: String
    let value: String
    let help:  String?

    init(_ label: String, _ value: String, help: String? = nil) {
        self.label = label
        self.value = value
        self.help  = help
    }

    var body: some View {
        LabeledContent(label) {
            Text(value.isEmpty ? "\u{2014}" : value) // em-dash for empty
                .textSelection(.enabled)
                .foregroundStyle(value.isEmpty ? .secondary : .primary)
                .frame(maxWidth: .infinity, alignment: .trailing)
                .monospaced()
        }
        .help(help ?? "")
        // Read-only result: combine label + value into one VoiceOver stop
        // ("FSPL, 123.4 dB") rather than two separate, unfocusable elements.
        .accessibilityElement(children: .combine)
        .accessibilityLabel(label)
        .accessibilityValue(value.isEmpty ? "No result" : value)
    }
}

// ── CopyResultsButton (#21) ──────────────────────────────────────────────────────

/// Toolbar button that copies a calculator page's results to the general
/// pasteboard as plain text, one `Label: Value` line per result.
struct CopyResultsButton: View {
    let rows: [(label: String, value: String)]
    @State private var copied = false

    var body: some View {
        Button {
            let text = rows.map { "\($0.label): \($0.value)" }.joined(separator: "\n")
            NSPasteboard.general.clearContents()
            NSPasteboard.general.setString(text, forType: .string)
            copied = true
            DispatchQueue.main.asyncAfter(deadline: .now() + 1.5) { copied = false }
        } label: {
            Label(copied ? "Copied" : "Copy Results",
                  systemImage: copied ? "checkmark" : "doc.on.doc")
        }
        .help("Copy all results to the clipboard as plain text")
        .accessibilityLabel(copied ? "Results copied to clipboard" : "Copy results to clipboard")
    }
}
