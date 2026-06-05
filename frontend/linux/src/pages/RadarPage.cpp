// RadarPage.cpp
#include "RadarPage.h"
#include "PageUtils.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>

RadarPage::RadarPage(QWidget* parent)
    : QWidget(parent)
{
    // ── Radar range inputs ────────────────────────────────────────────────────
    QFormLayout* inForm = nullptr;
    auto* inGroup = makeGroup(QStringLiteral("Radar Parameters"), inForm);

    auto* txPwrSb  = addSpinRow(inForm, QStringLiteral("Tx power (dBm)"),     -50.0,  200.0,    presenter_.tx_power_dbm(),    1.0, 1);
    auto* antGainSb= addSpinRow(inForm, QStringLiteral("Antenna gain (dBi)"), -30.0,   60.0,    presenter_.antenna_gain_dbi(),1.0, 1);
    auto* rcsSb    = addSpinRow(inForm, QStringLiteral("Target RCS (dBsm)"),  -40.0,   60.0,    presenter_.target_rcs_dbsm(), 1.0, 1);
    auto* freqSb   = addSpinRow(inForm, QStringLiteral("Frequency (MHz)"),      1.0, 100000.0,  presenter_.frequency_mhz(),   1.0, 1);
    auto* losseSb  = addSpinRow(inForm, QStringLiteral("System losses (dB)"),   0.0,   30.0,    presenter_.system_losses_db(),0.5, 1);
    auto* nfSb     = addSpinRow(inForm, QStringLiteral("Noise figure (dB)"),    0.0,   30.0,    presenter_.noise_figure_db(), 0.5, 1);
    auto* bwSb     = addSpinRow(inForm, QStringLiteral("Bandwidth (MHz)"),      0.001, 10000.0, presenter_.bandwidth_mhz(),   0.1, 3);
    auto* snrSb    = addSpinRow(inForm, QStringLiteral("Required SNR (dB)"),  -10.0,   50.0,    presenter_.required_snr_db(), 0.5, 1);

    // ── Signal processing inputs ──────────────────────────────────────────────────
    QFormLayout* spForm = nullptr;
    auto* spGroup = makeGroup(QStringLiteral("Signal Processing"), spForm);

    auto* tbSb = addSpinRow(spForm, QStringLiteral("Time-BW product"), 1.0, 1000000.0, presenter_.time_bandwidth_product(), 10.0, 0);
    auto* npSb = addIntSpinRow(spForm, QStringLiteral("Coherent pulses"), 1, 100000, presenter_.num_pulses());

    // ── Outputs ───────────────────────────────────────────────────────────────
    QFormLayout* outForm = nullptr;
    auto* outGroup = makeGroup(QStringLiteral("Results"), outForm);

    max_range_    = addResultRow(outForm, QStringLiteral("Max range"));
    two_way_loss_ = addResultRow(outForm, QStringLiteral("Two-way loss"));
    target_rcs_   = addResultRow(outForm, QStringLiteral("Target RCS"));
    pc_gain_      = addResultRow(outForm, QStringLiteral("PC gain"));
    ci_gain_      = addResultRow(outForm, QStringLiteral("Coherent integration gain"));
    lpi_advantage_= addResultRow(outForm, QStringLiteral("LPI advantage"));

    // ── Scroll container ──────────────────────────────────────────────────────
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
            [this](double v){ presenter_.set_tx_power(v); });
    connect(antGainSb,&QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_antenna_gain(v); });
    connect(rcsSb,    &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_target_rcs(v); });
    connect(freqSb,   &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_frequency(v); });
    connect(losseSb,  &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_system_losses(v); });
    connect(nfSb,     &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_noise_figure(v); });
    connect(bwSb,     &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_bandwidth(v); });
    connect(snrSb,    &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_required_snr(v); });
    connect(tbSb,     &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_time_bandwidth_product(v); });
    connect(npSb, &QSpinBox::valueChanged, this,
            [this](int v){ presenter_.set_num_pulses(v); });

    presenter_.set_on_change([this](const ewpresenter::RadarPresenter::Output& o){
        applyOutput(o);
    });

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
