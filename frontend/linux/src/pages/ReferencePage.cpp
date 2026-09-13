// ReferencePage.cpp
//
// Renders one ewpresenter::refdata page (#73 single page; #74 multi-page):
// value rows, formula rows, and a section's optional #72 diagram thumbnail.
// This page owns no reference data of its own — it is a pure view over
// ewpresenter/include/ewpresenter/reference_data.h.
#include "ReferencePage.h"
#include "PageUtils.h"

#include <ewpresenter/reference_data.h>

#include <algorithm>
#include <cstddef>

#include <QClipboard>
#include <QFont>
#include <QGridLayout>
#include <QGroupBox>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSizePolicy>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

namespace refdata = ewpresenter::refdata;

namespace {

/// Escapes '&' for display in a QGroupBox title or a row label.
/// Both treat '&' as a mnemonic marker (addRow(QString, ...) creates a buddy
/// QLabel, which does the same), so refdata text such as "Receiver & Signal"
/// rendered as "Receiver _Signal" — the '&' swallowed and the following space
/// underlined. The data layer is shared with frontends that have no such
/// convention, so the escaping belongs here, at the Qt display boundary.
QString displayText(QString text)
{
    return text.replace(QLatin1Char('&'), QStringLiteral("&&"));
}

/// Every section is laid out as one grid so that the formula pairs line up
/// in columns (WinUI parity — 848ad92): the standard forms share a column
/// that sizes to the section's widest, which puts every log form in that
/// section on a common left edge. Value rows span the two form columns and
/// diagrams span all four, so a section that interleaves them keeps its
/// order and only the pairs are column-bound.
enum SectionColumn { ColLabel = 0, ColStd = 1, ColLog = 2, ColCopy = 3, ColCount = 4 };

/// Cap on the standard-form column, matching WinUI's 400 px Auto column: it
/// stops one unusually wide standard form (the stand-off J/S line) from
/// starving the log form beside it.
constexpr int kStdFormMaxWidth = 300;

/// Fixed width of the trailing copy-button column.
constexpr int kCopyButtonWidth = 26;

/// Builds the "⧉" copy button shared by both row kinds.
QPushButton* makeCopyButton(const QString& copyText, const QString& label)
{
    auto* btn = new QPushButton(QStringLiteral("\u29c9"));
    btn->setFixedWidth(kCopyButtonWidth);
    btn->setFlat(true);
    btn->setToolTip(QStringLiteral("Copy ") + copyText);
    btn->setAccessibleName(QStringLiteral("Copy ") + label);
    QObject::connect(btn, &QPushButton::clicked, btn, [copyText](){
        QGuiApplication::clipboard()->setText(copyText);
    });
    return btn;
}

/// Builds a row's leading label. The buddy is set for the same reason
/// QFormLayout::addRow() sets one: it is what makes QLabel parse the
/// mnemonic, so displayText()'s "&&" escaping renders as a single '&'.
QLabel* makeRowLabel(const QString& label, QWidget* buddy)
{
    auto* lbl = new QLabel(displayText(label) + ':');
    lbl->setBuddy(buddy);
    return lbl;
}

/// Adds one "Value" reference row into @p grid at @p row.
/// The value spans the two form columns; rows with a copy value get a "⧉"
/// button in the trailing column. When @p registry is non-null, appends
/// {label, valLbl} so a page-level "Copy Results" button can include it.
void addRefRow(QGridLayout* grid,
               int row,
               const QString& label,
               const QString& value,
               const QString& copyValue,
               ResultRowRegistry* registry)
{
    auto* valLbl = new QLabel(value);
    valLbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
    valLbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    // Values range from short numbers to full prose (Glossary definitions,
    // RCS validity notes). Unwrapped, the longest line sets the page's
    // minimum width — at the default window size the Glossary clipped on the
    // right behind a horizontal scrollbar. Wrapping lets every page fit any
    // width down to the window minimum; short values are unaffected. Matches
    // macOS, where Text wraps by default.
    valLbl->setWordWrap(true);
    valLbl->setAccessibleName(label);
    // Match PageUtils.h's addResultRow(): calculator result values are
    // monospaced, and the Reference page's values sit in the same visual
    // role, so they must use the same family or the two sections read as
    // different typography.
    QFont valFont = valLbl->font();
    valFont.setFamily(QStringLiteral("monospace"));
    valLbl->setFont(valFont);
    if (registry)
        registry->append({label, valLbl});

    grid->addWidget(makeRowLabel(label, valLbl), row, ColLabel, Qt::AlignLeft | Qt::AlignVCenter);
    // Spans both form columns: a value row is not column-bound, and the
    // trailing column keeps its right edge level with the formula rows'.
    grid->addWidget(valLbl, row, ColStd, 1, 2);
    if (!copyValue.isEmpty())
        grid->addWidget(makeCopyButton(copyValue, label), row, ColCopy, Qt::AlignVCenter);
}

/// A formula image that shrinks to the width it is given and is never drawn
/// larger than its natural size. The PNG is a 2x render (see
/// scripts/render-diagrams.sh), so natural size is half its pixel size, and
/// drawing at that size keeps it crisp on hi-DPI screens.
///
/// This exists because a plain QLabel pixmap is a *fixed* size: the widest
/// masters (the stand-off J/S line, the radar-range log form) then set the
/// page's minimum width, and near the 980 px window minimum the Radar &
/// Detection page clipped its text behind a horizontal scrollbar. A static
/// cap only moves that threshold — the image has to respond to the width
/// actually available. This matches what the other frontends already do:
/// maxWidth + scaledToFit on macOS, MaxWidth + Stretch="Uniform" on WinUI.
class FormulaImage : public QLabel
{
public:
    /// @p maxLogicalWidth caps the drawn width (0 = natural size only).
    FormulaImage(const QPixmap& source, const QString& altText, int maxLogicalWidth = 0)
        : source_(source), maxWidth_(maxLogicalWidth)
    {
        setAccessibleName(altText);
        setToolTip(altText);
        // Horizontally shrinkable, vertically driven by heightForWidth.
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    }

