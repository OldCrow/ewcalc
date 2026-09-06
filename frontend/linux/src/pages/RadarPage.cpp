// RadarPage.cpp
#include "RadarPage.h"
#include "PageUtils.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>

namespace {
const QString kGroup = QStringLiteral("Radar");
}

RadarPage::RadarPage(QWidget* parent)
    : QWidget(parent)
{
    // ── Radar range inputs ───────────────────────────────────────────
    QFormLayout* inForm = nullptr;
    auto* inGroup = makeGroup(QStringLiteral("Radar Parameters"), inForm);

    auto* txPwrSb = addSpinRow(inForm, QStringLiteral("Tx power (dBm)"), -50.0, 200.0,
        presenter_.tx_power_dbm(), 1.0, 1, kGroup, QStringLiteral("tx_power_dbm"),
        QStringLiteral("Transmitter output power at the antenna port"));
    auto* antGainSb = addSpinRow(inForm, QStringLiteral("Antenna gain (dBi)"), -30.0, 60.0,
        presenter_.antenna_gain_dbi(), 1.0, 1, kGroup, QStringLiteral("antenna_gain_dbi"),
        QStringLiteral("Same antenna used for transmit and receive \u2014 appears squared in the radar range equation"));
    auto* rcsSb = addSpinRow(inForm, QStringLiteral("Target RCS (dBsm)"), -40.0, 60.0,
        presenter_.target_rcs_dbsm(), 1.0, 1, kGroup, QStringLiteral("target_rcs_dbsm"),
        QStringLiteral("Radar cross-section of the target (dB relative to 1 m\u00b2) \u2014 fighter ~0 to +10 dBsm, missile \u221210 dBsm"));
    auto* freqSb = addSpinRow(inForm, QStringLiteral("Frequency (MHz)"), 1.0, 100000.0,
        presenter_.frequency_mhz(), 1.0, 1, kGroup, QStringLiteral("frequency_mhz"),
        QStringLiteral("Radar carrier frequency"));
    auto* losseSb = addSpinRow(inForm, QStringLiteral("System losses (dB)"), 0.0, 30.0,
        presenter_.system_losses_db(), 0.5, 1, kGroup, QStringLiteral("system_losses_db"),
        QStringLiteral("Combined one-way losses: feed line, atmospheric absorption, beam-shape, etc."));
    auto* nfSb = addSpinRow(inForm, QStringLiteral("Noise figure (dB)"), 0.0, 30.0,
        presenter_.noise_figure_db(), 0.5, 1, kGroup, QStringLiteral("noise_figure_db"),
        QStringLiteral("Receiver noise figure \u2014 NF = 0 dB is ideal (noiseless); each additional dB raises the sensitivity floor"));
    auto* bwSb = addSpinRow(inForm, QStringLiteral("Bandwidth (MHz)"), 0.001, 10000.0,
        presenter_.bandwidth_mhz(), 0.1, 3, kGroup, QStringLiteral("bandwidth_mhz"),
        QStringLiteral("Matched-filter noise bandwidth \u2014 sets the thermal noise floor"));
    auto* snrSb = addSpinRow(inForm, QStringLiteral("Required SNR (dB)"), -10.0, 50.0,
        presenter_.required_snr_db(), 0.5, 1, kGroup, QStringLiteral("required_snr_db"),
        QStringLiteral("Minimum SNR for the desired probability of detection / false-alarm rate"));

    // ── Signal processing inputs ──────────────────────────────────────────────
    QFormLayout* spForm = nullptr;
    auto* spGroup = makeGroup(QStringLiteral("Signal Processing"), spForm);

    auto* tbSb = addSpinRow(spForm, QStringLiteral("Time-BW product"), 1.0, 1000000.0,
        presenter_.time_bandwidth_product(), 10.0, 0, kGroup, QStringLiteral("time_bandwidth_product"),
        QStringLiteral("Pulse compression ratio: pulse width \u00d7 bandwidth \u2014 determines PC gain and LPI advantage"));
    auto* npSb = addIntSpinRow(spForm, QStringLiteral("Coherent pulses"), 1, 100000,
        presenter_.num_pulses(), kGroup, QStringLiteral("num_pulses"),
        QStringLiteral("Number of pulses combined coherently \u2014 sets the coherent integration gain"));

    // ── Outputs ───────────────────────────────────────────────────────────────
    QFormLayout* outForm = nullptr;
    auto* outGroup = makeGroup(QStringLiteral("Results"), outForm);

    ResultRowRegistry results;
    max_range_ = addResultRow(outForm, QStringLiteral("Max range"),
        QStringLiteral("Maximum detection range at the required SNR with no signal processing gain applied"), &results);
    two_way_loss_ = addResultRow(outForm, QStringLiteral("Two-way loss"),
        QStringLiteral("Total two-way propagation loss at the maximum range"), &results);
    target_rcs_ = addResultRow(outForm, QStringLiteral("Target RCS"),
        QStringLiteral("Target radar cross-section (dB relative to 1 m\u00b2) \u2014 shown here with dBsm suffix so the result panel is self-contained"), &results);
    pc_gain_ = addResultRow(outForm, QStringLiteral("PC gain"),
        QStringLiteral("Pulse compression SNR gain: 10\u00b7log\u2081\u2080(time-bandwidth product)"), &results);
    ci_gain_ = addResultRow(outForm, QStringLiteral("Coherent integration gain"),
        QStringLiteral("Coherent integration SNR gain: 10\u00b7log\u2081\u2080(number of pulses)"), &results);
    lpi_advantage_ = addResultRow(outForm, QStringLiteral("LPI advantage"),
        QStringLiteral("Detection-range advantage an LPI waveform has against an intercept receiver: PC gain \u00f7 4"), &results);

    outForm->addRow(addCopyResultsButton(results));

    // ── Scroll container ──────────────────────────────────────────────────────────
    auto* content = new QWidget;
    auto* vbox    = new QVBoxLayout(content);
    vbox->addWidget(inGroup);
    vbox->addWidget(spGroup);
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
            [this, txPwrSb](double v){ presenter_.set_tx_power(v); applyFieldError(txPwrSb, presenter_.tx_power_error()); });
    connect(antGainSb,&QDoubleSpinBox::valueChanged, this,
            [this, antGainSb](double v){ presenter_.set_antenna_gain(v); applyFieldError(antGainSb, presenter_.antenna_gain_error()); });
    connect(rcsSb,    &QDoubleSpinBox::valueChanged, this,
            [this, rcsSb](double v){ presenter_.set_target_rcs(v); applyFieldError(rcsSb, presenter_.target_rcs_error()); });
    connect(freqSb,   &QDoubleSpinBox::valueChanged, this,
            [this, freqSb](double v){ presenter_.set_frequency(v); applyFieldError(freqSb, presenter_.frequency_error()); });
    connect(losseSb,  &QDoubleSpinBox::valueChanged, this,
            [this, losseSb](double v){ presenter_.set_system_losses(v); applyFieldError(losseSb, presenter_.system_losses_error()); });
    connect(nfSb,     &QDoubleSpinBox::valueChanged, this,
            [this, nfSb](double v){ presenter_.set_noise_figure(v); applyFieldError(nfSb, presenter_.noise_figure_error()); });
    connect(bwSb,     &QDoubleSpinBox::valueChanged, this,
            [this, bwSb](double v){ presenter_.set_bandwidth(v); applyFieldError(bwSb, presenter_.bandwidth_error()); });
    connect(snrSb,    &QDoubleSpinBox::valueChanged, this,
            [this, snrSb](double v){ presenter_.set_required_snr(v); applyFieldError(snrSb, presenter_.required_snr_error()); });
    connect(tbSb,     &QDoubleSpinBox::valueChanged, this,
            [this, tbSb](double v){ presenter_.set_time_bandwidth_product(v); applyFieldError(tbSb, presenter_.time_bandwidth_product_error()); });
    connect(npSb, &QSpinBox::valueChanged, this,
            [this, npSb](int v){ presenter_.set_num_pulses(v); applyFieldError(npSb, presenter_.num_pulses_error()); });

    // ── Restore persisted values (after presenter wiring, before first recompute) ──
    restoreSpinValue(txPwrSb,   kGroup, QStringLiteral("tx_power_dbm"));
    restoreSpinValue(antGainSb, kGroup, QStringLiteral("antenna_gain_dbi"));
    restoreSpinValue(rcsSb,     kGroup, QStringLiteral("target_rcs_dbsm"));
    restoreSpinValue(freqSb,    kGroup, QStringLiteral("frequency_mhz"));
    restoreSpinValue(losseSb,   kGroup, QStringLiteral("system_losses_db"));
    restoreSpinValue(nfSb,      kGroup, QStringLiteral("noise_figure_db"));
    restoreSpinValue(bwSb,      kGroup, QStringLiteral("bandwidth_mhz"));
    restoreSpinValue(snrSb,     kGroup, QStringLiteral("required_snr_db"));
    restoreSpinValue(tbSb,      kGroup, QStringLiteral("time_bandwidth_product"));
    restoreSpinValue(npSb,      kGroup, QStringLiteral("num_pulses"));

    presenter_.set_on_change([this](const ewpresenter::RadarPresenter::Output& o){
        applyOutput(o);
    });

    // Seed per-field validation-error styling from initial/restored values (#41).
    applyFieldError(txPwrSb,   presenter_.tx_power_error());
    applyFieldError(antGainSb, presenter_.antenna_gain_error());
    applyFieldError(rcsSb,     presenter_.target_rcs_error());
    applyFieldError(freqSb,    presenter_.frequency_error());
    applyFieldError(losseSb,   presenter_.system_losses_error());
    applyFieldError(nfSb,      presenter_.noise_figure_error());
    applyFieldError(bwSb,      presenter_.bandwidth_error());
    applyFieldError(snrSb,     presenter_.required_snr_error());
    applyFieldError(tbSb,      presenter_.time_bandwidth_product_error());
    applyFieldError(npSb,      presenter_.num_pulses_error());

    applyOutput(presenter_.output());
}

void RadarPage::applyOutput(const ewpresenter::RadarPresenter::Output& o)
{
    max_range_->setText(QString::fromStdString(o.max_range_str));
    two_way_loss_->setText(QString::fromStdString(o.two_way_loss_str));
    target_rcs_->setText(QString::fromStdString(o.target_rcs_str));
    pc_gain_->setText(QString::fromStdString(o.pulse_compression_gain_str));
    ci_gain_->setText(QString::fromStdString(o.coherent_integration_gain_str));
    lpi_advantage_->setText(QString::fromStdString(o.lpi_advantage_str));
}
