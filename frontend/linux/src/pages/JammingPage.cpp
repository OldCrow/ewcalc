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
    // ── Signal (Tx) ───────────────────────────────────────────────────────────
    QFormLayout* sigForm = nullptr;
    auto* sigGroup = makeGroup(QStringLiteral("Signal (Tx)"), sigForm);

    auto* sigErpSb  = addSpinRow(sigForm, QStringLiteral("Signal ERP (dBm)"),       -100.0, 200.0,    presenter_.signal_erp_dbm(),          1.0, 1);
    auto* sigHtSb   = addSpinRow(sigForm, QStringLiteral("Signal Tx height (m)"),      0.1, 100000.0, presenter_.signal_tx_height_m(),      0.5, 1);
    auto* sigDistSb = addSpinRow(sigForm, QStringLiteral("Signal\u2192Rx dist (km)"),  0.01, 10000.0,  presenter_.signal_to_rx_dist_km(),    0.1, 3);
    auto* sigBwSb   = addSpinRow(sigForm, QStringLiteral("Signal BW (MHz)"),          0.001, 1000.0,   presenter_.signal_bandwidth_mhz(), 0.001, 3);

    // ── Jammer ────────────────────────────────────────────────────────────────
    QFormLayout* jamForm = nullptr;
    auto* jamGroup = makeGroup(QStringLiteral("Jammer"), jamForm);

    auto* jamErpSb  = addSpinRow(jamForm, QStringLiteral("Jammer ERP (dBm)"),    -100.0, 200.0,   presenter_.jammer_erp_dbm(),       1.0, 1);
    auto* jamHtSb   = addSpinRow(jamForm, QStringLiteral("Jammer height (m)"),     0.1, 100000.0, presenter_.jammer_height_m(),      0.5, 1);
    auto* jamDistSb = addSpinRow(jamForm, QStringLiteral("Jammer\u2192Rx dist (km)"), 0.01, 10000.0, presenter_.jammer_to_rx_dist_km(), 0.1, 3);
    auto* hopRgSb   = addSpinRow(jamForm, QStringLiteral("Hop range (MHz)"),        0.0, 10000.0, presenter_.hop_range_mhz(),        1.0, 1);
    auto* jsThreshSb= addSpinRow(jamForm, QStringLiteral("J/S threshold (dB)"),   -30.0,    30.0, presenter_.js_threshold_db(),      0.5, 1);

    // ── Shared ────────────────────────────────────────────────────────────────
    QFormLayout* cmForm = nullptr;
    auto* cmGroup = makeGroup(QStringLiteral("Shared"), cmForm);

    auto* freqSb      = addSpinRow(cmForm, QStringLiteral("Frequency (MHz)"),        0.1, 100000.0, presenter_.frequency_mhz(),   1.0, 1);
    auto* rxHtSb      = addSpinRow(cmForm, QStringLiteral("Rx height (m)"),          0.1, 100000.0, presenter_.rx_height_m(),     0.5, 1);
    auto* rxGainSigSb = addSpinRow(cmForm, QStringLiteral("Rx gain \u2192 signal (dB)"), -30.0, 60.0, 0.0, 1.0, 1);
    auto* rxGainJamSb = addSpinRow(cmForm, QStringLiteral("Rx gain \u2192 jammer (dB)"), -30.0, 60.0, 0.0, 1.0, 1);

    // ── J/S Analysis results ──────────────────────────────────────────────────
    QFormLayout* jsForm = nullptr;
    auto* jsGroup = makeGroup(QStringLiteral("J/S Analysis"), jsForm);

    js_ratio_          = addResultRow(jsForm, QStringLiteral("J/S ratio"));
    signal_at_rx_      = addResultRow(jsForm, QStringLiteral("Signal at Rx"));
    jammer_at_rx_      = addResultRow(jsForm, QStringLiteral("Jammer at Rx"));
    burnthrough_range_ = addResultRow(jsForm, QStringLiteral("Burnthrough range"));

    // ── Partial-Band results ──────────────────────────────────────────────────
    QFormLayout* pbForm = nullptr;
    auto* pbGroup = makeGroup(QStringLiteral("Partial-Band"), pbForm);

    optimum_bw_  = addResultRow(pbForm, QStringLiteral("Optimum jammer BW"));
    duty_cycle_  = addResultRow(pbForm, QStringLiteral("Duty cycle"));

    // ── Scroll container ──────────────────────────────────────────────────────
    auto* content = new QWidget;
    auto* vbox    = new QVBoxLayout(content);
    vbox->addWidget(sigGroup);
    vbox->addWidget(jamGroup);
    vbox->addWidget(cmGroup);
    vbox->addWidget(jsGroup);
    vbox->addWidget(pbGroup);
    vbox->addStretch();

    auto* scroll = new QScrollArea(this);
    scroll->setWidget(content);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);

    // ── Signal wiring ─────────────────────────────────────────────────────────
    connect(sigErpSb,    &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_signal_erp(v); });
    connect(sigHtSb,     &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_signal_tx_height(v); });
    connect(sigDistSb,   &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_signal_to_rx_dist(v); });
    connect(freqSb,      &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_frequency(v); });
    connect(rxHtSb,      &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_rx_height(v); });
    connect(rxGainSigSb, &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_rx_gain_signal(v); });
    connect(rxGainJamSb, &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_rx_gain_jammer(v); });
    connect(jamErpSb,    &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_jammer_erp(v); });
    connect(jamHtSb,     &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_jammer_height(v); });
    connect(jamDistSb,   &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_jammer_to_rx_dist(v); });
    connect(hopRgSb,     &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_hop_range(v); });
    connect(jsThreshSb,  &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_js_threshold(v); });
    connect(sigBwSb,     &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_signal_bandwidth(v); });

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
    burnthrough_range_->setText(QString::fromStdString(o.burnthrough_range_str));
}
