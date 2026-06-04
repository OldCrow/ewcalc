// AntennaPage.cpp
#include "AntennaPage.h"
#include "PageUtils.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

AntennaPage::AntennaPage(QWidget* parent)
    : QWidget(parent)
{
    // ── Antenna inputs ────────────────────────────────────────────────────────
    QFormLayout* inForm = nullptr;
    auto* inGroup = makeGroup(QStringLiteral("Antenna Parameters"), inForm);

    auto* gainSb   = addSpinRow(inForm, QStringLiteral("Gain (dBi)"),         -10.0,  60.0,    presenter_.gain_dbi(),          1.0, 1);
    auto* azBwSb   = addSpinRow(inForm, QStringLiteral("Az beamwidth (deg)"),   0.1, 360.0,    presenter_.az_beamwidth_deg(),  1.0, 1);
    auto* elBwSb   = addSpinRow(inForm, QStringLiteral("El beamwidth (deg)"),   0.1, 360.0,    presenter_.el_beamwidth_deg(),  1.0, 1);
    auto* txPwrSb  = addSpinRow(inForm, QStringLiteral("Tx power (dBm)"),     -30.0, 100.0,    presenter_.tx_power_dbm(),      1.0, 1);
    auto* freqSb   = addSpinRow(inForm, QStringLiteral("Frequency (MHz)"),      0.1, 100000.0, presenter_.frequency_mhz(),     1.0, 1);

    // ── Outputs ───────────────────────────────────────────────────────────────
    QFormLayout* outForm = nullptr;
    auto* outGroup = makeGroup(QStringLiteral("Results"), outForm);

    erp_                 = addResultRow(outForm, QStringLiteral("ERP"));
    beamwidth_from_gain_ = addResultRow(outForm, QStringLiteral("Beamwidth from gain"));
    gain_from_beamwidth_ = addResultRow(outForm, QStringLiteral("Gain from beamwidth"));
    wavelength_          = addResultRow(outForm, QStringLiteral("Wavelength"));

    // ── Scroll container ──────────────────────────────────────────────────────
    auto* content = new QWidget;
    auto* vbox    = new QVBoxLayout(content);
    vbox->addWidget(inGroup);
    vbox->addWidget(outGroup);
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
            [this](double v){ presenter_.set_gain(v); });
    connect(azBwSb,  &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_az_beamwidth(v); });
    connect(elBwSb,  &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_el_beamwidth(v); });
    connect(txPwrSb, &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_tx_power(v); });
    connect(freqSb,  &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_frequency(v); });

    presenter_.set_on_change([this](const ewpresenter::AntennaPresenter::Output& o){
        applyOutput(o);
    });

    applyOutput(presenter_.output());
}

void AntennaPage::applyOutput(const ewpresenter::AntennaPresenter::Output& o)
{
    erp_->setText(QString::fromStdString(o.erp_str));
    beamwidth_from_gain_->setText(QString::fromStdString(o.beamwidth_from_gain_str));
    gain_from_beamwidth_->setText(QString::fromStdString(o.gain_from_beamwidth_str));
    wavelength_->setText(QString::fromStdString(o.wavelength_str));
}
