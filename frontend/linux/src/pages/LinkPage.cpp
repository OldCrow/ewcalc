// LinkPage.cpp
#include "LinkPage.h"
#include "PageUtils.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

LinkPage::LinkPage(QWidget* parent)
    : QWidget(parent)
{
    // ── Transmitter ───────────────────────────────────────────────────────────
    QFormLayout* txForm = nullptr;
    auto* txGroup = makeGroup(QStringLiteral("Transmitter"), txForm);

    auto* txPwrSb  = makeSpinBox(-50.0,   200.0,   presenter_.tx_power_dbm(),  1.0, 1);
    auto* txGainSb = makeSpinBox(-30.0,    60.0,   presenter_.tx_gain_db(),    1.0, 1);
    auto* txHtSb   = makeSpinBox(  0.1, 100000.0,  presenter_.tx_height_m(),   0.5, 1);

    txForm->addRow(QStringLiteral("Tx power (dBm):"),  txPwrSb);
    txForm->addRow(QStringLiteral("Tx gain (dB):"),    txGainSb);
    txForm->addRow(QStringLiteral("Tx height (m):"),   txHtSb);

    // ── Common ────────────────────────────────────────────────────────────────
    QFormLayout* cmForm = nullptr;
    auto* cmGroup = makeGroup(QStringLiteral("Common"), cmForm);

    auto* distSb = makeSpinBox(  0.01, 10000.0,  presenter_.distance_km(),    0.1, 3);
    auto* freqSb = makeSpinBox(  0.1,  100000.0, presenter_.frequency_mhz(), 1.0, 1);

    cmForm->addRow(QStringLiteral("Distance (km):"),   distSb);
    cmForm->addRow(QStringLiteral("Frequency (MHz):"), freqSb);

    // ── Receiver ──────────────────────────────────────────────────────────────
    QFormLayout* rxForm = nullptr;
    auto* rxGroup = makeGroup(QStringLiteral("Receiver"), rxForm);

    auto* rxGainSb = makeSpinBox(-30.0,    60.0,   presenter_.rx_gain_db(),           1.0, 1);
    auto* rxHtSb   = makeSpinBox(  0.1, 100000.0,  presenter_.rx_height_m(),          0.5, 1);
    auto* rxSensSb = makeSpinBox(-200.0,    0.0,   presenter_.rx_sensitivity_dbm(),   1.0, 1);

    rxForm->addRow(QStringLiteral("Rx gain (dB):"),         rxGainSb);
    rxForm->addRow(QStringLiteral("Rx height (m):"),        rxHtSb);
    rxForm->addRow(QStringLiteral("Rx sensitivity (dBm):"), rxSensSb);

    // ── Outputs ───────────────────────────────────────────────────────────────
    QFormLayout* outForm = nullptr;
    auto* outGroup = makeGroup(QStringLiteral("Results"), outForm);

    received_power_  = addResultRow(outForm, QStringLiteral("Received power"));
    path_loss_       = addResultRow(outForm, QStringLiteral("Path loss"));
    fresnel_zone_    = addResultRow(outForm, QStringLiteral("Fresnel crossover"));
    regime_          = addResultRow(outForm, QStringLiteral("Regime"));
    link_margin_     = addResultRow(outForm, QStringLiteral("Link margin"));
    effective_range_ = addResultRow(outForm, QStringLiteral("Effective range"));
    range_regime_    = addResultRow(outForm, QStringLiteral("Range regime"));

    // ── Scroll container ──────────────────────────────────────────────────────
    auto* content = new QWidget;
    auto* vbox    = new QVBoxLayout(content);
    vbox->addWidget(txGroup);
    vbox->addWidget(cmGroup);
    vbox->addWidget(rxGroup);
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
    connect(txPwrSb,  &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_tx_power(v); });
    connect(txGainSb, &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_tx_gain(v); });
    connect(txHtSb,   &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_tx_height(v); });
    connect(distSb,   &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_distance(v); });
    connect(freqSb,   &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_frequency(v); });
    connect(rxGainSb, &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_rx_gain(v); });
    connect(rxHtSb,   &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_rx_height(v); });
    connect(rxSensSb, &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_rx_sensitivity(v); });

    presenter_.set_on_change([this](const ewpresenter::LinkPresenter::Output& o){
        applyOutput(o);
    });

    applyOutput(presenter_.output());
}

void LinkPage::applyOutput(const ewpresenter::LinkPresenter::Output& o)
{
    received_power_->setText(QString::fromStdString(o.received_power_str));
    path_loss_->setText(QString::fromStdString(o.path_loss_str));
    fresnel_zone_->setText(QString::fromStdString(o.fresnel_zone_str));
    regime_->setText(QString::fromStdString(o.regime_str));
    link_margin_->setText(QString::fromStdString(o.link_margin_str));
    effective_range_->setText(QString::fromStdString(o.effective_range_str));
    range_regime_->setText(QString::fromStdString(o.range_regime_str));
}
