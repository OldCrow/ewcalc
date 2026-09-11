#pragma once
// RefData.h
//
// Static-content wrapper (#73): exposes ewpresenter::refdata's pages ->
// sections -> rows to C# as plain managed value types. Unlike the other
// adapters in this project, refdata carries no state and fires no change
// events, so there is no handle/create/destroy lifecycle here — just one
// static method that snapshots the whole static structure into managed
// arrays.
#include "MarshalHelper.h"
#include "ewpresenter/reference_data.h"

namespace EwPresenterNet {

/// Kind of a reference row. Numeric values match ewpresenter::refdata::RowKind
/// (and the C bridge's EwpRefRowKind) exactly.
public enum class RefRowKind : System::Byte {
    Value   = 0,
    Formula = 1,
};

/// One reference row. A null field means "not present" — see
/// ewpresenter/reference_data.h for the per-field rules (copy_value is null
/// for value rows with range display text and always null for formula rows;
/// log_value/svg_base are null except on formula rows).
public value struct RefRow {
    RefRowKind       RowKind;
    System::String^ Label;
    System::String^ Value;
    System::String^ CopyValue;
    System::String^ LogValue;
    System::String^ SvgBase;
};

public value struct RefSection {
    System::String^ Title;
    array<RefRow>^  Rows;
    System::String^ Diagram; ///< Optional #72 diagram base name (assets/diagrams/png);
                             ///  null when the section has no diagram.
};

public value struct RefPage {
    System::String^    Id;
    System::String^    Title;
    System::String^    Subtitle;
    array<RefSection>^ Sections;
};

/// Static reference-library content (#73), snapshotted from
/// ewpresenter::refdata::pages() on every call.
public ref class RefData sealed abstract {
public:
    static array<RefPage>^ GetPages();
};

} // namespace EwPresenterNet
