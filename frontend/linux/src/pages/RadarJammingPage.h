#pragma once
#include <QWidget>
#include <ewpresenter/radar_jamming_presenter.h>

class QLabel;

class RadarJammingPage : public QWidget
{
public:
    explicit RadarJammingPage(QWidget* parent = nullptr);
private:
    ewpresenter::RadarJammingPresenter presenter_;
    QLabel* js_ratio_;
    QLabel* js_margin_;
    QLabel* burnthrough_range_;
    QLabel* status_;
    void applyOutput(const ewpresenter::RadarJammingPresenter::Output& o);
};
