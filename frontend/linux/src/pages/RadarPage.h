#pragma once
#include <QWidget>
#include <ewpresenter/radar_presenter.h>

class QLabel;

class RadarPage : public QWidget
{
public:
    explicit RadarPage(QWidget* parent = nullptr);
private:
    ewpresenter::RadarPresenter presenter_;
    QLabel* max_range_;
    QLabel* two_way_loss_;
    QLabel* target_rcs_;
    QLabel* pc_gain_;
    QLabel* ci_gain_;
    QLabel* lpi_advantage_;
    void applyOutput(const ewpresenter::RadarPresenter::Output& o);
};
