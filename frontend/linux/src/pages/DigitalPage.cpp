// DigitalPage.cpp
#include "DigitalPage.h"
#include "PageUtils.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

namespace {
const QString kGroup = QStringLiteral("Digital");
}

DigitalPage::DigitalPage(QWidget* parent)
    : QWidget(parent)
{
    // ── Digital Link ────────────────────────────────────────────────────────
    QFormLayout* dlForm = nullptr;
    auto* dlGroup = makeGroup(QStringLiteral("Digital Link"), dlForm);

    auto* dataRateSb = addSpinRow(dlForm, QStringLiteral("Data rate (Mbps)"), 0.0001, 10000.0,
        presenter_.data_rate_mhz(), 0.01, 4, kGroup, QStringLiteral("data_rate_mhz"),
        QStringLiteral("Information bit rate \u2014 shared between the Eb/N\u2080 conversion and DSSS process gain"));
    auto* bwSb = addSpinRow(dlForm, QStringLiteral("Bandwidth (MHz)"), 0.001, 10000.0,
        presenter_.bandwidth_mhz(), 0.1, 3, kGroup, QStringLiteral("bandwidth_mhz"),
        QStringLiteral("Receiver noise bandwidth \u2014 wider than the data rate gives Eb/N\u2080 > SNR"));
    auto* snrSb = addSpinRow(dlForm, QStringLiteral("Received SNR (dB)"), -30.0, 60.0,
        presenter_.snr_db(), 0.5, 1, kGroup, QStringLiteral("snr_db"),
        QStringLiteral("Carrier-to-noise ratio measured in the noise bandwidth"));

    // ── DSSS ──────────────────────────────────────────────────────
    QFormLayout* dsForm = nullptr;
    auto* dsGroup = makeGroup(QStringLiteral("DSSS"), dsForm);

    auto* chipRateSb = addSpinRow(dsForm, QStringLiteral("Chip rate (Mcps)"), 0.0001, 10000.0,
        presenter_.chip_rate_mhz(), 1.0, 3, kGroup, QStringLiteral("chip_rate_mhz"),
        QStringLiteral("Spread-spectrum chipping rate \u2014 determines the spreading bandwidth and process gain"));
    auto* reqEbNoSb = addSpinRow(dsForm, QStringLiteral("Required Eb/N\u2080 (dB)"), -10.0, 30.0,
        presenter_.required_eb_no_db(), 0.5, 1, kGroup, QStringLiteral("required_eb_no_db"),
        QStringLiteral("Minimum energy-per-bit to noise density for acceptable BER \u2014 typically 10\u201313 dB for BPSK/QPSK"));
    auto* implLossSb = addSpinRow(dsForm, QStringLiteral("Impl. loss (dB)"), 0.0, 10.0,
        presenter_.implementation_loss_db(), 0.5, 1, kGroup, QStringLiteral("implementation_loss_db"),
        QStringLiteral("Practical losses from non-ideal code synchronisation, filter roll-off, etc. \u2014 typically 1\u20133 dB"));

    // ── Outputs ───────────────────────────────────────────────────────────────
    QFormLayout* outForm = nullptr;
    auto* outGroup = makeGroup(QStringLiteral("Results"), outForm);

    ResultRowRegistry results;
    eb_no_ = addResultRow(outForm, QStringLiteral("Eb/N\u2080"),
        QStringLiteral("Energy-per-bit to noise density: SNR + 10\u00b7log\u2081\u2080(bandwidth / data rate)"), &results);
    required_snr_for_eb_no_ = addResultRow(outForm, QStringLiteral("Required SNR"),
        QStringLiteral("Carrier SNR needed to achieve the required Eb/N\u2080, given the same bandwidth and data rate"), &results);
    process_gain_ = addResultRow(outForm, QStringLiteral("Process gain"),
        QStringLiteral("DSSS spreading gain: 10\u00b7log\u2081\u2080(chip rate / data rate)"), &results);
    jamming_margin_ = addResultRow(outForm, QStringLiteral("Jamming margin"),
        QStringLiteral("Process gain minus required Eb/N\u2080 minus implementation losses \u2014 positive means spreading gain exceeds jammer advantage"), &results);
    required_js_ = addResultRow(outForm, QStringLiteral("Required J/S"),
        QStringLiteral("J/S a jammer must achieve to overcome the spreading gain \u2014 negative means the jammer has an inherent advantage"), &results);

    outForm->addRow(addCopyResultsButton(results));

    // ── Scroll container ──────────────────────────────────────────────────────────
    auto* content = new QWidget;
    auto* vbox    = new QVBoxLayout(content);
    vbox->addWidget(dlGroup);
    vbox->addWidget(dsGroup);
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
    connect(dataRateSb, &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_data_rate(v); });
    connect(bwSb,       &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_bandwidth(v); });
    connect(snrSb,      &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_snr(v); });
    connect(chipRateSb, &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_chip_rate(v); });
    connect(reqEbNoSb,  &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_required_eb_no(v); });
    connect(implLossSb, &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_implementation_loss(v); });

    // ── Restore persisted values (after presenter wiring, before first recompute) ──
    restoreSpinValue(dataRateSb, kGroup, QStringLiteral("data_rate_mhz"));
    restoreSpinValue(bwSb,       kGroup, QStringLiteral("bandwidth_mhz"));
    restoreSpinValue(snrSb,      kGroup, QStringLiteral("snr_db"));
    restoreSpinValue(chipRateSb, kGroup, QStringLiteral("chip_rate_mhz"));
    restoreSpinValue(reqEbNoSb,  kGroup, QStringLiteral("required_eb_no_db"));
    restoreSpinValue(implLossSb, kGroup, QStringLiteral("implementation_loss_db"));

    presenter_.set_on_change([this](const ewpresenter::DigitalPresenter::Output& o){
        applyOutput(o);
    });

    applyOutput(presenter_.output());
}

void DigitalPage::applyOutput(const ewpresenter::DigitalPresenter::Output& o)
{
    eb_no_->setText(QString::fromStdString(o.eb_no_str));
    required_snr_for_eb_no_->setText(QString::fromStdString(o.required_snr_for_eb_no_str));
    process_gain_->setText(QString::fromStdString(o.process_gain_str));
    jamming_margin_->setText(QString::fromStdString(o.jamming_margin_str));
    required_js_->setText(QString::fromStdString(o.required_js_str));
}