    bool  hasHeightForWidth() const override { return true; }
    int   heightForWidth(int w) const override { return scaledSize(w).height(); }
    QSize sizeHint() const override          { return scaledSize(naturalWidth()); }
    // Allow the layout to squeeze the image rather than force the page wider.
    QSize minimumSizeHint() const override   { return QSize(1, 1); }

protected:
    /// Draws the source scaled to the width actually granted, left-aligned and
    /// vertically centred. Painting rather than holding a pre-scaled pixmap is
    /// deliberate: QLabel *clips* a pixmap wider than the widget, so any moment
    /// where the layout had shrunk the label but its stored pixmap was still
    /// the old size cut the formula off mid-glyph — seen on the stand-off J/S
    /// line, which lost its trailing "− 10 log₁₀ σ(m²) dB". Scaling at paint
    /// time cannot disagree with the current geometry.
    void paintEvent(QPaintEvent*) override
    {
        const QSize target = scaledSize(width());
        if (target.isEmpty() || source_.isNull())
            return;
        QPainter painter(this);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        const QRect where(0, (height() - target.height()) / 2, target.width(), target.height());
        painter.drawPixmap(where, source_);
    }

private:
    int naturalWidth() const
    {
        const int natural = qRound(source_.width() / source_.devicePixelRatio());
        return (maxWidth_ > 0) ? std::min(natural, maxWidth_) : natural;
    }

    /// Logical size when drawn into @p w px — aspect-preserving, never past
    /// natural size.
    QSize scaledSize(int w) const
    {
        const int natural = naturalWidth();
        if (natural <= 0)
            return QSize();
        const int   target = std::clamp(w, 1, natural);
        const qreal scale  = qreal(target) / natural;
        return QSize(target,
                     qMax(1, qRound(source_.height() / source_.devicePixelRatio() * scale)));
    }

