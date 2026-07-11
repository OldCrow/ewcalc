// LocationPage.cpp
#include "LocationPage.h"
#include "PageUtils.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

namespace {
const QString kGroup = QStringLiteral("Location");
}

LocationPage::LocationPage(QWidget* parent)
    : QWidget(parent)
{
    // ── AOA inputs ────────────────────────────────────────────────────────────────
    QFormLayout* aoaForm = nullptr;
    auto* aoaGroup = makeGroup(QStringLiteral("AOA (Angle of Arrival)"), aoaForm);

    auto* rmsBeSb = addSpinRow(aoaForm, QStringLiteral("RMS bearing error (\u00b0)"), 0.01, 45.0,
        presenter_.rms_bearing_error_deg(), 0.1, 2, kGroup, QStringLiteral("rms_bearing_error_deg"),
        QStringLiteral("RMS angular error of the direction-finding receivers"));
    auto* aoaRgSb = addSpinRow(aoaForm, QStringLiteral("Range (km)"), 0.1, 10000.0,
        presenter_.aoa_range_km(), 1.0, 1, kGroup, QStringLiteral("aoa_range_km"),
        QStringLiteral("Slant range from receivers to emitter \u2014 shared by AOA and TDOA calculations"));

    // ── TDOA inputs ─────────────────────────────────────────────────────────
    QFormLayout* tdoaForm = nullptr;
    auto* tdoaGroup = makeGroup(QStringLiteral("TDOA (Time Difference of Arrival)"), tdoaForm);

    auto* rmsTimeSb = addSpinRow(tdoaForm, QStringLiteral("RMS timing error (ns)"), 0.001, 100000.0,
        presenter_.rms_time_error_ns(), 1.0, 3, kGroup, QStringLiteral("rms_time_error_ns"),
        QStringLiteral("RMS TDOA measurement error \u2014 converts to a range-difference error via speed of light"));
    auto* baselineSb = addSpinRow(tdoaForm, QStringLiteral("Baseline (km)"), 0.1, 10000.0,
        presenter_.baseline_km(), 1.0, 1, kGroup, QStringLiteral("baseline_km"),
        QStringLiteral("Receiver separation distance \u2014 wider baseline reduces CEP: CEP = c\u00b7\u03c3_t\u00b7R / (2\u00b7B)"));

    // ── EEP inputs ─────────────────────────────────────────────────────────
    QFormLayout* eepForm = nullptr;
    auto* eepGroup = makeGroup(QStringLiteral("EEP (Error Ellipse \u2192 CEP)"), eepForm);

    auto* semiMajSb = addSpinRow(eepForm, QStringLiteral("Semi-major 1\u03c3 (km)"), 0.001, 1000.0,
        presenter_.semi_major_km(), 0.1, 3, kGroup, QStringLiteral("semi_major_km"),
        QStringLiteral("Semi-major axis of the 1\u03c3 error ellipse (must be \u2265 semi-minor)"));
    auto* semiMinSb = addSpinRow(eepForm, QStringLiteral("Semi-minor 1\u03c3 (km)"), 0.001, 1000.0,
        presenter_.semi_minor_km(), 0.1, 3, kGroup, QStringLiteral("semi_minor_km"),
        QStringLiteral("Semi-minor axis of the 1\u03c3 error ellipse"));

    // ── Outputs ───────────────────────────────────────────────────────────────
    QFormLayout* outForm = nullptr;
    auto* outGroup = makeGroup(QStringLiteral("Results"), outForm);

    ResultRowRegistry results;
    cep_aoa_ = addResultRow(outForm, QStringLiteral("CEP (AOA)"),
        QStringLiteral("50% Circular Error Probable from angle-of-arrival: 1.2 \u00d7 range \u00d7 tan(RMS error)"), &results);
    cep_tdoa_ = addResultRow(outForm, QStringLiteral("CEP (TDOA)"),
        QStringLiteral("50% Circular Error Probable from TDOA: c\u00b7\u03c3_t\u00b7R / (2\u00b7B) \u2014 improves with wider baseline or shorter range"), &results);
    cep_eep_ = addResultRow(outForm, QStringLiteral("CEP (EEP)"),
        QStringLiteral("CEP from an Elliptical Error Probable: 0.59 \u00d7 (semi-major + semi-minor)"), &results);

    outForm->addRow(addCopyResultsButton(results));

    // ── Scroll container ──────────────────────────────────────────────────────────
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
    // Semi-minor's displayed error combines its own bounds error with
    // eep_axis_error(), a cross-field check (semi-minor must not exceed
    // semi-major) that range clamping alone can't express — mirrors the
    // macOS LocationView / Windows LocationPage treatment.
    auto updateSemiMinorError = [this, semiMinSb] {
        auto err = presenter_.semi_minor_error();
        if (err == ewpresenter::FieldError::none) err = presenter_.eep_axis_error();
        applyFieldError(semiMinSb, err);
    };

    connect(rmsBeSb,   &QDoubleSpinBox::valueChanged, this,
            [this, rmsBeSb](double v){ presenter_.set_rms_bearing_error(v); applyFieldError(rmsBeSb, presenter_.rms_bearing_error()); });
    connect(aoaRgSb,   &QDoubleSpinBox::valueChanged, this,
            [this, aoaRgSb](double v){ presenter_.set_aoa_range(v); applyFieldError(aoaRgSb, presenter_.aoa_range_error()); });
    connect(rmsTimeSb, &QDoubleSpinBox::valueChanged, this,
            [this, rmsTimeSb](double v){ presenter_.set_rms_time_error(v); applyFieldError(rmsTimeSb, presenter_.rms_time_error()); });
    connect(baselineSb,&QDoubleSpinBox::valueChanged, this,
            [this, baselineSb](double v){ presenter_.set_baseline(v); applyFieldError(baselineSb, presenter_.baseline_error()); });
    connect(semiMajSb, &QDoubleSpinBox::valueChanged, this,
            [this, semiMajSb, updateSemiMinorError](double v){
                presenter_.set_semi_major(v);
                applyFieldError(semiMajSb, presenter_.semi_major_error());
                updateSemiMinorError();
            });
    connect(semiMinSb, &QDoubleSpinBox::valueChanged, this,
            [this, updateSemiMinorError](double v){ presenter_.set_semi_minor(v); updateSemiMinorError(); });

    // ── Restore persisted values (after presenter wiring, before first recompute) ──
    restoreSpinValue(rmsBeSb,    kGroup, QStringLiteral("rms_bearing_error_deg"));
    restoreSpinValue(aoaRgSb,    kGroup, QStringLiteral("aoa_range_km"));
    restoreSpinValue(rmsTimeSb,  kGroup, QStringLiteral("rms_time_error_ns"));
    restoreSpinValue(baselineSb, kGroup, QStringLiteral("baseline_km"));
    restoreSpinValue(semiMajSb,  kGroup, QStringLiteral("semi_major_km"));
    restoreSpinValue(semiMinSb,  kGroup, QStringLiteral("semi_minor_km"));

    presenter_.set_on_change([this](const ewpresenter::LocationPresenter::Output& o){
        applyOutput(o);
    });

    // Seed per-field validation-error styling from initial/restored values (#41).
    applyFieldError(rmsBeSb,    presenter_.rms_bearing_error());
    applyFieldError(aoaRgSb,    presenter_.aoa_range_error());
    applyFieldError(rmsTimeSb,  presenter_.rms_time_error());
    applyFieldError(baselineSb, presenter_.baseline_error());
    applyFieldError(semiMajSb,  presenter_.semi_major_error());
    updateSemiMinorError();

    applyOutput(presenter_.output());
}

void LocationPage::applyOutput(const ewpresenter::LocationPresenter::Output& o)
{
    cep_aoa_->setText(QString::fromStdString(o.cep_aoa_str));
    cep_tdoa_->setText(QString::fromStdString(o.cep_tdoa_str));
    cep_eep_->setText(QString::fromStdString(o.cep_eep_str));
}
