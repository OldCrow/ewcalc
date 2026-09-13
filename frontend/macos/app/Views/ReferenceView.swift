// ReferenceView.swift
import SwiftUI

// ── Bridge-backed data model (#73) ────────────────────────────────────────────
// All reference content lives in ewpresenter::refdata (single source of
// truth), exposed through the ewp_ref_* C API. This view renders that data;
// it owns no content of its own.

private enum RefRow {
    case value(label: String, value: String, copyValue: String?)
    /// Typeset formula: standard and log forms as bundled PNG snippets
    /// ("<svgBase>-std"/"-log", 2x renders), with Unicode text used as
    /// accessibility label, tooltip, and copy text.
    case formula(name: String, svgBase: String, stdText: String, logText: String?)
}

private struct RefSection {
    let title: String
    let diagram: String?   ///< Optional #72 diagram base name (bundled PNG).
    let rows: [RefRow]
    /// Shared standard-form column width (pt): the widest standard form in
    /// this section, capped at 400 so one long master cannot blow the
    /// column open. 0 when the section has no formula rows. Aligning the
    /// forms per section is the cross-platform column model (Known Gaps —
    /// fixed on WinUI/Linux first, same Grid-per-section idea).
    let stdColWidth: CGFloat
}

/// Natural 1x display width of a bundled 2x formula PNG; 0 if missing.
private func formulaNaturalWidth(_ name: String) -> CGFloat {
    guard let path = Bundle.main.path(forResource: name, ofType: "png"),
          let image = NSImage(byReferencingFile: path) else { return 0 }
    return image.size.width / 2
}

private func str(_ ptr: UnsafePointer<CChar>?) -> String? {
    ptr.map { String(cString: $0) }
}

/// Loads one reference page from the bridge (static data; loaded once).
private func loadSections(page: Int) -> [RefSection] {
    var sections: [RefSection] = []
    for sec in 0..<ewp_ref_section_count(page) {
        var rows: [RefRow] = []
        for row in 0..<ewp_ref_row_count(page, sec) {
            guard let label = str(ewp_ref_row_label(page, sec, row)),
                  let value = str(ewp_ref_row_value(page, sec, row)) else { continue }
            if ewp_ref_row_kind(page, sec, row) == EWP_REF_ROW_FORMULA,
               let svgBase = str(ewp_ref_row_svg_base(page, sec, row)) {
                rows.append(.formula(name: label,
                                     svgBase: svgBase,
                                     stdText: value,
                                     logText: str(ewp_ref_row_log_value(page, sec, row))))
            } else {
                rows.append(.value(label: label,
                                   value: value,
                                   copyValue: str(ewp_ref_row_copy_value(page, sec, row))))
            }
        }
        let stdWidths: [CGFloat] = rows.compactMap {
            if case let .formula(_, svgBase, _, _) = $0 {
                return formulaNaturalWidth("\(svgBase)-std")
            }
            return nil
        }
        sections.append(RefSection(title: str(ewp_ref_section_title(page, sec)) ?? "",
                                   diagram: str(ewp_ref_section_diagram(page, sec)),
                                   rows: rows,
                                   stdColWidth: min(stdWidths.max() ?? 0, 400)))
    }
    return sections
}

// ── Row views ─────────────────────────────────────────────────────────────────

/// Small copy-to-clipboard button flashing a green checkmark on click.
private struct CopyButton: View {
    let text: String
    let subject: String
    @State private var copied = false

    var body: some View {
        Button {
            NSPasteboard.general.clearContents()
            NSPasteboard.general.setString(text, forType: .string)
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
        .help("Copy \(text)")
        .accessibilityLabel(copied ? "Copied \(text)" : "Copy \(text) for \(subject)")
    }
}

/// A value row with an optional copy button. Rows without a copy value show
/// a fixed-width placeholder so value text stays right-aligned in all rows.
private struct ValueRow: View {
    let label: String
    let value: String
    let copyValue: String?

    var body: some View {
        LabeledContent(label) {
            HStack(spacing: 4) {
                Text(value)
                    .monospaced()
                    .frame(maxWidth: .infinity, alignment: .trailing)
                if let cv = copyValue {
                    CopyButton(text: cv, subject: label)
                } else {
                    Color.clear.frame(width: 22)
                }
            }
            .frame(height: 16)
        }
    }
}

/// One typeset formula snippet loaded from the bundle (2x PNG, shown at
/// half its pixel size). The Unicode text carries selectability: it is the
/// accessibility label, the tooltip, and what the copy button copies.
private struct FormulaImage: View {
    let name: String     ///< Bundle resource base name, e.g. "fspl-std".
    let text: String     ///< Unicode equivalent (alt text / tooltip / copy).
    var widthCap: CGFloat?  ///< Extra cap (e.g. the section column).

