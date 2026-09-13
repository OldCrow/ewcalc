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
#include <QFormLayout>
#include <QGroupBox>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
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

/// Escapes '&' for display in a QGroupBox title or a QFormLayout row label.
/// Both treat '&' as a mnemonic marker (addRow(QString, ...) creates a buddy
/// QLabel, which does the same), so refdata text such as "Receiver & Signal"
/// rendered as "Receiver _Signal" — the '&' swallowed and the following space
/// underlined. The data layer is shared with frontends that have no such
/// convention, so the escaping belongs here, at the Qt display boundary.
QString displayText(QString text)
{
    return text.replace(QLatin1Char('&'), QStringLiteral("&&"));
}

/// Adds one "Value" reference row to @p form.
/// Rows with a copy value get a small "⧉" button; others get a spacer.
/// When @p registry is non-null, appends {label, valLbl} so a page-level
/// "Copy Results" button can include this row.
void addRefRow(QFormLayout* form,
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

    // All rows use the same HBox layout so value text aligns consistently:
    // [value label — stretch] [copy button or fixed spacer — 26px]
    auto* cell = new QWidget;
    auto* hbox = new QHBoxLayout(cell);
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->setSpacing(4);
    hbox->addWidget(valLbl, 1);

    if (!copyValue.isEmpty()) {
        auto* btn = new QPushButton(QStringLiteral("\u29c9"));
        btn->setFixedWidth(26);
        btn->setFlat(true);
        btn->setToolTip(QStringLiteral("Copy ") + copyValue);
        btn->setAccessibleName(QStringLiteral("Copy ") + label);
        const QString cv = copyValue;
        QObject::connect(btn, &QPushButton::clicked, btn, [cv](){
            QGuiApplication::clipboard()->setText(cv);
        });
        hbox->addWidget(btn, 0);
    } else {
        // Fixed-width spacer keeps value text at the same right edge
        // as rows that have a copy button.
        auto* spacer = new QWidget;
        spacer->setFixedWidth(26);
        hbox->addWidget(spacer, 0);
    }
    form->addRow(displayText(label) + ':', cell);
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
    FormulaImage(const QPixmap& source, const QString& altText)
        : source_(source)
    {
        setAccessibleName(altText);
        setToolTip(altText);
        // Horizontally shrinkable, vertically driven by heightForWidth.
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        redraw(naturalWidth());
    }

    bool  hasHeightForWidth() const override { return true; }
    int   heightForWidth(int w) const override { return scaledSize(w).height(); }
    QSize sizeHint() const override          { return scaledSize(naturalWidth()); }
    // Allow the layout to squeeze the image rather than force the page wider.
    QSize minimumSizeHint() const override   { return QSize(1, 1); }

protected:
    void resizeEvent(QResizeEvent* event) override
    {
        QLabel::resizeEvent(event);
        redraw(width());
    }

private:
    int naturalWidth() const
    {
        return qRound(source_.width() / source_.devicePixelRatio());
    }

    /// Logical size when drawn into @p w px — aspect-preserving, never past
    /// natural size.
    QSize scaledSize(int w) const
    {
        const int natural = naturalWidth();
        const int target  = std::clamp(w, 1, natural);
        const qreal scale = qreal(target) / natural;
        return QSize(target,
                     qMax(1, qRound(source_.height() / source_.devicePixelRatio() * scale)));
    }

    void redraw(int w)
    {
        const QSize target = scaledSize(w);
        if (target == drawnAt_)
            return;              // avoids a setPixmap/relayout loop
        drawnAt_ = target;
        QPixmap pix = (target.width() == naturalWidth())
                          ? source_
                          : source_.scaledToWidth(qRound(target.width() * source_.devicePixelRatio()),
                                                  Qt::SmoothTransformation);
        pix.setDevicePixelRatio(source_.devicePixelRatio());
        setPixmap(pix);
    }

    QPixmap source_;
    QSize   drawnAt_;
};

/// Creates the formula image for @p resourcePath. @p altText (the Unicode
/// plain-text equivalent from the data layer) becomes the accessible name and
/// tooltip, standing in for the image for screen readers and on hover.
QLabel* makeFormulaImage(const QString& resourcePath, const QString& altText)
{
    QPixmap pix(resourcePath);
    pix.setDevicePixelRatio(2.0);
    return new FormulaImage(pix, altText);
}

/// Adds one "Formula" reference row to @p form: the standard and (if
/// present) log forms rendered side by side as typeset images, followed by
/// a "⧉" button that copies the Unicode plain-text form(s) to the
/// clipboard — matching the copy-button style used by value rows.
void addFormulaRow(QFormLayout* form, const refdata::Row& row)
{
    const QString label   = QString::fromUtf8(row.label);
    const QString stdText = QString::fromUtf8(row.value);
    const QString base    = QString::fromUtf8(row.svg_base);

    auto* cell = new QWidget;
    auto* hbox = new QHBoxLayout(cell);
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->setSpacing(12);

    hbox->addWidget(
        makeFormulaImage(QStringLiteral(":/formulas/%1-std.png").arg(base), stdText),
        0);

    QString copyText = stdText;
    if (row.log_value) {
        const QString logText = QString::fromUtf8(row.log_value);
        hbox->addWidget(
            makeFormulaImage(QStringLiteral(":/formulas/%1-log.png").arg(base), logText),
            0);
        copyText += QStringLiteral("   |   ") + logText;
    }
    hbox->addStretch(1);

    auto* btn = new QPushButton(QStringLiteral("\u29c9"));
    btn->setFixedWidth(26);
    btn->setFlat(true);
    btn->setToolTip(QStringLiteral("Copy ") + copyText);
    btn->setAccessibleName(QStringLiteral("Copy ") + label);
    QObject::connect(btn, &QPushButton::clicked, btn, [copyText](){
        QGuiApplication::clipboard()->setText(copyText);
    });
    hbox->addWidget(btn, 0);

    form->addRow(displayText(label) + ':', cell);
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
            auto* form = new QFormLayout(box);
            // Formula rows are fixed-width images that cannot wrap; label +
            // standard + log form side by side (Two-ray ground reflection is
            // the widest) overflows the content area near the 980 px window
            // minimum, pushing the copy buttons off-screen behind a horizontal
            // scrollbar. WrapLongRows keeps rows side by side when they fit
            // and drops only an over-wide field under its label. Word-wrapped
            // value rows have a small minimum width, so they never trigger it.
            form->setRowWrapPolicy(QFormLayout::WrapLongRows);

            if (section.diagram)
                form->addRow(makeSectionDiagram(QString::fromUtf8(section.diagram)));

            for (std::size_t r = 0; r < section.row_count; ++r) {
                const auto& row = section.rows[r];
                switch (row.kind) {
                    case refdata::RowKind::Value:
                        addRefRow(form,
                                  QString::fromUtf8(row.label),
                                  QString::fromUtf8(row.value),
                                  row.copy_value ? QString::fromUtf8(row.copy_value) : QString{},
                                  &results);
                        break;
                    case refdata::RowKind::Formula:
                        addFormulaRow(form, row);
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
