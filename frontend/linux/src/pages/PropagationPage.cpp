// PropagationPage.cpp
#include "PropagationPage.h"
#include "PageUtils.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

namespace {
const QString kGroup = QStringLiteral("Propagation");
}

PropagationPage::PropagationPage(QWidget* parent)
    : QWidget(parent)
{
    // ── Inputs ────────────────────────────────────────────────────────────────
    QFormLayout* inForm = nullptr;
    auto* inGroup = makeGroup(QStringLiteral("Inputs"), inForm);

    auto* distSb = addSpinRow(inForm, QStringLiteral("Distance (km)"), 0.01, 10000.0,
        presenter_.distance_km(), 0.1, 3, kGroup, QStringLiteral("distance_km"),
        QStringLiteral("Total path length between transmitter and receiver"));
    auto* freqSb = addSpinRow(inForm, QStringLiteral("Frequency (MHz)"), 0.1, 100000.0,
        presenter_.frequency_mhz(), 1.0, 1, kGroup, QStringLiteral("frequency_mhz"),
        QStringLiteral("Carrier frequency \u2014 path loss scales as f\u00b2 in free space"));
    auto* txHtSb = addSpinRow(inForm, QStringLiteral("Tx height (m)"), 0.1, 100000.0,
        presenter_.tx_height_m(), 0.5, 1, kGroup, QStringLiteral("tx_height_m"),
        QStringLiteral("Transmit antenna height above ground \u2014 determines the Fresnel zone crossover distance"));
    auto* rxHtSb = addSpinRow(inForm, QStringLiteral("Rx height (m)"), 0.1, 100000.0,
        presenter_.rx_height_m(), 0.5, 1, kGroup, QStringLiteral("rx_height_m"),
        QStringLiteral("Receive antenna height above ground \u2014 determines the Fresnel zone crossover distance"));
    auto* obsSb = addSpinRow(inForm, QStringLiteral("Obstruction height (m)"), 0.0, 10000.0,
        presenter_.obstruction_height_m(), 1.0, 1, kGroup, QStringLiteral("obstruction_height_m"),
        QStringLiteral("Knife-edge obstacle height above flat-earth baseline at path midpoint \u2014 0 = no obstruction"));

    // ── Outputs ───────────────────────────────────────────────────────────────
    QFormLayout* outForm = nullptr;
    auto* outGroup = makeGroup(QStringLiteral("Results"), outForm);

    ResultRowRegistry results;
    fspl_ = addResultRow(outForm, QStringLiteral("FSPL"),
        QStringLiteral("Free-space path loss: 32.44 + 20\u00b7log\u2081\u2080(km) + 20\u00b7log\u2081\u2080(MHz) \u2014 valid below the Fresnel zone crossover"), &results);
    two_ray_loss_ = addResultRow(outForm, QStringLiteral("2-ray loss"),
        QStringLiteral("Two-ray ground-reflection loss: 120 + 40\u00b7log\u2081\u2080(km) \u2212 20\u00b7log\u2081\u2080(h_tx) \u2212 20\u00b7log\u2081\u2080(h_rx) \u2014 loss scales as d\u2074 beyond Fresnel crossover"), &results);
    fresnel_zone_ = addResultRow(outForm, QStringLiteral("Fresnel crossover"),
        QStringLiteral("Range at which ground reflection begins to dominate: h_tx\u00b7h_rx\u00b7f / 24 000 km"), &results);
    path_loss_ = addResultRow(outForm, QStringLiteral("Path loss"),
        QStringLiteral("Applicable loss for this geometry \u2014 FSPL below the Fresnel crossover, 2-ray above it"), &results);
    regime_ = addResultRow(outForm, QStringLiteral("Regime"),
        QStringLiteral("Propagation model in use: LOS (free-space) or 2-ray (ground reflection)"), &results);
    earth_bulge_ = addResultRow(outForm, QStringLiteral("Earth bulge (mid)"),
        QStringLiteral("Height the earth surface rises at the path midpoint under a standard k = 4/3 atmosphere"), &results);
    horizon_range_ = addResultRow(outForm, QStringLiteral("Radar horizon"),
        QStringLiteral("Maximum visibility range combining both antenna heights: 4.12 \u00d7 (\u221ah_tx + \u221ah_rx) km"), &results);
    diffraction_loss_ = addResultRow(outForm, QStringLiteral("Diffraction Loss"),
        QStringLiteral("Knife-edge diffraction loss at path midpoint using the Fresnel diffraction parameter"), &results);

    outForm->addRow(addCopyResultsButton(results));

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
    connect(distSb,  &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_distance(v); });
    connect(freqSb,  &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_frequency(v); });
    connect(txHtSb,  &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_tx_height(v); });
    connect(rxHtSb,  &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_rx_height(v); });
    connect(obsSb,   &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_obstruction_height(v); });

    // ── Restore persisted values (after presenter wiring, before first recompute) ──
    restoreSpinValue(distSb, kGroup, QStringLiteral("distance_km"));
    restoreSpinValue(freqSb, kGroup, QStringLiteral("frequency_mhz"));
    restoreSpinValue(txHtSb, kGroup, QStringLiteral("tx_height_m"));
    restoreSpinValue(rxHtSb, kGroup, QStringLiteral("rx_height_m"));
    restoreSpinValue(obsSb,  kGroup, QStringLiteral("obstruction_height_m"));

    // Presenter fires synchronously on the calling thread; direct update is safe.
    presenter_.set_on_change([this](const ewpresenter::PropagationPresenter::Output& o){
        applyOutput(o);
    });

    // Seed outputs with current defaults.
    applyOutput(presenter_.output());
}

void PropagationPage::applyOutput(const ewpresenter::PropagationPresenter::Output& o)
{
    fspl_->setText(QString::fromStdString(o.fspl_str));
    two_ray_loss_->setText(QString::fromStdString(o.two_ray_loss_str));
    fresnel_zone_->setText(QString::fromStdString(o.fresnel_zone_str));
    path_loss_->setText(QString::fromStdString(o.path_loss_str));
    regime_->setText(QString::fromStdString(o.regime_str));
    earth_bulge_->setText(QString::fromStdString(o.earth_bulge_str));
    horizon_range_->setText(QString::fromStdString(o.horizon_range_str));
    diffraction_loss_->setText(QString::fromStdString(o.diffraction_loss_str));
}