    var body: some View {
        if let path = Bundle.main.path(forResource: name, ofType: "png"),
           let nsImage = NSImage(byReferencingFile: path) {
            // Natural size is 1x (2x asset at half pixel size) so glyphs
            // match across formulas; maxWidth + scaledToFit lets a WIDE
            // master (e.g. the stand-off J/S line) shrink when the pane —
            // or the section's shared column — is narrower than natural.
            // Never upscale.
            let natural = nsImage.size.width / 2
            let maxW = min(natural, widthCap ?? natural)
            Image(nsImage: nsImage)
                .resizable()
                .scaledToFit()
                .frame(maxWidth: maxW,
                       maxHeight: nsImage.size.height / 2)
                .accessibilityLabel(text)
                .help(text)
        } else {
            // Asset missing — fall back to the Unicode text itself.
            Text(text).monospaced()
        }
    }
}

/// A formula row: name, then standard and log forms side by side in columns
/// (log column absent when the formula has no log form).
private struct FormulaRow: View {
    let name: String
    let svgBase: String
    let stdText: String
    let logText: String?
    let stdColWidth: CGFloat

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            HStack(spacing: 4) {
                Text(name)
                Spacer()
                CopyButton(text: copyText, subject: name)
            }
            // Column model (parity with WinUI/Linux): every standard form
            // in a section occupies the same leading column — sized to the
            // section's widest, capped 400 — so all log forms land on one
            // shared left edge instead of starting wherever each standard
            // form ends. Falls back to stacked forms when the pane is too
            // narrow for the columns.
            ViewThatFits(in: .horizontal) {
                HStack(alignment: .center, spacing: 24) {
                    FormulaImage(name: "\(svgBase)-std", text: stdText,
                                 widthCap: stdColWidth)
                        .frame(width: stdColWidth, alignment: .leading)
                    if let logText {
                        Divider()
                        FormulaImage(name: "\(svgBase)-log", text: logText, widthCap: nil)
                    }
                    Spacer(minLength: 0)
                }
                VStack(alignment: .leading, spacing: 8) {
                    FormulaImage(name: "\(svgBase)-std", text: stdText, widthCap: nil)
                    if let logText {
                        FormulaImage(name: "\(svgBase)-log", text: logText, widthCap: nil)
                    }
                }
            }
        }
        .padding(.vertical, 2)
    }

    private var copyText: String {
        logText.map { "\(stdText)   |   \($0)" } ?? stdText
    }
}

// ── ReferenceView ─────────────────────────────────────────────────────────────

struct ReferenceView: View {
    private let title: String
    private let sections: [RefSection]

    init(pageIndex: Int) {
        title = ewp_ref_page_title(pageIndex).map { String(cString: $0) } ?? "Reference"
        sections = loadSections(page: pageIndex)
    }

    var body: some View {
        Form {
            ForEach(sections, id: \.title) { section in
                Section(section.title) {
                    if let diagram = section.diagram {
                        SectionDiagram(name: diagram)
                    }
                    ForEach(Array(section.rows.enumerated()), id: \.offset) { _, row in
                        switch row {
                        case let .value(label, value, copyValue):
                            ValueRow(label: label, value: value, copyValue: copyValue)
                        case let .formula(name, svgBase, stdText, logText):
                            FormulaRow(name: name, svgBase: svgBase,
                                       stdText: stdText, logText: logText,
                                       stdColWidth: section.stdColWidth)
                        }
                    }
                }
            }
        }
        .formStyle(.grouped)
        .navigationTitle(title)
    }
}

/// A section's geometry thumbnail: a #72 diagram PNG (2x render, bundled
/// for the calculator panes) shown inline at reduced size.
private struct SectionDiagram: View {
    let name: String

    var body: some View {
        if let path = Bundle.main.path(forResource: name, ofType: "png"),
           let nsImage = NSImage(byReferencingFile: path) {
            Image(nsImage: nsImage)
                .resizable()
                .scaledToFit()
                .frame(maxWidth: 480)
                .frame(maxWidth: .infinity, alignment: .center)
                .accessibilityLabel(name.replacingOccurrences(of: "-", with: " "))
        }
    }
}
