#pragma once
/// @file PageUtils.h
/// @brief Lightweight helpers used by every calculator page.

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QString>
#include <QWidget>

/// Creates a QDoubleSpinBox without adding it to a layout.
/// Used for spinboxes that live inside custom layouts (e.g. stage rows).
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
    sb->setAlignment(Qt::AlignRight);
    return sb;
}

/// Creates a fixed-width, right-aligned QDoubleSpinBox and adds it to the
/// given QFormLayout. This matches the macOS InputRow visual style:
/// label flush-left, spinbox fixed-width and right-aligned in its row.
/// Returns the spinbox so the caller can connect valueChanged.
inline QDoubleSpinBox* addSpinRow(
    QFormLayout* form, const QString& label,
    double minVal, double maxVal, double initVal,
    double step = 1.0, int decimals = 1)
{
    auto* sb = new QDoubleSpinBox;
    sb->setRange(minVal, maxVal);
    sb->setSingleStep(step);
    sb->setDecimals(decimals);
    sb->setValue(initVal);
    sb->setAlignment(Qt::AlignRight);
    sb->setMinimumWidth(110);
    sb->setMaximumWidth(150);

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
    int minVal, int maxVal, int initVal)
{
    auto* sb = new QSpinBox;
    sb->setRange(minVal, maxVal);
    sb->setValue(initVal);
    sb->setAlignment(Qt::AlignRight);
    sb->setMinimumWidth(110);
    sb->setMaximumWidth(150);

    auto* wrapper = new QWidget;
    auto* hbox    = new QHBoxLayout(wrapper);
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->addStretch(1);
    hbox->addWidget(sb);

    form->addRow(label + ':', wrapper);
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
