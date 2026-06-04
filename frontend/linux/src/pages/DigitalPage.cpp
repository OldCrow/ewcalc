// DigitalPage.cpp
#include "DigitalPage.h"
#include "PageUtils.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

DigitalPage::DigitalPage(QWidget* parent)
    : QWidget(parent)
{
    // ── Digital Link ──────────────────────────────────────────────────────────
    QFormLayout* dlForm = nullptr;
    auto* dlGroup = makeGroup(QStringLiteral("Digital Link"), dlForm);

    auto* dataRateSb = addSpinRow(dlForm, QStringLiteral("Data rate (Mbps)"),  0.0001, 10000.0, presenter_.data_rate_mhz(),  0.01, 4);
    auto* bwSb       = addSpinRow(dlForm, QStringLiteral("Bandwidth (MHz)"),   0.001,  10000.0, presenter_.bandwidth_mhz(),  0.1,  3);
    auto* snrSb      = addSpinRow(dlForm, QStringLiteral("Received SNR (dB)"), -30.0,     60.0, presenter_.snr_db(),         0.5,  1);

    // ── DSSS ────────────────────────────────────────────────────────────────
    QFormLayout* dsForm = nullptr;
    auto* dsGroup = makeGroup(QStringLiteral("DSSS"), dsForm);

    auto* chipRateSb  = addSpinRow(dsForm, QStringLiteral("Chip rate (Mcps)"),       0.0001, 10000.0, presenter_.chip_rate_mhz(),           1.0, 3);
    auto* reqEbNoSb   = addSpinRow(dsForm, QStringLiteral("Required Eb/N\u2080 (dB)"), -10.0,    30.0, presenter_.required_eb_no_db(),       0.5, 1);
    auto* implLossSb  = addSpinRow(dsForm, QStringLiteral("Impl. loss (dB)"),          0.0,    10.0, presenter_.implementation_loss_db(),  0.5, 1);

    // ── Outputs ───────────────────────────────────────────────────────────────
    QFormLayout* outForm = nullptr;
    auto* outGroup = makeGroup(QStringLiteral("Results"), outForm);

    eb_no_           = addResultRow(outForm, QStringLiteral("Eb/N\u2080"));
    snr_from_eb_no_  = addResultRow(outForm, QStringLiteral("SNR (from Eb/No)"));
    process_gain_    = addResultRow(outForm, QStringLiteral("Process gain"));
    jamming_margin_  = addResultRow(outForm, QStringLiteral("Jamming margin"));
    required_js_     = addResultRow(outForm, QStringLiteral("Required J/S"));

    // ── Scroll container ──────────────────────────────────────────────────────
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

    presenter_.set_on_change([this](const ewpresenter::DigitalPresenter::Output& o){
        applyOutput(o);
    });

    applyOutput(presenter_.output());
}

void DigitalPage::applyOutput(const ewpresenter::DigitalPresenter::Output& o)
{
    eb_no_->setText(QString::fromStdString(o.eb_no_str));
    snr_from_eb_no_->setText(QString::fromStdString(o.snr_from_eb_no_str));
    process_gain_->setText(QString::fromStdString(o.process_gain_str));
    jamming_margin_->setText(QString::fromStdString(o.jamming_margin_str));
    required_js_->setText(QString::fromStdString(o.required_js_str));
}
