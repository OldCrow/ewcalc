// LocationPage.cpp
#include "LocationPage.h"
#include "PageUtils.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

LocationPage::LocationPage(QWidget* parent)
    : QWidget(parent)
{
    // ── AOA inputs ────────────────────────────────────────────────────────────
    QFormLayout* aoaForm = nullptr;
    auto* aoaGroup = makeGroup(QStringLiteral("AOA (Angle of Arrival)"), aoaForm);

    auto* rmsBeSb = addSpinRow(aoaForm, QStringLiteral("RMS bearing error (\u00b0)"), 0.01, 45.0,    presenter_.rms_bearing_error_deg(), 0.1, 2);
    auto* aoaRgSb = addSpinRow(aoaForm, QStringLiteral("Range (km)"),              0.1,  10000.0, presenter_.aoa_range_km(),          1.0, 1);

    // ── TDOA inputs ───────────────────────────────────────────────────────────────
    QFormLayout* tdoaForm = nullptr;
    auto* tdoaGroup = makeGroup(QStringLiteral("TDOA (Time Difference of Arrival)"), tdoaForm);

    auto* rmsTimeSb  = addSpinRow(tdoaForm, QStringLiteral("RMS timing error (ns)"), 0.001, 100000.0, presenter_.rms_time_error_ns(), 1.0, 3);
    auto* baselineSb = addSpinRow(tdoaForm, QStringLiteral("Baseline (km)"),         0.1,   10000.0,  presenter_.baseline_km(),       1.0, 1);

    // ── EEP inputs ───────────────────────────────────────────────────────────────
    QFormLayout* eepForm = nullptr;
    auto* eepGroup = makeGroup(QStringLiteral("EEP (Error Ellipse \u2192 CEP)"), eepForm);

    auto* semiMajSb = addSpinRow(eepForm, QStringLiteral("Semi-major 1\u03c3 (km)"), 0.001, 1000.0, presenter_.semi_major_km(), 0.1, 3);
    auto* semiMinSb = addSpinRow(eepForm, QStringLiteral("Semi-minor 1\u03c3 (km)"), 0.001, 1000.0, presenter_.semi_minor_km(), 0.1, 3);

    // ── Outputs ───────────────────────────────────────────────────────────────
    QFormLayout* outForm = nullptr;
    auto* outGroup = makeGroup(QStringLiteral("Results"), outForm);

    cep_aoa_  = addResultRow(outForm, QStringLiteral("CEP (AOA)"));
    cep_tdoa_ = addResultRow(outForm, QStringLiteral("CEP (TDOA)"));
    cep_eep_  = addResultRow(outForm, QStringLiteral("CEP (EEP)"));

    // ── Scroll container ──────────────────────────────────────────────────────
    auto* content = new QWidget;
    auto* vbox    = new QVBoxLayout(content);
    vbox->addWidget(aoaGroup);
    vbox->addWidget(tdoaGroup);
    vbox->addWidget(eepGroup);
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
    connect(rmsBeSb,   &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_rms_bearing_error(v); });
    connect(aoaRgSb,   &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_aoa_range(v); });
    connect(rmsTimeSb, &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_rms_time_error(v); });
    connect(baselineSb,&QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_baseline(v); });
    connect(semiMajSb, &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_semi_major(v); });
    connect(semiMinSb, &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_semi_minor(v); });

    presenter_.set_on_change([this](const ewpresenter::LocationPresenter::Output& o){
        applyOutput(o);
    });

    applyOutput(presenter_.output());
}

void LocationPage::applyOutput(const ewpresenter::LocationPresenter::Output& o)
{
    cep_aoa_->setText(QString::fromStdString(o.cep_aoa_str));
    cep_tdoa_->setText(QString::fromStdString(o.cep_tdoa_str));
    cep_eep_->setText(QString::fromStdString(o.cep_eep_str));
}
