// DetectionPage.cpp
#include "DetectionPage.h"
#include "PageUtils.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>

namespace {
const QString kGroup = QStringLiteral("Detection");
}

DetectionPage::DetectionPage(QWidget* parent)
    : QWidget(parent)
{
    ResultRowRegistry results;

    // ── Detection statistics ──────────────────────────────────────────────────
    QFormLayout* statForm = nullptr;
    auto* statGroup = makeGroup(QStringLiteral("Detection statistics"), statForm);

    auto* pdSb = addSpinRow(statForm, QStringLiteral("Probability of detection"), 0.1, 0.99,
        presenter_.pd(), 0.01, 2, kGroup, QStringLiteral("pd"),
        QStringLiteral("Single-scan probability of detection \u2014 Shnidman's equation is valid for 0.1 to 0.99"));
    auto* pfaSb = addSpinRow(statForm, QStringLiteral("Pfa exponent (10^x)"), -9.0, -3.0,
        presenter_.pfa_exponent(), 1.0, 0, kGroup, QStringLiteral("pfa_exponent"),
        QStringLiteral("False-alarm probability exponent x, Pfa = 10^x \u2014 e.g. \u22126 for Pfa = 10\u207b\u2076"));
    auto* npSb = addIntSpinRow(statForm, QStringLiteral("Pulses integrated"), 1, 100,
        presenter_.num_pulses(), kGroup, QStringLiteral("num_pulses"),
        QStringLiteral("Pulses noncoherently integrated \u2014 Shnidman's equation is valid for 1 to 100"));
    auto* swSb = addIntSpinRow(statForm, QStringLiteral("Swerling case (0\u20134)"), 0, 4,
        presenter_.swerling_case(), kGroup, QStringLiteral("swerling_case"),
        QStringLiteral("Target fluctuation model: 0 nonfluctuating, 1\u20134 the Swerling cases"));

    required_snr_ = addResultRow(statForm, QStringLiteral("Required SNR (Shnidman)"),
        QStringLiteral("Single-pulse SNR required for the chosen Pd/Pfa and Swerling case, after noncoherent integration (Shnidman's equation)"), &results);
    required_snr_albersheim_ = addResultRow(statForm, QStringLiteral("Required SNR (Albersheim, Sw0)"),
        QStringLiteral("Nonfluctuating-target reference from Albersheim's equation \u2014 compare against the Shnidman value to see the fluctuation penalty"), &results);
    fluctuation_loss_ = addResultRow(statForm, QStringLiteral("Fluctuation loss"),
        QStringLiteral("Extra SNR the fluctuating target needs over the nonfluctuating (Swerling 0) case"), &results);

    // ── Scan timing ───────────────────────────────────────────────────────────
    QFormLayout* scanForm = nullptr;
    auto* scanGroup = makeGroup(QStringLiteral("Scan timing"), scanForm);

    auto* bwDegSb = addSpinRow(scanForm, QStringLiteral("Azimuth beamwidth (deg)"), 0.1, 45.0,
        presenter_.beamwidth_deg(), 0.5, 1, kGroup, QStringLiteral("beamwidth_deg"),
        QStringLiteral("Antenna 3 dB azimuth beamwidth"));
    auto* scanSb = addSpinRow(scanForm, QStringLiteral("Scan rate (deg/s)"), 1.0, 720.0,
        presenter_.scan_rate_deg_s(), 1.0, 1, kGroup, QStringLiteral("scan_rate_deg_s"),
        QStringLiteral("Antenna rotation rate \u2014 36 deg/s is a 10 s rotation"));
    auto* prfSb = addSpinRow(scanForm, QStringLiteral("PRF (Hz)"), 10.0, 1000000.0,
        presenter_.prf_hz(), 10.0, 0, kGroup, QStringLiteral("prf_hz"),
        QStringLiteral("Pulse repetition frequency"));

    dwell_time_ = addResultRow(scanForm, QStringLiteral("Dwell time"),
        QStringLiteral("Time the beam illuminates a point target each scan: beamwidth \u00f7 scan rate"), &results);
    hits_per_scan_ = addResultRow(scanForm, QStringLiteral("Hits per scan"),
        QStringLiteral("Pulses on target per scan: dwell time \u00d7 PRF"), &results);

    // ── False-alarm rate ──────────────────────────────────────────────────────
    QFormLayout* farForm = nullptr;
    auto* farGroup = makeGroup(QStringLiteral("False-alarm rate"), farForm);

    auto* nbwSb = addSpinRow(farForm, QStringLiteral("Noise bandwidth (MHz)"), 0.001, 10000.0,
        presenter_.bandwidth_mhz(), 0.1, 3, kGroup, QStringLiteral("bandwidth_mhz"),
        QStringLiteral("Receiver noise bandwidth \u2014 with Pfa, sets the false-alarm rate"));

    far_ = addResultRow(farForm, QStringLiteral("False-alarm rate"),
        QStringLiteral("Average false alarms per second: Pfa \u00d7 noise bandwidth"), &results);

    farForm->addRow(addCopyResultsButton(results));

    // ── Scroll container ──────────────────────────────────────────────────────────
    auto* content = new QWidget;
    auto* vbox    = new QVBoxLayout(content);
    vbox->addWidget(statGroup);
    vbox->addWidget(scanGroup);
    vbox->addWidget(farGroup);
    vbox->addStretch();

    auto* scroll = new QScrollArea(this);
    scroll->setWidget(content);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);

    // ── Signal wiring ─────────────────────────────────────────────────────────
    connect(pdSb,    &QDoubleSpinBox::valueChanged, this,
            [this, pdSb](double v){ presenter_.set_pd(v); applyFieldError(pdSb, presenter_.pd_error()); });
    connect(pfaSb,   &QDoubleSpinBox::valueChanged, this,
            [this, pfaSb](double v){ presenter_.set_pfa_exponent(v); applyFieldError(pfaSb, presenter_.pfa_exponent_error()); });
    connect(npSb, &QSpinBox::valueChanged, this,
            [this, npSb](int v){ presenter_.set_num_pulses(v); applyFieldError(npSb, presenter_.num_pulses_error()); });
    connect(swSb, &QSpinBox::valueChanged, this,
            [this, swSb](int v){ presenter_.set_swerling_case(v); applyFieldError(swSb, presenter_.swerling_case_error()); });
    connect(bwDegSb, &QDoubleSpinBox::valueChanged, this,
            [this, bwDegSb](double v){ presenter_.set_beamwidth(v); applyFieldError(bwDegSb, presenter_.beamwidth_error()); });
    connect(scanSb,  &QDoubleSpinBox::valueChanged, this,
            [this, scanSb](double v){ presenter_.set_scan_rate(v); applyFieldError(scanSb, presenter_.scan_rate_error()); });
    connect(prfSb,   &QDoubleSpinBox::valueChanged, this,
            [this, prfSb](double v){ presenter_.set_prf(v); applyFieldError(prfSb, presenter_.prf_error()); });
    connect(nbwSb,   &QDoubleSpinBox::valueChanged, this,
            [this, nbwSb](double v){ presenter_.set_bandwidth(v); applyFieldError(nbwSb, presenter_.bandwidth_error()); });

    // ── Restore persisted values (after presenter wiring, before first recompute) ──
    restoreSpinValue(pdSb,    kGroup, QStringLiteral("pd"));
    restoreSpinValue(pfaSb,   kGroup, QStringLiteral("pfa_exponent"));
    restoreSpinValue(npSb,    kGroup, QStringLiteral("num_pulses"));
    restoreSpinValue(swSb,    kGroup, QStringLiteral("swerling_case"));
    restoreSpinValue(bwDegSb, kGroup, QStringLiteral("beamwidth_deg"));
    restoreSpinValue(scanSb,  kGroup, QStringLiteral("scan_rate_deg_s"));
    restoreSpinValue(prfSb,   kGroup, QStringLiteral("prf_hz"));
    restoreSpinValue(nbwSb,   kGroup, QStringLiteral("bandwidth_mhz"));

    presenter_.set_on_change([this](const ewpresenter::DetectionPresenter::Output& o){
        applyOutput(o);
    });

    // Seed per-field validation-error styling from initial/restored values (#41).
    applyFieldError(pdSb,    presenter_.pd_error());
    applyFieldError(pfaSb,   presenter_.pfa_exponent_error());
    applyFieldError(npSb,    presenter_.num_pulses_error());
    applyFieldError(swSb,    presenter_.swerling_case_error());
    applyFieldError(bwDegSb, presenter_.beamwidth_error());
    applyFieldError(scanSb,  presenter_.scan_rate_error());
    applyFieldError(prfSb,   presenter_.prf_error());
    applyFieldError(nbwSb,   presenter_.bandwidth_error());

    applyOutput(presenter_.output());
}

void DetectionPage::applyOutput(const ewpresenter::DetectionPresenter::Output& o)
{
    required_snr_->setText(QString::fromStdString(o.required_snr_str));
    required_snr_albersheim_->setText(QString::fromStdString(o.required_snr_albersheim_str));
    fluctuation_loss_->setText(QString::fromStdString(o.fluctuation_loss_str));
    dwell_time_->setText(QString::fromStdString(o.dwell_time_str));
    hits_per_scan_->setText(QString::fromStdString(o.hits_per_scan_str));
    far_->setText(QString::fromStdString(o.far_str));
}
