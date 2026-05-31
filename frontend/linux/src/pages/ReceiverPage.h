#pragma once
#include <QWidget>
#include <ewpresenter/receiver_presenter.h>
#include <vector>

class QDoubleSpinBox;
class QLabel;
class QSpinBox;
class QVBoxLayout;

class ReceiverPage : public QWidget
{
public:
    explicit ReceiverPage(QWidget* parent = nullptr);

private:
    ewpresenter::ReceiverPresenter presenter_;

    // Stage row widgets (kept in sync with presenter_.stages())
    struct StageRow {
        QDoubleSpinBox* nf;
        QDoubleSpinBox* gain;
    };
    std::vector<StageRow> stage_rows_;
    QWidget*    stages_container_ = nullptr;
    QVBoxLayout* stages_layout_   = nullptr;

    // Output labels
    QLabel* sensitivity_;
    QLabel* cascaded_nf_;
    QLabel* sfdr2_;
    QLabel* sfdr3_;
    QLabel* digital_dr_;

    void rebuildStageList();
    void pushStages();
    void addStage();
    void removeStage(int index);
    void applyOutput(const ewpresenter::ReceiverPresenter::Output& o);
};
