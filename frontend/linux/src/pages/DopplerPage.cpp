// DopplerPage.cpp
#include "DopplerPage.h"
#include "DiagramUtils.h"
#include "PageUtils.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

namespace {
const QString kGroup = QStringLiteral("Doppler");
}

DopplerPage::DopplerPage(QWidget* parent)
    : QWidget(parent)
{
    ResultRowRegistry results;

    // ── Doppler & ambiguity ───────────────────────────────────────────────────
    QFormLayout* dopForm = nullptr;
    auto* dopGroup = makeGroup(QStringLiteral("Doppler & ambiguity"), dopForm);

    auto* freqSb = addSpinRow(dopForm, QStringLiteral("Carrier frequency (MHz)"), 1.0, 100000.0,
        presenter_.frequency_mhz(), 100.0, 1, kGroup, QStringLiteral("frequency_mhz"),
        QStringLiteral("Radar carrier frequency"));
    auto* speedSb = addSpinRow(dopForm, QStringLiteral("Radial speed (closing +) (m/s)"), -3000.0, 3000.0,
        presenter_.radial_speed_m_s(), 10.0, 1, kGroup, QStringLiteral("radial_speed_m_s"),
        QStringLiteral("Target radial speed \u2014 positive closing, negative opening"));
    auto* prfSb = addSpinRow(dopForm, QStringLiteral("PRF (Hz)"), 10.0, 1000000.0,
        presenter_.prf_hz(), 10.0, 0, kGroup, QStringLiteral("prf_hz"),
        QStringLiteral("Pulse repetition frequency"));

    doppler_shift_ = addResultRow(dopForm, QStringLiteral("Doppler shift"),
        QStringLiteral("Two-way Doppler shift: 2 \u00d7 radial speed \u00f7 wavelength"), &results);
    unambiguous_range_ = addResultRow(dopForm, QStringLiteral("Unambiguous range"),
        QStringLiteral("Maximum unambiguous range: c \u00f7 (2 \u00d7 PRF)"), &results);
    blind_speed_ = addResultRow(dopForm, QStringLiteral("First blind speed"),
        QStringLiteral("Radial speed whose Doppler shift equals the PRF \u2014 invisible to an MTI canceller"), &results);
    unambiguous_velocity_ = addResultRow(dopForm, QStringLiteral("Unambiguous velocity (\u00b1)"),
        QStringLiteral("Radial-velocity window measurable without aliasing: half the blind speed each way"), &results);

    // ── Resolution ────────────────────────────────────────────────────────────
    QFormLayout* resForm = nullptr;
    auto* resGroup = makeGroup(QStringLiteral("Resolution"), resForm);

    auto* bwSb = addSpinRow(resForm, QStringLiteral("Waveform bandwidth (MHz)"), 0.001, 10000.0,
        presenter_.bandwidth_mhz(), 0.1, 3, kGroup, QStringLiteral("bandwidth_mhz"),
        QStringLiteral("Compressed waveform bandwidth \u2014 sets the range resolution"));
    auto* rangeSb = addSpinRow(resForm, QStringLiteral("Target range (km)"), 0.1, 5000.0,
        presenter_.target_range_km(), 10.0, 1, kGroup, QStringLiteral("target_range_km"),
        QStringLiteral("Range to the target \u2014 scales the cross-range cell size"));
    auto* azSb = addSpinRow(resForm, QStringLiteral("Azimuth beamwidth (deg)"), 0.1, 45.0,
        presenter_.beamwidth_az_deg(), 0.5, 1, kGroup, QStringLiteral("beamwidth_az_deg"),
        QStringLiteral("Antenna 3 dB azimuth beamwidth"));
    auto* elSb = addSpinRow(resForm, QStringLiteral("Elevation beamwidth (deg)"), 0.1, 45.0,
        presenter_.beamwidth_el_deg(), 0.5, 1, kGroup, QStringLiteral("beamwidth_el_deg"),
        QStringLiteral("Antenna 3 dB elevation beamwidth"));

    range_resolution_ = addResultRow(resForm, QStringLiteral("Range resolution"),
        QStringLiteral("Range resolution: c \u00f7 (2 \u00d7 bandwidth)"), &results);
    cross_range_az_ = addResultRow(resForm, QStringLiteral("Cross-range (az)"),
        QStringLiteral("Azimuth cross-range cell: range \u00d7 azimuth beamwidth"), &results);
    cross_range_el_ = addResultRow(resForm, QStringLiteral("Cross-range (el)"),
        QStringLiteral("Elevation cross-range cell: range \u00d7 elevation beamwidth"), &results);

    resForm->addRow(addCopyResultsButton(results));

    // ── Scroll container ──────────────────────────────────────────────────────────
    auto* content = new QWidget;
    auto* vbox    = new QVBoxLayout(content);
    vbox->addWidget(dopGroup);
    vbox->addWidget(resGroup);
    vbox->addWidget(makeDiagramGroup({
        QStringLiteral(":/diagrams/resolution-cell.png")}));
    vbox->addStretch();

    auto* scroll = new QScrollArea(this);
    scroll->setWidget(content);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);

    // ── Signal wiring ─────────────────────────────────────────────────────────
    connect(freqSb,  &QDoubleSpinBox::valueChanged, this,
            [this, freqSb](double v){ presenter_.set_frequency(v); applyFieldError(freqSb, presenter_.frequency_error()); });
    connect(speedSb, &QDoubleSpinBox::valueChanged, this,
            [this, speedSb](double v){ presenter_.set_radial_speed(v); applyFieldError(speedSb, presenter_.radial_speed_error()); });
    connect(prfSb,   &QDoubleSpinBox::valueChanged, this,
            [this, prfSb](double v){ presenter_.set_prf(v); applyFieldError(prfSb, presenter_.prf_error()); });
    connect(bwSb,    &QDoubleSpinBox::valueChanged, this,
            [this, bwSb](double v){ presenter_.set_bandwidth(v); applyFieldError(bwSb, presenter_.bandwidth_error()); });
    connect(rangeSb, &QDoubleSpinBox::valueChanged, this,
            [this, rangeSb](double v){ presenter_.set_target_range(v); applyFieldError(rangeSb, presenter_.target_range_error()); });
    connect(azSb,    &QDoubleSpinBox::valueChanged, this,
            [this, azSb](double v){ presenter_.set_beamwidth_az(v); applyFieldError(azSb, presenter_.beamwidth_az_error()); });
    connect(elSb,    &QDoubleSpinBox::valueChanged, this,
            [this, elSb](double v){ presenter_.set_beamwidth_el(v); applyFieldError(elSb, presenter_.beamwidth_el_error()); });

    // ── Restore persisted values (after presenter wiring, before first recompute) ──
    restoreSpinValue(freqSb,  kGroup, QStringLiteral("frequency_mhz"));
    restoreSpinValue(speedSb, kGroup, QStringLiteral("radial_speed_m_s"));
    restoreSpinValue(prfSb,   kGroup, QStringLiteral("prf_hz"));
    restoreSpinValue(bwSb,    kGroup, QStringLiteral("bandwidth_mhz"));
    restoreSpinValue(rangeSb, kGroup, QStringLiteral("target_range_km"));
    restoreSpinValue(azSb,    kGroup, QStringLiteral("beamwidth_az_deg"));
    restoreSpinValue(elSb,    kGroup, QStringLiteral("beamwidth_el_deg"));

    presenter_.set_on_change([this](const ewpresenter::DopplerPresenter::Output& o){
        applyOutput(o);
    });

    // Seed per-field validation-error styling from initial/restored values (#41).
    applyFieldError(freqSb,  presenter_.frequency_error());
    applyFieldError(speedSb, presenter_.radial_speed_error());
    applyFieldError(prfSb,   presenter_.prf_error());
    applyFieldError(bwSb,    presenter_.bandwidth_error());
    applyFieldError(rangeSb, presenter_.target_range_error());
    applyFieldError(azSb,    presenter_.beamwidth_az_error());
    applyFieldError(elSb,    presenter_.beamwidth_el_error());

    applyOutput(presenter_.output());
}

void DopplerPage::applyOutput(const ewpresenter::DopplerPresenter::Output& o)
{
    doppler_shift_->setText(QString::fromStdString(o.doppler_shift_str));
    unambiguous_range_->setText(QString::fromStdString(o.unambiguous_range_str));
    blind_speed_->setText(QString::fromStdString(o.blind_speed_str));
    unambiguous_velocity_->setText(QString::fromStdString(o.unambiguous_velocity_str));
    range_resolution_->setText(QString::fromStdString(o.range_resolution_str));
    cross_range_az_->setText(QString::fromStdString(o.cross_range_az_str));
    cross_range_el_->setText(QString::fromStdString(o.cross_range_el_str));
}
