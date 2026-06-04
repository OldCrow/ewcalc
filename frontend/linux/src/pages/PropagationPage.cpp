// PropagationPage.cpp
#include "PropagationPage.h"
#include "PageUtils.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

PropagationPage::PropagationPage(QWidget* parent)
    : QWidget(parent)
{
    // ── Inputs ────────────────────────────────────────────────────────────────
    QFormLayout* inForm = nullptr;
    auto* inGroup = makeGroup(QStringLiteral("Inputs"), inForm);

    auto* distSb    = addSpinRow(inForm, QStringLiteral("Distance (km)"),       0.01, 10000.0,  presenter_.distance_km(),          0.1, 3);
    auto* freqSb    = addSpinRow(inForm, QStringLiteral("Frequency (MHz)"),     0.1,  100000.0, presenter_.frequency_mhz(),        1.0, 1);
    auto* txHtSb    = addSpinRow(inForm, QStringLiteral("Tx height (m)"),       0.1,  100000.0, presenter_.tx_height_m(),          0.5, 1);
    auto* rxHtSb    = addSpinRow(inForm, QStringLiteral("Rx height (m)"),       0.1,  100000.0, presenter_.rx_height_m(),          0.5, 1);
    auto* obsSb     = addSpinRow(inForm, QStringLiteral("Obstruction height (m)"), 0.0, 10000.0, presenter_.obstruction_height_m(), 1.0, 1);

    // ── Outputs ───────────────────────────────────────────────────────────────
    QFormLayout* outForm = nullptr;
    auto* outGroup = makeGroup(QStringLiteral("Results"), outForm);

    fspl_        = addResultRow(outForm, QStringLiteral("FSPL"));
    two_ray_loss_= addResultRow(outForm, QStringLiteral("2-ray loss"));
    fresnel_zone_= addResultRow(outForm, QStringLiteral("Fresnel crossover"));
    path_loss_   = addResultRow(outForm, QStringLiteral("Path loss"));
    regime_      = addResultRow(outForm, QStringLiteral("Regime"));
    earth_bulge_     = addResultRow(outForm, QStringLiteral("Earth bulge (mid)"));
    horizon_range_   = addResultRow(outForm, QStringLiteral("Radar horizon"));
    diffraction_loss_= addResultRow(outForm, QStringLiteral("Diffraction Loss"));

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
