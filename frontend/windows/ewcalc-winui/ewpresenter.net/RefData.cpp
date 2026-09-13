// RefData.cpp
// C4679: static virtual interface members (IDivisionOperators etc. on System::IntPtr)
// are not importable by C++/CLI; the affected interfaces are unused here.
// Still required for CI, which builds on windows-2022 (VS 17.x). Verified
// 2026-09-09 that VS 18 2026 (MSVC 19.51) no longer emits it — drop this
// pragma once the CI runner moves off windows-2022.
#pragma warning(disable: 4679)
#include "RefData.h"

namespace EwPresenterNet {

static RefRow ToManagedRow(const ewpresenter::refdata::Row& r) {
    RefRow row{};
    row.RowKind   = static_cast<RefRowKind>(r.kind);
    row.Label     = ToManaged(r.label);
    row.Value     = ToManaged(r.value);
    row.CopyValue = ToManaged(r.copy_value);
    row.LogValue  = ToManaged(r.log_value);
    row.SvgBase   = ToManaged(r.svg_base);
    return row;
}

static RefSection ToManagedSection(const ewpresenter::refdata::Section& s) {
    RefSection section{};
    section.Title = ToManaged(s.title);
    auto rows = gcnew array<RefRow>(static_cast<int>(s.row_count));
    for (std::size_t i = 0; i < s.row_count; ++i)
        rows[static_cast<int>(i)] = ToManagedRow(s.rows[i]);
    section.Rows = rows;
    section.Diagram = ToManaged(s.diagram);
    return section;
}

static RefPage ToManagedPage(const ewpresenter::refdata::Page& p) {
    RefPage page{};
    page.Id       = ToManaged(p.id);
    page.Title    = ToManaged(p.title);
    page.Subtitle = ToManaged(p.subtitle);
    auto sections = gcnew array<RefSection>(static_cast<int>(p.section_count));
    for (std::size_t i = 0; i < p.section_count; ++i)
        sections[static_cast<int>(i)] = ToManagedSection(p.sections[i]);
    page.Sections = sections;
    return page;
}

array<RefPage>^ RefData::GetPages() {
    auto pages  = ewpresenter::refdata::pages();
    auto result = gcnew array<RefPage>(static_cast<int>(pages.size()));
    for (std::size_t i = 0; i < pages.size(); ++i)
        result[static_cast<int>(i)] = ToManagedPage(pages[i]);
    return result;
}

} // namespace EwPresenterNet
