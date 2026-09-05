#pragma once
#include <QWidget>
#include <ewpresenter/detection_presenter.h>

class QLabel;

class DetectionPage : public QWidget
{
public:
    explicit DetectionPage(QWidget* parent = nullptr);
private:
    ewpresenter::DetectionPresenter presenter_;
    QLabel* required_snr_;
    QLabel* required_snr_albersheim_;
    QLabel* fluctuation_loss_;
    QLabel* dwell_time_;
    QLabel* hits_per_scan_;
    QLabel* far_;
    void applyOutput(const ewpresenter::DetectionPresenter::Output& o);
};
