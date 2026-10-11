// RadarJammingPage.cpp
#include "RadarJammingPage.h"
#include "DiagramUtils.h"
#include "PageUtils.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

namespace {
const QString kGroup = QStringLiteral("RadarJamming");
}

RadarJammingPage::RadarJammingPage(QWidget* parent)
    : QWidget(parent)
{
    using Presenter = ewpresenter::RadarJammingPresenter;
    ResultRowRegistry results;

    // ── Geometry selector + shared inputs ────────────────────────────────────
    QFormLayout* radarForm = nullptr;
    auto* radarGroup = makeGroup(QStringLiteral("Radar & jammer"), radarForm);

    // Int-enum selector: item data carries the presenter's geometry constant.
    auto* geomCb = new QComboBox;
    geomCb->addItem(QStringLiteral("Self-protection"), Presenter::kSelfProtection);
    geomCb->addItem(QStringLiteral("Stand-off"),       Presenter::kStandOff);
    geomCb->setAccessibleName(QStringLiteral("Geometry"));
    const QString geomHelp = QStringLiteral(
        "Self-protection: the jammer rides on the target. "
        "Stand-off: the jammer sits at its own range and reaches the radar through its sidelobes");
    geomCb->setToolTip(geomHelp);
    geomCb->setMinimumWidth(110);
    geomCb->setMaximumWidth(150);
    auto* geomWrap = new QWidget;
    auto* geomBox  = new QHBoxLayout(geomWrap);
    geomBox->setContentsMargins(0, 0, 0, 0);
    geomBox->addStretch(1);
    geomBox->addWidget(geomCb);
    radarForm->addRow(QStringLiteral("Geometry:"), geomWrap);

    auto* radarErpSb = addSpinRow(radarForm, QStringLiteral("Radar ERP (dBm)"), -100.0, 200.0,
        presenter_.radar_erp_dbm(), 1.0, 1, kGroup, QStringLiteral("radar_erp_dbm"),
        QStringLiteral("Radar effective radiated power toward the target: peak power + transmit antenna gain"));
    auto* jamErpSb = addSpinRow(radarForm, QStringLiteral("Jammer ERP (dBm)"), -100.0, 200.0,
        presenter_.jammer_erp_dbm(), 1.0, 1, kGroup, QStringLiteral("jammer_erp_dbm"),
        QStringLiteral("Jammer effective radiated power toward the radar. Must lie within the radar's receiver bandwidth"));
    auto* rcsSb = addSpinRow(radarForm, QStringLiteral("Target RCS (dBsm)"), -40.0, 60.0,
        presenter_.target_rcs_dbsm(), 1.0, 1, kGroup, QStringLiteral("target_rcs_dbsm"),
        QStringLiteral("Target radar cross-section"));
    auto* tgtRangeSb = addSpinRow(radarForm, QStringLiteral("Target range (km)"), 0.01, 10000.0,
        presenter_.target_range_km(), 1.0, 2, kGroup, QStringLiteral("target_range_km"),
        QStringLiteral("Radar-to-target range"));
    auto* thrSb = addSpinRow(radarForm, QStringLiteral("J/S threshold (dB)"), -30.0, 30.0,
        presenter_.js_threshold_db(), 1.0, 1, kGroup, QStringLiteral("js_threshold_db"),
        QStringLiteral("J/S at or above which the jamming is considered effective"));

    // Stand-off-only rows; hidden in self-protection mode (see applyGeometryVisibility).
    auto* jamRangeSb = addSpinRow(radarForm, QStringLiteral("Jammer range (km)"), 0.01, 10000.0,
        presenter_.jammer_range_km(), 1.0, 2, kGroup, QStringLiteral("jammer_range_km"),
        QStringLiteral("Radar-to-jammer range (stand-off only)"));
    auto* gmSb = addSpinRow(radarForm, QStringLiteral("Mainbeam gain G_M (dB)"), -30.0, 60.0,
        presenter_.mainbeam_gain_db(), 1.0, 1, kGroup, QStringLiteral("mainbeam_gain_db"),
        QStringLiteral("Radar receive gain toward the target (stand-off only)"));
    auto* gsSb = addSpinRow(radarForm, QStringLiteral("Sidelobe gain G_S (dB)"), -30.0, 60.0,
        presenter_.sidelobe_gain_db(), 1.0, 1, kGroup, QStringLiteral("sidelobe_gain_db"),
        QStringLiteral("Radar receive gain toward the jammer (stand-off only)"));

    // ── Results ──────────────────────────────────────────────────────────────
    QFormLayout* resForm = nullptr;
    auto* resGroup = makeGroup(QStringLiteral("Results"), resForm);

    js_ratio_ = addResultRow(resForm, QStringLiteral("J/S"),
        QStringLiteral("Jamming-to-signal ratio at the radar receiver, at the current target range"), &results);
    js_margin_ = addResultRow(resForm, QStringLiteral("J/S margin"),
        QStringLiteral("J/S minus the threshold — positive means the jammer is effective"), &results);
    burnthrough_range_ = addResultRow(resForm, QStringLiteral("Burnthrough range"),
        QStringLiteral("Range at which J/S falls to the threshold. Self-protection: the radar sees the target inside it. "
                       "Stand-off: the radar-to-target range, with the jammer held at its range"), &results);
    status_ = addResultRow(resForm, QStringLiteral("Status"),
        QStringLiteral("Whether the jammer is effective at the current target range"), &results);

    resForm->addRow(addCopyResultsButton(results));

    // ── Scroll container ──────────────────────────────────────────────────────────
    // One diagram group per geometry; only the selected mode's is visible.
    auto* spDiagram = makeDiagramGroup({QStringLiteral(":/diagrams/jamming-self-protection.png")});
    auto* soDiagram = makeDiagramGroup({QStringLiteral(":/diagrams/jamming-stand-off.png")});

    auto* content = new QWidget;
    auto* vbox    = new QVBoxLayout(content);
    vbox->addWidget(radarGroup);
    vbox->addWidget(resGroup);
    vbox->addWidget(spDiagram);
    vbox->addWidget(soDiagram);
    vbox->addStretch();

    auto* scroll = new QScrollArea(this);
    scroll->setWidget(content);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);

    // Shows/hides the stand-off-only rows and the matching diagram.
    // QFormLayout::setRowVisible needs Qt 6.4; CI's Ubuntu 22.04 ships 6.2,
    // so hide each row's label and field directly.
    auto setRowShown = [radarForm](QWidget* field, bool shown) {
        field->setVisible(shown);
        if (QWidget* label = radarForm->labelForField(field))
            label->setVisible(shown);
    };
    auto applyGeometryVisibility = [=](int geometry) {
        const bool standOff = (geometry == Presenter::kStandOff);
        setRowShown(jamRangeSb->parentWidget(), standOff);
        setRowShown(gmSb->parentWidget(), standOff);
        setRowShown(gsSb->parentWidget(), standOff);
        spDiagram->setVisible(!standOff);
        soDiagram->setVisible(standOff);
    };

    // ── Signal wiring ─────────────────────────────────────────────────────────
    connect(geomCb, &QComboBox::currentIndexChanged, this,
            [this, geomCb, applyGeometryVisibility](int idx){
                const int g = geomCb->itemData(idx).toInt();
                presenter_.set_geometry(g);
                applyGeometryVisibility(g);
                AppSettings::instance().setValue(kGroup, QStringLiteral("geometry"), g);
            });
    connect(radarErpSb, &QDoubleSpinBox::valueChanged, this,
            [this, radarErpSb](double v){ presenter_.set_radar_erp(v); applyFieldError(radarErpSb, presenter_.radar_erp_error()); });
    connect(jamErpSb,   &QDoubleSpinBox::valueChanged, this,
            [this, jamErpSb](double v){ presenter_.set_jammer_erp(v); applyFieldError(jamErpSb, presenter_.jammer_erp_error()); });
    connect(rcsSb,      &QDoubleSpinBox::valueChanged, this,
            [this, rcsSb](double v){ presenter_.set_target_rcs(v); applyFieldError(rcsSb, presenter_.target_rcs_error()); });
    connect(tgtRangeSb, &QDoubleSpinBox::valueChanged, this,
            [this, tgtRangeSb](double v){ presenter_.set_target_range(v); applyFieldError(tgtRangeSb, presenter_.target_range_error()); });
    connect(thrSb,      &QDoubleSpinBox::valueChanged, this,
            [this, thrSb](double v){ presenter_.set_js_threshold(v); applyFieldError(thrSb, presenter_.js_threshold_error()); });
    connect(jamRangeSb, &QDoubleSpinBox::valueChanged, this,
            [this, jamRangeSb](double v){ presenter_.set_jammer_range(v); applyFieldError(jamRangeSb, presenter_.jammer_range_error()); });
    connect(gmSb,       &QDoubleSpinBox::valueChanged, this,
            [this, gmSb](double v){ presenter_.set_mainbeam_gain(v); applyFieldError(gmSb, presenter_.mainbeam_gain_error()); });
    connect(gsSb,       &QDoubleSpinBox::valueChanged, this,
            [this, gsSb](double v){ presenter_.set_sidelobe_gain(v); applyFieldError(gsSb, presenter_.sidelobe_gain_error()); });

    // ── Restore persisted values (after presenter wiring, before first recompute) ──
    restoreSpinValue(radarErpSb, kGroup, QStringLiteral("radar_erp_dbm"));
    restoreSpinValue(jamErpSb,   kGroup, QStringLiteral("jammer_erp_dbm"));
    restoreSpinValue(rcsSb,      kGroup, QStringLiteral("target_rcs_dbsm"));
    restoreSpinValue(tgtRangeSb, kGroup, QStringLiteral("target_range_km"));
    restoreSpinValue(thrSb,      kGroup, QStringLiteral("js_threshold_db"));
    restoreSpinValue(jamRangeSb, kGroup, QStringLiteral("jammer_range_km"));
    restoreSpinValue(gmSb,       kGroup, QStringLiteral("mainbeam_gain_db"));
    restoreSpinValue(gsSb,       kGroup, QStringLiteral("sidelobe_gain_db"));

    // Geometry: restore by item data; unknown stored values fall back to the default.
    {
        const int saved = AppSettings::instance()
            .value(kGroup, QStringLiteral("geometry"), presenter_.geometry()).toInt();
        const int idx = geomCb->findData(saved);
        geomCb->setCurrentIndex(idx >= 0 ? idx : geomCb->findData(presenter_.geometry()));
    }

    presenter_.set_on_change([this](const Presenter::Output& o){
        applyOutput(o);
    });

    // Seed per-field validation-error styling from initial/restored values (#41).
    applyFieldError(radarErpSb, presenter_.radar_erp_error());
    applyFieldError(jamErpSb,   presenter_.jammer_erp_error());
    applyFieldError(rcsSb,      presenter_.target_rcs_error());
    applyFieldError(tgtRangeSb, presenter_.target_range_error());
    applyFieldError(thrSb,      presenter_.js_threshold_error());
    applyFieldError(jamRangeSb, presenter_.jammer_range_error());
    applyFieldError(gmSb,       presenter_.mainbeam_gain_error());
    applyFieldError(gsSb,       presenter_.sidelobe_gain_error());

    // Covers the case where no currentIndexChanged fired (default index restored).
    applyGeometryVisibility(presenter_.geometry());
    applyOutput(presenter_.output());
}

void RadarJammingPage::applyOutput(const ewpresenter::RadarJammingPresenter::Output& o)
{
    js_ratio_->setText(QString::fromStdString(o.js_ratio_str));
    js_margin_->setText(QString::fromStdString(o.js_margin_str));
    burnthrough_range_->setText(QString::fromStdString(o.burnthrough_range_str));
    status_->setText(QString::fromStdString(o.status_str));
}
