#pragma once
/// @file PageUtils.h
/// @brief Lightweight helpers used by every calculator page.

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QString>

/// Creates a QDoubleSpinBox with the given range, initial value, single-step,
/// and decimal precision. Initial value is set before any signals are connected
/// so no spurious valueChanged fires during construction.
inline QDoubleSpinBox* makeSpinBox(
    double minVal, double maxVal, double initVal,
    double step = 1.0, int decimals = 3,
    QWidget* parent = nullptr)
{
    auto* sb = new QDoubleSpinBox(parent);
    sb->setRange(minVal, maxVal);
    sb->setSingleStep(step);
    sb->setDecimals(decimals);
    sb->setValue(initVal);
    return sb;
}

/// Adds a read-only result row to a QFormLayout and returns the QLabel.
inline QLabel* addResultRow(QFormLayout* form, const QString& fieldLabel)
{
    auto* lbl = new QLabel(QStringLiteral("\u2014")); // em-dash placeholder
    lbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
    form->addRow(fieldLabel + ':', lbl);
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
