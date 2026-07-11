// JammingPage.cpp
#include "JammingPage.h"
#include "PageUtils.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

namespace {
const QString kGroup = QStringLiteral("Jamming");
}

JammingPage::JammingPage(QWidget* parent)
    : QWidget(parent)
{
    // ── Signal (Tx) ─────────────────────────────────────────────────────────
    QFormLayout* sigForm = nullptr;
    auto* sigGroup = makeGroup(QStringLiteral("Signal (Tx)"), sigForm);

    auto* sigErpSb = addSpinRow(sigForm, QStringLiteral("Signal ERP (dBm)"), -100.0, 200.0,
        presenter_.signal_erp_dbm(), 1.0, 1, kGroup, QStringLiteral("signal_erp_dbm"),
        QStringLiteral("Effective radiated power of the desired signal: transmitter power + antenna gain"));
    auto* sigHtSb = addSpinRow(sigForm, QStringLiteral("Signal Tx height (m)"), 0.1, 100000.0,
        presenter_.signal_tx_height_m(), 0.5, 1, kGroup, QStringLiteral("signal_tx_height_m"));
    auto* sigDistSb = addSpinRow(sigForm, QStringLiteral("Signal\u2192Rx dist (km)"), 0.01, 10000.0,
        presenter_.signal_to_rx_dist_km(), 0.1, 3, kGroup, QStringLiteral("signal_to_rx_dist_km"));
    auto* sigBwSb = addSpinRow(sigForm, QStringLiteral("Signal BW (MHz)"), 0.001, 1000.0,
        presenter_.signal_bandwidth_mhz(), 0.001, 3, kGroup, QStringLiteral("signal_bandwidth_mhz"),
        QStringLiteral("Occupied bandwidth of the target signal \u2014 used for partial-band jamming optimisation"));

    // ── Jammer ─────────────────────────────────────────────────────────
    QFormLayout* jamForm = nullptr;
    auto* jamGroup = makeGroup(QStringLiteral("Jammer"), jamForm);

    auto* jamErpSb = addSpinRow(jamForm, QStringLiteral("Jammer ERP (dBm)"), -100.0, 200.0,
        presenter_.jammer_erp_dbm(), 1.0, 1, kGroup, QStringLiteral("jammer_erp_dbm"),
        QStringLiteral("Effective radiated power of the jammer toward the receiver"));
    auto* jamHtSb = addSpinRow(jamForm, QStringLiteral("Jammer height (m)"), 0.1, 100000.0,
        presenter_.jammer_height_m(), 0.5, 1, kGroup, QStringLiteral("jammer_height_m"));
    auto* jamDistSb = addSpinRow(jamForm, QStringLiteral("Jammer\u2192Rx dist (km)"), 0.01, 10000.0,
        presenter_.jammer_to_rx_dist_km(), 0.1, 3, kGroup, QStringLiteral("jammer_to_rx_dist_km"));
    auto* hopRgSb = addSpinRow(jamForm, QStringLiteral("Hop range (MHz)"), 0.0, 10000.0,
        presenter_.hop_range_mhz(), 1.0, 1, kGroup, QStringLiteral("hop_range_mhz"),
        QStringLiteral("Total frequency-hopping bandwidth of the target signal \u2014 set to 0 for a non-hopping (single-channel) signal; partial-band results will show N/A"));
    auto* jsThreshSb = addSpinRow(jamForm, QStringLiteral("J/S threshold (dB)"), -30.0, 30.0,
        presenter_.js_threshold_db(), 0.5, 1, kGroup, QStringLiteral("js_threshold_db"),
        QStringLiteral("J/S level at which jamming is considered effective \u2014 used to calculate burnthrough range"));

    // ── Shared ─────────────────────────────────────────────────────────
    QFormLayout* cmForm = nullptr;
    auto* cmGroup = makeGroup(QStringLiteral("Shared"), cmForm);

    auto* freqSb = addSpinRow(cmForm, QStringLiteral("Frequency (MHz)"), 0.1, 100000.0,
        presenter_.frequency_mhz(), 1.0, 1, kGroup, QStringLiteral("frequency_mhz"));
    auto* rxHtSb = addSpinRow(cmForm, QStringLiteral("Rx height (m)"), 0.1, 100000.0,
        presenter_.rx_height_m(), 0.5, 1, kGroup, QStringLiteral("rx_height_m"));
    auto* rxGainSigSb = addSpinRow(cmForm, QStringLiteral("Rx gain \u2192 signal (dB)"), -30.0, 60.0,
        presenter_.rx_gain_signal_db(), 1.0, 1, kGroup, QStringLiteral("rx_gain_signal_db"),
        QStringLiteral("Receive antenna gain toward the signal transmitter \u2014 use the main lobe gain if the receiver antenna is pointed at the signal"));
    auto* rxGainJamSb = addSpinRow(cmForm, QStringLiteral("Rx gain \u2192 jammer (dB)"), -30.0, 60.0,
        presenter_.rx_gain_jammer_db(), 1.0, 1, kGroup, QStringLiteral("rx_gain_jammer_db"),
        QStringLiteral("Receive antenna gain toward the jammer \u2014 a directional antenna with low sidelobes can reject an off-axis jammer by 20\u201330 dB; sidelobes are typically \u221213 to \u221220 dBc"));

    // ── J/S Analysis results ────────────────────────────────────────────────
    QFormLayout* jsForm = nullptr;
    auto* jsGroup = makeGroup(QStringLiteral("J/S Analysis"), jsForm);

    ResultRowRegistry results;
    js_ratio_ = addResultRow(jsForm, QStringLiteral("J/S ratio"),
        QStringLiteral("Jammer-to-signal power ratio at the receiver input (dB)"), &results);
    signal_at_rx_ = addResultRow(jsForm, QStringLiteral("Signal at Rx"), QString(), &results);
    jammer_at_rx_ = addResultRow(jsForm, QStringLiteral("Jammer at Rx"), QString(), &results);
    burnthrough_range_ = addResultRow(jsForm, QStringLiteral("Burnthrough range"),
        QStringLiteral("Signal range at which J/S falls to the threshold \u2014 the jammer becomes ineffective beyond this distance"), &results);

    // ── Partial-Band results ──────────────────────────────────────────────────
    QFormLayout* pbForm = nullptr;
    auto* pbGroup = makeGroup(QStringLiteral("Partial-Band"), pbForm);

    optimum_bw_ = addResultRow(pbForm, QStringLiteral("Optimum jammer BW"),
        QStringLiteral("Narrowing the jamming bandwidth increases instantaneous J/S at the cost of hit probability; this is the optimum trade-off"), &results);
    duty_cycle_ = addResultRow(pbForm, QStringLiteral("Duty cycle"),
        QStringLiteral("Fraction of the hopping band covered by the optimum jamming bandwidth"), &results);

    pbForm->addRow(addCopyResultsButton(results));

    // ── Scroll container ──────────────────────────────────────────────────────────
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
            [this, sigErpSb](double v){ presenter_.set_signal_erp(v); applyFieldError(sigErpSb, presenter_.signal_erp_error()); });
    connect(sigHtSb,     &QDoubleSpinBox::valueChanged, this,
            [this, sigHtSb](double v){ presenter_.set_signal_tx_height(v); applyFieldError(sigHtSb, presenter_.signal_tx_height_error()); });
    connect(sigDistSb,   &QDoubleSpinBox::valueChanged, this,
            [this, sigDistSb](double v){ presenter_.set_signal_to_rx_dist(v); applyFieldError(sigDistSb, presenter_.signal_to_rx_dist_error()); });
    connect(freqSb,      &QDoubleSpinBox::valueChanged, this,
            [this, freqSb](double v){ presenter_.set_frequency(v); applyFieldError(freqSb, presenter_.frequency_error()); });
    connect(rxHtSb,      &QDoubleSpinBox::valueChanged, this,
            [this, rxHtSb](double v){ presenter_.set_rx_height(v); applyFieldError(rxHtSb, presenter_.rx_height_error()); });
    connect(rxGainSigSb, &QDoubleSpinBox::valueChanged, this,
            [this, rxGainSigSb](double v){ presenter_.set_rx_gain_signal(v); applyFieldError(rxGainSigSb, presenter_.rx_gain_signal_error()); });
    connect(rxGainJamSb, &QDoubleSpinBox::valueChanged, this,
            [this, rxGainJamSb](double v){ presenter_.set_rx_gain_jammer(v); applyFieldError(rxGainJamSb, presenter_.rx_gain_jammer_error()); });
    connect(jamErpSb,    &QDoubleSpinBox::valueChanged, this,
            [this, jamErpSb](double v){ presenter_.set_jammer_erp(v); applyFieldError(jamErpSb, presenter_.jammer_erp_error()); });
    connect(jamHtSb,     &QDoubleSpinBox::valueChanged, this,
            [this, jamHtSb](double v){ presenter_.set_jammer_height(v); applyFieldError(jamHtSb, presenter_.jammer_height_error()); });
    connect(jamDistSb,   &QDoubleSpinBox::valueChanged, this,
            [this, jamDistSb](double v){ presenter_.set_jammer_to_rx_dist(v); applyFieldError(jamDistSb, presenter_.jammer_to_rx_dist_error()); });
    connect(hopRgSb,     &QDoubleSpinBox::valueChanged, this,
            [this, hopRgSb](double v){ presenter_.set_hop_range(v); applyFieldError(hopRgSb, presenter_.hop_range_error()); });
    connect(jsThreshSb,  &QDoubleSpinBox::valueChanged, this,
            [this, jsThreshSb](double v){ presenter_.set_js_threshold(v); applyFieldError(jsThreshSb, presenter_.js_threshold_error()); });
    connect(sigBwSb,     &QDoubleSpinBox::valueChanged, this,
            [this, sigBwSb](double v){ presenter_.set_signal_bandwidth(v); applyFieldError(sigBwSb, presenter_.signal_bandwidth_error()); });

    // ── Restore persisted values (after presenter wiring, before first recompute) ──
    restoreSpinValue(sigErpSb,    kGroup, QStringLiteral("signal_erp_dbm"));
    restoreSpinValue(sigHtSb,     kGroup, QStringLiteral("signal_tx_height_m"));
    restoreSpinValue(sigDistSb,   kGroup, QStringLiteral("signal_to_rx_dist_km"));
    restoreSpinValue(sigBwSb,     kGroup, QStringLiteral("signal_bandwidth_mhz"));
    restoreSpinValue(jamErpSb,    kGroup, QStringLiteral("jammer_erp_dbm"));
    restoreSpinValue(jamHtSb,     kGroup, QStringLiteral("jammer_height_m"));
    restoreSpinValue(jamDistSb,   kGroup, QStringLiteral("jammer_to_rx_dist_km"));
    restoreSpinValue(hopRgSb,     kGroup, QStringLiteral("hop_range_mhz"));
    restoreSpinValue(jsThreshSb,  kGroup, QStringLiteral("js_threshold_db"));
    restoreSpinValue(freqSb,      kGroup, QStringLiteral("frequency_mhz"));
    restoreSpinValue(rxHtSb,      kGroup, QStringLiteral("rx_height_m"));
    restoreSpinValue(rxGainSigSb, kGroup, QStringLiteral("rx_gain_signal_db"));
    restoreSpinValue(rxGainJamSb, kGroup, QStringLiteral("rx_gain_jammer_db"));

    presenter_.set_on_change([this](const ewpresenter::JammingPresenter::Output& o){
        applyOutput(o);
    });

    // Seed per-field validation-error styling from initial/restored values (#41).
    applyFieldError(sigErpSb,    presenter_.signal_erp_error());
    applyFieldError(sigHtSb,     presenter_.signal_tx_height_error());
    applyFieldError(sigDistSb,   presenter_.signal_to_rx_dist_error());
    applyFieldError(sigBwSb,     presenter_.signal_bandwidth_error());
    applyFieldError(jamErpSb,    presenter_.jammer_erp_error());
    applyFieldError(jamHtSb,     presenter_.jammer_height_error());
    applyFieldError(jamDistSb,   presenter_.jammer_to_rx_dist_error());
    applyFieldError(hopRgSb,     presenter_.hop_range_error());
    applyFieldError(jsThreshSb,  presenter_.js_threshold_error());
    applyFieldError(freqSb,      presenter_.frequency_error());
    applyFieldError(rxHtSb,      presenter_.rx_height_error());
    applyFieldError(rxGainSigSb, presenter_.rx_gain_signal_error());
    applyFieldError(rxGainJamSb, presenter_.rx_gain_jammer_error());

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
