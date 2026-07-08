#pragma once
/// @file PageUtils.h
/// @brief Lightweight helpers used by every calculator page.
///
/// These helpers wire up three cross-cutting UX concerns in one place so
/// individual pages stay declarative:
///  - Tooltips (setToolTip) ported from the macOS help strings (issue #18).
///  - Accessible names (setAccessibleName) derived from each field's visible
///    label (issue #19).
///  - Input persistence: input rows auto-save to AppSettings on every change;
///    pages call restoreSpinValue() once wiring to the presenter is complete
///    to seed the restored value (issue #20).
///  - A "Copy Results" clipboard button built from a registry of result rows
///    (issue #21).

#include "Settings.h"

#include <QClipboard>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPair>
#include <QPushButton>
#include <QSpinBox>
#include <QString>
#include <QVector>
#include <QWidget>

/// One entry in a page's result-row registry: the visible label and the
/// QLabel displaying its current formatted value.
using ResultRowRegistry = QVector<QPair<QString, QLabel*>>;

/// Creates a QDoubleSpinBox without adding it to a layout.
/// Used for spinboxes that live inside custom layouts (e.g. stage rows).
/// Ownership: the caller must either pass a non-null parent to take immediate
/// ownership, or add the returned spinbox to a layout (which re-parents it)
/// before any exception can be thrown — otherwise the widget will leak.
inline QDoubleSpinBox* makeSpinBox(
    double minVal, double maxVal, double initVal,
    double step = 1.0, int decimals = 3,
    const QString& accessibleName = QString(),
    QWidget* parent = nullptr)
{
    auto* sb = new QDoubleSpinBox(parent);
    sb->setRange(minVal, maxVal);
    sb->setSingleStep(step);
    sb->setDecimals(decimals);
    sb->setValue(initVal);
    sb->setAlignment(Qt::AlignRight);
    if (!accessibleName.isEmpty())
        sb->setAccessibleName(accessibleName);
    return sb;
}

/// Creates a fixed-width, right-aligned QDoubleSpinBox and adds it to the
/// given QFormLayout. This matches the macOS InputRow visual style:
/// label flush-left, spinbox fixed-width and right-aligned in its row.
///
/// Sets a tooltip from @p help (when non-empty, ported from macOS InputRow
/// help text) and an accessible name from @p label. Auto-saves to
/// AppSettings[settingsGroup][settingsKey] on every edit.
///
/// Returns the spinbox so the caller can connect valueChanged to the
/// presenter setter. NOTE: after wiring that connection, call
/// restoreSpinValue() once more so a persisted value that differs from
/// initVal is actually propagated to the presenter before the first
/// recompute (see issue #20).
inline QDoubleSpinBox* addSpinRow(
    QFormLayout* form, const QString& label,
    double minVal, double maxVal, double initVal,
    double step, int decimals,
    const QString& settingsGroup, const QString& settingsKey,
    const QString& help = QString())
{
    auto* sb = new QDoubleSpinBox;
    sb->setRange(minVal, maxVal);
    sb->setSingleStep(step);
    sb->setDecimals(decimals);
    sb->setValue(initVal);
    sb->setAlignment(Qt::AlignRight);
    sb->setMinimumWidth(110);
    sb->setMaximumWidth(150);
    sb->setAccessibleName(label);
    if (!help.isEmpty())
        sb->setToolTip(help);

    QObject::connect(sb, &QDoubleSpinBox::valueChanged, sb,
        [settingsGroup, settingsKey](double v) {
            AppSettings::instance().setValue(settingsGroup, settingsKey, v);
        });

    auto* wrapper = new QWidget;
    auto* hbox    = new QHBoxLayout(wrapper);
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->addStretch(1);
    hbox->addWidget(sb);

    form->addRow(label + ':', wrapper);
    return sb;
}

