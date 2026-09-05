#pragma once
#include <QWidget>
#include <ewpresenter/doppler_presenter.h>

class QLabel;

class DopplerPage : public QWidget
{
public:
    explicit DopplerPage(QWidget* parent = nullptr);
private:
    ewpresenter::DopplerPresenter presenter_;
    QLabel* doppler_shift_;
    QLabel* unambiguous_range_;
    QLabel* blind_speed_;
    QLabel* unambiguous_velocity_;
    QLabel* range_resolution_;
    QLabel* cross_range_az_;
    QLabel* cross_range_el_;
    void applyOutput(const ewpresenter::DopplerPresenter::Output& o);
};
