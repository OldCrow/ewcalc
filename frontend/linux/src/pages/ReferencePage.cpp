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
#include <QScrollArea>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

namespace refdata = ewpresenter::refdata;

namespace {

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
    form->addRow(label + ':', cell);
}

/// Creates a QLabel showing a typeset formula image. The PNG is a 2x render
/// (see scripts/render-diagrams.sh); setDevicePixelRatio(2.0) on the pixmap
/// makes Qt lay it out — and draw it — at half its pixel size, so it reads
/// crisp on hi-DPI screens without upscaling blur. @p altText (the Unicode
/// plain-text equivalent from the data layer) becomes the label's accessible
/// name and tooltip, standing in for the image for screen readers and on
/// hover.
QLabel* makeFormulaImage(const QString& resourcePath, const QString& altText)
{
    auto* lbl = new QLabel;
    QPixmap pix(resourcePath);
    pix.setDevicePixelRatio(2.0);
    lbl->setPixmap(pix);
    lbl->setAccessibleName(altText);
    lbl->setToolTip(altText);
    return lbl;
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

    form->addRow(label + ':', cell);
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
            auto* box  = new QGroupBox(QString::fromUtf8(section.title));
            auto* form = new QFormLayout(box);

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
