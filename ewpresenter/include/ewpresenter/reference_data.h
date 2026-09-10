#pragma once

/// @file reference_data.h
/// @brief Static reference-library content: single source of truth (#73).
///
/// All reference-page content — value tables, formula listings, and entry
/// metadata — lives here as static structures. Frontends render this data;
/// they never own reference content of their own. Exposed to Swift through
/// the C bridge (ewp_ref_*), and consumed directly by the Qt and WinUI
/// frontends via this header.
///
/// Formula rows follow the v1.2.0 rendering decision: each form (standard
/// and log) is a typeset SVG snippet from assets/formulas/ (rendered to PNG
/// by scripts/render-diagrams.sh, mid-gray strokes so one asset serves light
/// and dark themes), paired with a Unicode plain-text equivalent used as
/// accessibility label, tooltip, and copy-to-clipboard text.

#include <cstddef>
#include <cstdint>
#include <span>

namespace ewpresenter::refdata {

/// Kind of a reference row.
enum class RowKind : std::uint8_t {
    Value   = 0, ///< label + display value (+ optional copy value)
    Formula = 1  ///< name + typeset standard/log forms with Unicode alt text
};

/// One reference row. String pointers are static-lifetime UTF-8; a nullptr
/// field means "not present" (see per-field notes).
struct Row {
    RowKind     kind;
    const char* label;      ///< Entry label (Value) or formula name (Formula).
    const char* value;      ///< Display string (Value) or standard-form
                            ///  Unicode text (Formula: alt text + copy text).
    const char* copy_value; ///< Value rows: numeric string for the copy
                            ///  button; nullptr = no copy button (ranges).
                            ///  Formula rows: nullptr (copy uses value).
    const char* log_value;  ///< Formula rows: log-form Unicode text; nullptr
                            ///  when the formula has no log form. Value rows:
                            ///  nullptr.
    const char* svg_base;   ///< Formula rows: base name of the typeset asset
                            ///  pair "<svg_base>-std" / "<svg_base>-log"
                            ///  under assets/formulas/ (PNG in png/). The
                            ///  "-log" asset exists iff log_value is set.
                            ///  Value rows: nullptr.
};

/// A titled group of rows.
struct Section {
    const char* title;
    const Row*  rows;
    std::size_t row_count;
};

/// One reference page. `id` is a stable machine key (navigation, settings
/// persistence); `title`/`subtitle` are display strings.
struct Page {
    const char*    id;
    const char*    title;
    const char*    subtitle;
    const Section* sections;
    std::size_t    section_count;
};

/// All reference pages, in display order. Static lifetime.
[[nodiscard]] std::span<const Page> pages() noexcept;

} // namespace ewpresenter::refdata
