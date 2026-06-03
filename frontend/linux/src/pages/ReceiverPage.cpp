// ReceiverPage.cpp
#include "ReceiverPage.h"
#include "PageUtils.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>

ReceiverPage::ReceiverPage(QWidget* parent)
    : QWidget(parent)
{
    // ── Sensitivity / DR inputs ───────────────────────────────────────────────
    QFormLayout* inForm = nullptr;
    auto* inGroup = makeGroup(QStringLiteral("System Inputs"), inForm);

    auto* bwSb   = addSpinRow(inForm, QStringLiteral("Bandwidth (MHz)"),    0.001, 10000.0, presenter_.bandwidth_mhz(),       0.1, 3);
    auto* nfSb   = addSpinRow(inForm, QStringLiteral("Noise figure (dB)"),  0.0,     30.0,  presenter_.noise_figure_db(),     0.5, 1);
    auto* snrSb  = addSpinRow(inForm, QStringLiteral("Required SNR (dB)"), -20.0,    50.0,  presenter_.required_snr_db(),     0.5, 1);
    auto* iip2Sb = addSpinRow(inForm, QStringLiteral("IIP2 (dBm)"),        -50.0,   100.0,  presenter_.second_order_ip_dbm(), 1.0, 1);
    auto* iip3Sb = addSpinRow(inForm, QStringLiteral("IIP3 (dBm)"),        -50.0,   100.0,  presenter_.third_order_ip_dbm(),  1.0, 1);
    auto* adcSb  = addIntSpinRow(inForm, QStringLiteral("ADC bits"),        1, 64, presenter_.adc_bits());

    // ── Noise-chain stage cascade ─────────────────────────────────────────────
    auto* stageGroup = new QGroupBox(QStringLiteral("Noise Chain Stages"));
    auto* stageVBox  = new QVBoxLayout(stageGroup);

    stages_container_ = new QWidget;
    stages_layout_    = new QVBoxLayout(stages_container_);
    stages_layout_->setContentsMargins(0, 0, 0, 0);
    stages_layout_->setSpacing(2);

    auto* addBtn = new QPushButton(QStringLiteral("+ Add stage"));
    stageVBox->addWidget(stages_container_);
    stageVBox->addWidget(addBtn, 0, Qt::AlignLeft);

    // ── Outputs ───────────────────────────────────────────────────────────────
    QFormLayout* outForm = nullptr;
    auto* outGroup = makeGroup(QStringLiteral("Results"), outForm);

    sensitivity_       = addResultRow(outForm, QStringLiteral("Sensitivity"));
    cascaded_nf_       = addResultRow(outForm, QStringLiteral("Cascaded NF"));
    system_noise_temp_ = addResultRow(outForm, QStringLiteral("Sys. noise temp"));
    sfdr2_             = addResultRow(outForm, QStringLiteral("SFDR (2nd order)"));
    sfdr3_             = addResultRow(outForm, QStringLiteral("SFDR (3rd order)"));
    digital_dr_        = addResultRow(outForm, QStringLiteral("Digital DR"));

    // ── Scroll container ──────────────────────────────────────────────────────
    auto* content = new QWidget;
    auto* vbox    = new QVBoxLayout(content);
    vbox->addWidget(inGroup);
    vbox->addWidget(stageGroup);
    vbox->addWidget(outGroup);
    vbox->addStretch();

    auto* scroll = new QScrollArea(this);
    scroll->setWidget(content);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);

    // ── Signal wiring: system inputs ─────────────────────────────────────────
    connect(bwSb,   &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_bandwidth(v); });
    connect(nfSb,   &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_noise_figure(v); });
    connect(snrSb,  &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_required_snr(v); });
    connect(iip2Sb, &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_second_order_ip(v); });
    connect(iip3Sb, &QDoubleSpinBox::valueChanged, this,
            [this](double v){ presenter_.set_third_order_ip(v); });
    connect(adcSb, &QSpinBox::valueChanged, this,
            [this](int v){ presenter_.set_adc_bits(v); });

    // ── Signal wiring: stage management ──────────────────────────────────────
    connect(addBtn, &QPushButton::clicked, this, [this]{ addStage(); });

    presenter_.set_on_change([this](const ewpresenter::ReceiverPresenter::Output& o){
        applyOutput(o);
    });

    // Populate stage list from presenter defaults and seed outputs.
    rebuildStageList();
    applyOutput(presenter_.output());
}

