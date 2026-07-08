// LinkPage.cpp
#include "LinkPage.h"
#include "PageUtils.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

namespace {
const QString kGroup = QStringLiteral("Link");
}

LinkPage::LinkPage(QWidget* parent)
    : QWidget(parent)
{
    // ── Transmitter: Tx power, Tx gain, Frequency (matches macOS) ─────────
    QFormLayout* txForm = nullptr;
    auto* txGroup = makeGroup(QStringLiteral("Transmitter"), txForm);

    auto* txPwrSb = addSpinRow(txForm, QStringLiteral("Tx power (dBm)"), -50.0, 200.0,
        presenter_.tx_power_dbm(), 1.0, 1, kGroup, QStringLiteral("tx_power_dbm"),
        QStringLiteral("Transmitter output power at the antenna port"));
    auto* txGainSb = addSpinRow(txForm, QStringLiteral("Tx gain (dB)"), -30.0, 60.0,
        presenter_.tx_gain_db(), 1.0, 1, kGroup, QStringLiteral("tx_gain_db"),
        QStringLiteral("Transmit antenna gain toward the receiver (dBi)"));
    auto* freqSb = addSpinRow(txForm, QStringLiteral("Frequency (MHz)"), 0.1, 100000.0,
        presenter_.frequency_mhz(), 1.0, 1, kGroup, QStringLiteral("frequency_mhz"),
        QStringLiteral("Carrier frequency \u2014 used to select the propagation regime"));

    // ── Geometry: Distance, Tx height, Rx height (matches macOS) ────────
    QFormLayout* geoForm = nullptr;
    auto* geoGroup = makeGroup(QStringLiteral("Geometry"), geoForm);

    auto* distSb = addSpinRow(geoForm, QStringLiteral("Distance (km)"), 0.01, 10000.0,
        presenter_.distance_km(), 0.1, 3, kGroup, QStringLiteral("distance_km"));
    auto* txHtSb = addSpinRow(geoForm, QStringLiteral("Tx height (m)"), 0.1, 100000.0,
        presenter_.tx_height_m(), 0.5, 1, kGroup, QStringLiteral("tx_height_m"));
    auto* rxHtSb = addSpinRow(geoForm, QStringLiteral("Rx height (m)"), 0.1, 100000.0,
        presenter_.rx_height_m(), 0.5, 1, kGroup, QStringLiteral("rx_height_m"));

    // ── Receiver: Rx gain, Rx sensitivity (matches macOS) ───────────
    QFormLayout* rxForm = nullptr;
    auto* rxGroup = makeGroup(QStringLiteral("Receiver"), rxForm);

    auto* rxGainSb = addSpinRow(rxForm, QStringLiteral("Rx gain (dB)"), -30.0, 60.0,
        presenter_.rx_gain_db(), 1.0, 1, kGroup, QStringLiteral("rx_gain_db"),
        QStringLiteral("Receive antenna gain toward the transmitter (dBi)"));
    auto* rxSensSb = addSpinRow(rxForm, QStringLiteral("Rx sensitivity (dBm)"), -200.0, 0.0,
        presenter_.rx_sensitivity_dbm(), 1.0, 1, kGroup, QStringLiteral("rx_sensitivity_dbm"),
        QStringLiteral("Minimum signal level the receiver can detect; the link closes when received power \u2265 sensitivity"));

    // ── Outputs ───────────────────────────────────────────────────────────────
    QFormLayout* outForm = nullptr;
    auto* outGroup = makeGroup(QStringLiteral("Results"), outForm);

    ResultRowRegistry results;
    received_power_ = addResultRow(outForm, QStringLiteral("Received power"),
        QStringLiteral("Signal power at the receiver input: Ptx + Gtx + Grx \u2212 path loss"), &results);
    path_loss_ = addResultRow(outForm, QStringLiteral("Path loss"), QString(), &results);
    link_margin_ = addResultRow(outForm, QStringLiteral("Link margin"),
        QStringLiteral("Received power minus sensitivity \u2014 positive = link closed; negative = link failure"), &results);
    fresnel_zone_ = addResultRow(outForm, QStringLiteral("Fresnel crossover"),
        QStringLiteral("Range at which ground reflection begins to dominate"), &results);
    regime_ = addResultRow(outForm, QStringLiteral("Regime"), QString(), &results);
    effective_range_ = addResultRow(outForm, QStringLiteral("Effective range"),
        QStringLiteral("Maximum range at which the link closes \u2014 where received power equals sensitivity"), &results);
    range_regime_ = addResultRow(outForm, QStringLiteral("Range regime"),
        QStringLiteral("Propagation model that limits the effective range"), &results);

    outForm->addRow(addCopyResultsButton(results));

    // ── Scroll container ──────────────────────────────────────────────────────────
    auto* content = new QWidget;
    auto* vbox    = new QVBoxLayout(content);
    vbox->addWidget(txGroup);
    vbox->addWidget(geoGroup);
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

    // ── Signal wiring ──────────────────────────────────────────────────────
    connect(txPwrSb,  &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_tx_power(v); });
    connect(txGainSb, &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_tx_gain(v); });
    connect(freqSb,   &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_frequency(v); });
    connect(distSb,   &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_distance(v); });
    connect(txHtSb,   &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_tx_height(v); });
    connect(rxHtSb,   &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_rx_height(v); });
    connect(rxGainSb, &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_rx_gain(v); });
    connect(rxSensSb, &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_rx_sensitivity(v); });

    // ── Restore persisted values (after presenter wiring, before first recompute) ──
    restoreSpinValue(txPwrSb,  kGroup, QStringLiteral("tx_power_dbm"));
    restoreSpinValue(txGainSb, kGroup, QStringLiteral("tx_gain_db"));
    restoreSpinValue(freqSb,   kGroup, QStringLiteral("frequency_mhz"));
    restoreSpinValue(distSb,   kGroup, QStringLiteral("distance_km"));
    restoreSpinValue(txHtSb,   kGroup, QStringLiteral("tx_height_m"));
    restoreSpinValue(rxHtSb,   kGroup, QStringLiteral("rx_height_m"));
    restoreSpinValue(rxGainSb, kGroup, QStringLiteral("rx_gain_db"));
    restoreSpinValue(rxSensSb, kGroup, QStringLiteral("rx_sensitivity_dbm"));

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
