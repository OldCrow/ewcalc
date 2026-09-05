// AntennaPage.cpp
#include "AntennaPage.h"
#include "DiagramUtils.h"
#include "PageUtils.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

namespace {
const QString kGroup = QStringLiteral("Antenna");
}

AntennaPage::AntennaPage(QWidget* parent)
    : QWidget(parent)
{
    // ── Antenna inputs ────────────────────────────────────────────────────────
    QFormLayout* inForm = nullptr;
    auto* inGroup = makeGroup(QStringLiteral("Antenna Parameters"), inForm);

    // Lower bound matches AntennaPresenter::set_gain's -6.35 dBi validation
    // floor (below that, beamwidth_from_gain() is out of its valid domain
    // and exceeds 360° — see audit #1 / libew antenna.h).
    auto* gainSb = addSpinRow(inForm, QStringLiteral("Gain (dBi)"), -6.35, 60.0,
        presenter_.gain_dbi(), 1.0, 1, kGroup, QStringLiteral("gain_dbi"),
        QStringLiteral("Antenna gain relative to an isotropic radiator"));
    auto* azBwSb = addSpinRow(inForm, QStringLiteral("Az beamwidth (deg)"), 0.1, 360.0,
        presenter_.az_beamwidth_deg(), 1.0, 1, kGroup, QStringLiteral("az_beamwidth_deg"),
        QStringLiteral("Azimuth 3 dB beamwidth \u2014 used with elevation beamwidth to estimate gain"));
    auto* elBwSb = addSpinRow(inForm, QStringLiteral("El beamwidth (deg)"), 0.1, 360.0,
        presenter_.el_beamwidth_deg(), 1.0, 1, kGroup, QStringLiteral("el_beamwidth_deg"),
        QStringLiteral("Elevation 3 dB beamwidth"));
    auto* txPwrSb = addSpinRow(inForm, QStringLiteral("Tx power (dBm)"), -30.0, 100.0,
        presenter_.tx_power_dbm(), 1.0, 1, kGroup, QStringLiteral("tx_power_dbm"),
        QStringLiteral("Transmitter output power \u2014 used to compute ERP"));
    auto* freqSb = addSpinRow(inForm, QStringLiteral("Frequency (MHz)"), 0.1, 100000.0,
        presenter_.frequency_mhz(), 1.0, 1, kGroup, QStringLiteral("frequency_mhz"),
        QStringLiteral("Carrier frequency \u2014 used to compute free-space wavelength"));

    // ── Outputs ───────────────────────────────────────────────────────────────
    QFormLayout* outForm = nullptr;
    auto* outGroup = makeGroup(QStringLiteral("Results"), outForm);

    ResultRowRegistry results;
    erp_ = addResultRow(outForm, QStringLiteral("ERP"),
        QStringLiteral("Effective Radiated Power = Tx power + antenna gain"), &results);
    beamwidth_from_gain_ = addResultRow(outForm, QStringLiteral("Beamwidth from gain"),
        QStringLiteral("Approximate 3 dB beamwidth derived from gain (Tai & Pereira approximation)"), &results);
    gain_from_beamwidth_ = addResultRow(outForm, QStringLiteral("Gain from beamwidth"),
        QStringLiteral("Approximate gain from az \u00d7 el beamwidth: 10\u00b7log\u2081\u2080(30000 / (\u03b8_az \u00b7 \u03b8_el))"), &results);
    wavelength_ = addResultRow(outForm, QStringLiteral("Wavelength"),
        QStringLiteral("Free-space wavelength at the given frequency"), &results);

    outForm->addRow(addCopyResultsButton(results));

    // ── Scroll container ──────────────────────────────────────────────────────────
    auto* content = new QWidget;
    auto* vbox    = new QVBoxLayout(content);
    vbox->addWidget(inGroup);
    vbox->addWidget(outGroup);
    vbox->addWidget(makeDiagramGroup({
        QStringLiteral(":/diagrams/antenna-beamwidth.png")}));
    vbox->addStretch();

    auto* scroll = new QScrollArea(this);
    scroll->setWidget(content);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);

    // ── Signal wiring ─────────────────────────────────────────────────────────
    connect(gainSb,  &QDoubleSpinBox::valueChanged, this,
            [this, gainSb](double v){ presenter_.set_gain(v); applyFieldError(gainSb, presenter_.gain_error()); });
    connect(azBwSb,  &QDoubleSpinBox::valueChanged, this,
            [this, azBwSb](double v){ presenter_.set_az_beamwidth(v); applyFieldError(azBwSb, presenter_.az_beamwidth_error()); });
    connect(elBwSb,  &QDoubleSpinBox::valueChanged, this,
            [this, elBwSb](double v){ presenter_.set_el_beamwidth(v); applyFieldError(elBwSb, presenter_.el_beamwidth_error()); });
    connect(txPwrSb, &QDoubleSpinBox::valueChanged, this,
            [this, txPwrSb](double v){ presenter_.set_tx_power(v); applyFieldError(txPwrSb, presenter_.tx_power_error()); });
    connect(freqSb,  &QDoubleSpinBox::valueChanged, this,
            [this, freqSb](double v){ presenter_.set_frequency(v); applyFieldError(freqSb, presenter_.frequency_error()); });

    // ── Restore persisted values (after presenter wiring, before first recompute) ──
    restoreSpinValue(gainSb,  kGroup, QStringLiteral("gain_dbi"));
    restoreSpinValue(azBwSb,  kGroup, QStringLiteral("az_beamwidth_deg"));
    restoreSpinValue(elBwSb,  kGroup, QStringLiteral("el_beamwidth_deg"));
    restoreSpinValue(txPwrSb, kGroup, QStringLiteral("tx_power_dbm"));
    restoreSpinValue(freqSb,  kGroup, QStringLiteral("frequency_mhz"));

    presenter_.set_on_change([this](const ewpresenter::AntennaPresenter::Output& o){
        applyOutput(o);
    });

    // Seed per-field validation-error styling from initial/restored values (#41).
    applyFieldError(gainSb,  presenter_.gain_error());
    applyFieldError(azBwSb,  presenter_.az_beamwidth_error());
    applyFieldError(elBwSb,  presenter_.el_beamwidth_error());
    applyFieldError(txPwrSb, presenter_.tx_power_error());
    applyFieldError(freqSb,  presenter_.frequency_error());

    applyOutput(presenter_.output());
}

void AntennaPage::applyOutput(const ewpresenter::AntennaPresenter::Output& o)
{
    erp_->setText(QString::fromStdString(o.erp_str));
    beamwidth_from_gain_->setText(QString::fromStdString(o.beamwidth_from_gain_str));
    gain_from_beamwidth_->setText(QString::fromStdString(o.gain_from_beamwidth_str));
    wavelength_->setText(QString::fromStdString(o.wavelength_str));
}
