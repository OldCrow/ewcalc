// JammingPage.cpp
#include "JammingPage.h"
#include "PageUtils.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

JammingPage::JammingPage(QWidget* parent)
    : QWidget(parent)
{
    // ── J/S geometry inputs ───────────────────────────────────────────────────
    QFormLayout* inForm = nullptr;
    auto* inGroup = makeGroup(QStringLiteral("J/S Geometry"), inForm);

    auto* sigErpSb  = makeSpinBox(-100.0, 200.0, presenter_.signal_erp_dbm(),       1.0, 1);
    auto* jamErpSb  = makeSpinBox(-100.0, 200.0, presenter_.jammer_erp_dbm(),       1.0, 1);
    auto* sigDistSb = makeSpinBox(   0.01, 10000.0, presenter_.signal_to_rx_dist_km(), 0.1, 3);
    auto* jamDistSb = makeSpinBox(   0.01, 10000.0, presenter_.jammer_to_rx_dist_km(),0.1, 3);
    auto* sigHtSb   = makeSpinBox(   0.1,  100000.0, presenter_.signal_tx_height_m(), 0.5, 1);
    auto* jamHtSb   = makeSpinBox(   0.1,  100000.0, presenter_.jammer_height_m(),    0.5, 1);
    auto* rxHtSb    = makeSpinBox(   0.1,  100000.0, presenter_.rx_height_m(),        0.5, 1);
    auto* freqSb    = makeSpinBox(   0.1,  100000.0, presenter_.frequency_mhz(),      1.0, 1);
    auto* rxGainSigSb = makeSpinBox(-30.0, 60.0, 0.0, 1.0, 1);
    auto* rxGainJamSb = makeSpinBox(-30.0, 60.0, 0.0, 1.0, 1);

    inForm->addRow(QStringLiteral("Signal ERP (dBm):"),       sigErpSb);
    inForm->addRow(QStringLiteral("Jammer ERP (dBm):"),       jamErpSb);
    inForm->addRow(QStringLiteral("Signal→Rx dist (km):"),    sigDistSb);
    inForm->addRow(QStringLiteral("Jammer→Rx dist (km):"),    jamDistSb);
    inForm->addRow(QStringLiteral("Signal Tx height (m):"),   sigHtSb);
    inForm->addRow(QStringLiteral("Jammer height (m):"),      jamHtSb);
    inForm->addRow(QStringLiteral("Rx height (m):"),          rxHtSb);
    inForm->addRow(QStringLiteral("Frequency (MHz):"),        freqSb);
    inForm->addRow(QStringLiteral("Rx gain → signal (dB):"),  rxGainSigSb);
    inForm->addRow(QStringLiteral("Rx gain → jammer (dB):"),  rxGainJamSb);

    // ── Partial-band inputs ───────────────────────────────────────────────────
    QFormLayout* pbForm = nullptr;
    auto* pbGroup = makeGroup(QStringLiteral("Partial-Band"), pbForm);

    auto* sigBwSb  = makeSpinBox(0.001, 1000.0,  presenter_.signal_bandwidth_mhz(), 0.001, 3);
    auto* hopRgSb  = makeSpinBox(0.001, 10000.0, presenter_.hop_range_mhz(),        1.0,   1);

    pbForm->addRow(QStringLiteral("Signal BW (MHz):"),    sigBwSb);
    pbForm->addRow(QStringLiteral("Hop range (MHz):"),    hopRgSb);

    // ── Outputs ───────────────────────────────────────────────────────────────
    QFormLayout* outForm = nullptr;
    auto* outGroup = makeGroup(QStringLiteral("Results"), outForm);

    js_ratio_    = addResultRow(outForm, QStringLiteral("J/S ratio"));
    signal_at_rx_= addResultRow(outForm, QStringLiteral("Signal at Rx"));
    jammer_at_rx_= addResultRow(outForm, QStringLiteral("Jammer at Rx"));
    optimum_bw_  = addResultRow(outForm, QStringLiteral("Optimum jammer BW"));
    duty_cycle_  = addResultRow(outForm, QStringLiteral("Duty cycle"));

    // ── Scroll container ──────────────────────────────────────────────────────
    auto* content = new QWidget;
    auto* vbox    = new QVBoxLayout(content);
    vbox->addWidget(inGroup);
    vbox->addWidget(pbGroup);
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
    connect(sigErpSb,   &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_signal_erp(v); });
    connect(jamErpSb,   &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_jammer_erp(v); });
    connect(sigDistSb,  &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_signal_to_rx_dist(v); });
    connect(jamDistSb,  &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_jammer_to_rx_dist(v); });
    connect(sigHtSb,    &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_signal_tx_height(v); });
    connect(jamHtSb,    &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_jammer_height(v); });
    connect(rxHtSb,     &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_rx_height(v); });
    connect(freqSb,     &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_frequency(v); });
    connect(rxGainSigSb,&QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_rx_gain_signal(v); });
    connect(rxGainJamSb,&QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_rx_gain_jammer(v); });
    connect(sigBwSb,    &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_signal_bandwidth(v); });
    connect(hopRgSb,    &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_hop_range(v); });

    presenter_.set_on_change([this](const ewpresenter::JammingPresenter::Output& o){
        applyOutput(o);
    });

    applyOutput(presenter_.output());
}

void JammingPage::applyOutput(const ewpresenter::JammingPresenter::Output& o)
{
    js_ratio_->setText(QString::fromStdString(o.js_ratio_str));
    signal_at_rx_->setText(QString::fromStdString(o.signal_at_rx_str));
    jammer_at_rx_->setText(QString::fromStdString(o.jammer_at_rx_str));
    optimum_bw_->setText(QString::fromStdString(o.optimum_bw_str));
    duty_cycle_->setText(QString::fromStdString(o.duty_cycle_str));
}