void ReceiverPage::rebuildStageList()
{
    // Remove all existing row widgets.
    while (QLayoutItem* item = stages_layout_->takeAt(0)) {
        if (QWidget* w = item->widget()) w->deleteLater();
        delete item;
    }
    stage_rows_.clear();

    const auto& stages = presenter_.stages();
    for (int i = 0; i < static_cast<int>(stages.size()); ++i) {
        auto* row    = new QWidget;
        auto* hbox   = new QHBoxLayout(row);
        hbox->setContentsMargins(0, 0, 0, 0);

        auto* idxLbl = new QLabel(QStringLiteral("S%1").arg(i + 1));
        idxLbl->setMinimumWidth(28);

        auto* nfSb = makeSpinBox(-10.0, 50.0, stages[i].noise_figure_db, 0.5, 1);
        auto* gSb  = makeSpinBox(-60.0, 60.0, stages[i].gain_db,         0.5, 1);

        auto* nfLbl = new QLabel(QStringLiteral("NF"));
        auto* gLbl  = new QLabel(QStringLiteral("G"));
        auto* dbLbl1 = new QLabel(QStringLiteral("dB"));
        auto* dbLbl2 = new QLabel(QStringLiteral("dB"));

        auto* rmBtn = new QPushButton(QStringLiteral("−"));
        rmBtn->setMaximumWidth(28);

        hbox->addWidget(idxLbl);
        hbox->addWidget(nfLbl);
        hbox->addWidget(nfSb);
        hbox->addWidget(dbLbl1);
        hbox->addSpacing(8);
        hbox->addWidget(gLbl);
        hbox->addWidget(gSb);
        hbox->addWidget(dbLbl2);
        hbox->addStretch();
        hbox->addWidget(rmBtn);

        stages_layout_->addWidget(row);
        stage_rows_.push_back({nfSb, gSb});

        // Capture index by value; valid for the lifetime of this row.
        connect(nfSb, &QDoubleSpinBox::valueChanged, this,
                [this](double){ pushStages(); });
        connect(gSb,  &QDoubleSpinBox::valueChanged, this,
                [this](double){ pushStages(); });
        connect(rmBtn, &QPushButton::clicked, this,
                [this, i]{ removeStage(i); });
    }
}

void ReceiverPage::pushStages()
{
    std::vector<ewpresenter::ReceiverPresenter::StageInput> stages;
    stages.reserve(stage_rows_.size());
    for (const auto& r : stage_rows_)
        stages.push_back({r.nf->value(), r.gain->value()});
    presenter_.set_stages(std::move(stages));
}

void ReceiverPage::addStage()
{
    auto stages = presenter_.stages();
    stages.push_back({3.0, 0.0});
    presenter_.set_stages(std::move(stages));
    rebuildStageList();
}

void ReceiverPage::removeStage(int index)
{
    auto stages = presenter_.stages();
    if (stages.size() <= 1) return; // keep at least one stage
    stages.erase(stages.begin() + index);
    presenter_.set_stages(std::move(stages));
    rebuildStageList();
}

void ReceiverPage::applyOutput(const ewpresenter::ReceiverPresenter::Output& o)
{
    sensitivity_->setText(QString::fromStdString(o.sensitivity_str));
    cascaded_nf_->setText(QString::fromStdString(o.cascaded_nf_str));
    system_noise_temp_->setText(QString::fromStdString(o.system_noise_temp_str));
    sfdr2_->setText(QString::fromStdString(o.sfdr2_str));
    sfdr3_->setText(QString::fromStdString(o.sfdr3_str));
    digital_dr_->setText(QString::fromStdString(o.digital_dr_str));
}