    QPixmap source_;
    int     maxWidth_ = 0;
};

/// Creates the formula image for @p resourcePath. @p altText (the Unicode
/// plain-text equivalent from the data layer) becomes the accessible name and
/// tooltip, standing in for the image for screen readers and on hover.
QLabel* makeFormulaImage(const QString& resourcePath, const QString& altText,
                         int maxLogicalWidth = 0)
{
    QPixmap pix(resourcePath);
    pix.setDevicePixelRatio(2.0);
    return new FormulaImage(pix, altText, maxLogicalWidth);
}

/// Adds one "Formula" reference row into @p grid at @p row: the standard
/// form in the shared standard-form column and, when present, the log form
/// in the column beside it, followed by a "⧉" button that copies the Unicode
/// plain-text form(s) — matching the copy-button style used by value rows.
///
/// Each form is its own widget in its own column rather than a pair sharing
/// one box, so a form shrinks only when its own column is too narrow. The
/// trade-off that accepts — a shrunken log form no longer matches its
/// standard form's glyph size — is inherent to column-binding the two, and
/// only shows at widths where a form must shrink at all. WinUI made the same
/// trade in 848ad92.
void addFormulaRow(QGridLayout* grid, int row, const refdata::Row& formulaRow)
{
    const QString label   = QString::fromUtf8(formulaRow.label);
    const QString stdText = QString::fromUtf8(formulaRow.value);
    const QString base    = QString::fromUtf8(formulaRow.svg_base);

    auto* stdImage = makeFormulaImage(QStringLiteral(":/formulas/%1-std.png").arg(base),
                                      stdText, kStdFormMaxWidth);
    grid->addWidget(makeRowLabel(label, stdImage), row, ColLabel, Qt::AlignLeft | Qt::AlignVCenter);
    grid->addWidget(stdImage, row, ColStd);

    QString copyText = stdText;
    if (formulaRow.log_value) {
        const QString logText = QString::fromUtf8(formulaRow.log_value);
        grid->addWidget(makeFormulaImage(QStringLiteral(":/formulas/%1-log.png").arg(base), logText),
                        row, ColLog);
        copyText += QStringLiteral("   |   ") + logText;
    }

    grid->addWidget(makeCopyButton(copyText, label), row, ColCopy, Qt::AlignVCenter);
}

/// Maximum logical width (px) for a section's diagram thumbnail — matches
/// the macOS ReferenceView's `frame(maxWidth: 480)`.
constexpr int kSectionDiagramMaxWidth = 480;

/// Loads a section's optional #72 diagram (":/diagrams/<name>.png", a 2x
/// render — see DiagramUtils.h) as a centered thumbnail capped at
/// kSectionDiagramMaxWidth logical px, never upscaled past its natural size.
/// This intentionally doesn't reuse DiagramUtils.h's DiagramLabel/
/// makeDiagramGroup: those build a resizable, collapsible "Geometry" group
/// for a page's own diagrams, where this is a small always-visible, fixed
/// thumbnail inline in a reference section — same PNG source and DPR
/// convention, different presentation.
QLabel* makeSectionDiagram(const QString& name)
{
    QPixmap pix(QStringLiteral(":/diagrams/%1.png").arg(name));
    constexpr qreal dpr = 2.0;
    if (!pix.isNull()) {
        const int naturalWidth = qRound(pix.width() / dpr);
        const int targetWidth  = std::min(naturalWidth, kSectionDiagramMaxWidth);
        if (targetWidth > 0 && targetWidth < naturalWidth)
            pix = pix.scaledToWidth(qRound(targetWidth * dpr), Qt::SmoothTransformation);
        pix.setDevicePixelRatio(dpr);
    }

    auto* lbl = new QLabel;
    lbl->setPixmap(pix);
    lbl->setAlignment(Qt::AlignHCenter);
    const QString alt = QString(name).replace('-', ' ');
    lbl->setAccessibleName(alt);
    lbl->setToolTip(alt);
    return lbl;
}

} // namespace

// ── ReferencePage ─────────────────────────────────────────────────────────────

ReferencePage::ReferencePage(std::size_t pageIndex, QWidget* parent)
    : QWidget(parent)
{
    auto* content = new QWidget;
    auto* vbox    = new QVBoxLayout(content);

    const auto pages = refdata::pages();
    ResultRowRegistry results;
    if (pageIndex < pages.size()) {
        const auto& page = pages[pageIndex];
        for (std::size_t s = 0; s < page.section_count; ++s) {
            const auto& section = page.sections[s];
            auto* box  = new QGroupBox(displayText(QString::fromUtf8(section.title)));
            auto* grid = new QGridLayout(box);
            grid->setHorizontalSpacing(12);
            // Only the log-form column takes up slack, so the standard-form
            // column stays at the width of the section's widest standard form
            // and every log form in the section shares a left edge.
            grid->setColumnStretch(ColLog, 1);
            grid->setColumnMinimumWidth(ColCopy, kCopyButtonWidth);

            int gridRow = 0;
            if (section.diagram)
                grid->addWidget(makeSectionDiagram(QString::fromUtf8(section.diagram)),
                                gridRow++, ColLabel, 1, ColCount, Qt::AlignHCenter);

            for (std::size_t r = 0; r < section.row_count; ++r) {
                const auto& row = section.rows[r];
                switch (row.kind) {
                    case refdata::RowKind::Value:
                        addRefRow(grid, gridRow++,
                                  QString::fromUtf8(row.label),
                                  QString::fromUtf8(row.value),
                                  row.copy_value ? QString::fromUtf8(row.copy_value) : QString{},
                                  &results);
                        break;
                    case refdata::RowKind::Formula:
                        addFormulaRow(grid, gridRow++, row);
                        break;
                }
            }
            vbox->addWidget(box);
        }
    }
    vbox->addWidget(addCopyResultsButton(results));
    vbox->addStretch();

    auto* scroll = new QScrollArea(this);
    scroll->setWidget(content);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);
}