/// Same as addSpinRow but for integer QSpinBox (ADC bits, coherent pulses, etc.).
inline QSpinBox* addIntSpinRow(
    QFormLayout* form, const QString& label,
    int minVal, int maxVal, int initVal,
    const QString& settingsGroup, const QString& settingsKey,
    const QString& help = QString())
{
    auto* sb = new QSpinBox;
    sb->setRange(minVal, maxVal);
    sb->setValue(initVal);
    sb->setAlignment(Qt::AlignRight);
    sb->setMinimumWidth(110);
    sb->setMaximumWidth(150);
    sb->setAccessibleName(label);
    if (!help.isEmpty())
        sb->setToolTip(help);

    QObject::connect(sb, &QSpinBox::valueChanged, sb,
        [settingsGroup, settingsKey](int v) {
            AppSettings::instance().setValue(settingsGroup, settingsKey, v);
        });

    auto* wrapper = new QWidget;
    auto* hbox    = new QHBoxLayout(wrapper);
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->addStretch(1);
    hbox->addWidget(sb);

    form->addRow(label + ':', wrapper);
    return sb;
}

/// Seeds @p sb with its persisted value (if any) from
/// AppSettings[group][key], clamped to the spinbox's range. Call this once
/// per field, after the presenter-update connection has been wired, so a
/// restored value is propagated to the presenter before the first recompute.
/// A no-op (no signal emitted) if there is nothing persisted yet or the
/// persisted value equals the current value.
inline void restoreSpinValue(QDoubleSpinBox* sb, const QString& group, const QString& key)
{
    const double restored = AppSettings::instance().value(group, key, sb->value()).toDouble();
    sb->setValue(qBound(sb->minimum(), restored, sb->maximum()));
}

/// Integer overload of restoreSpinValue().
inline void restoreSpinValue(QSpinBox* sb, const QString& group, const QString& key)
{
    const int restored = AppSettings::instance().value(group, key, sb->value()).toInt();
    sb->setValue(qBound(sb->minimum(), restored, sb->maximum()));
}

/// Adds a read-only result row to a QFormLayout and returns the QLabel.
/// Value text is right-aligned in monospaced font, matching the macOS ResultRow style.
/// Sets a tooltip from @p help (ported from macOS ResultRow help text) and an
/// accessible name from @p fieldLabel. When @p registry is non-null, appends
/// {fieldLabel, label} so a page-level "Copy Results" button can include it.
inline QLabel* addResultRow(
    QFormLayout* form, const QString& fieldLabel,
    const QString& help = QString(),
    ResultRowRegistry* registry = nullptr)
{
    auto* lbl = new QLabel(QStringLiteral("\u2014")); // em-dash placeholder
    lbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
    lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    lbl->setAccessibleName(fieldLabel);
    if (!help.isEmpty())
        lbl->setToolTip(help);
    QFont f = lbl->font();
    f.setFamily(QStringLiteral("monospace"));
    lbl->setFont(f);
    form->addRow(fieldLabel + ':', lbl);
    if (registry)
        registry->append({fieldLabel, lbl});
    return lbl;
}

/// Creates a titled QGroupBox wrapping a new QFormLayout.
/// @param outLayout   receives the form layout; caller populates it.
inline QGroupBox* makeGroup(const QString& title, QFormLayout*& outLayout)
{
    auto* box    = new QGroupBox(title);
    outLayout    = new QFormLayout(box);
    return box;
}

/// Creates a "Copy Results" button that copies every row in @p results to
/// the system clipboard as plain text, one "Label: Value" line per row.
inline QPushButton* addCopyResultsButton(const ResultRowRegistry& results)
{
    auto* btn = new QPushButton(QStringLiteral("Copy Results"));
    btn->setToolTip(QStringLiteral("Copy all result values on this page to the clipboard"));
    btn->setAccessibleName(QStringLiteral("Copy Results"));
    QObject::connect(btn, &QPushButton::clicked, btn, [results] {
        QString text;
        for (const auto& row : results)
            text += row.first + QStringLiteral(": ") + row.second->text() + QStringLiteral("\n");
        QGuiApplication::clipboard()->setText(text);
    });
    return btn;
}
