#pragma once
#include <QWidget>
#include <ewpresenter/antenna_presenter.h>

class QLabel;

class AntennaPage : public QWidget
{
public:
    explicit AntennaPage(QWidget* parent = nullptr);
private:
    ewpresenter::AntennaPresenter presenter_;
    QLabel* erp_;
    QLabel* beamwidth_from_gain_;
    QLabel* gain_from_beamwidth_;
    QLabel* wavelength_;
    void applyOutput(const ewpresenter::AntennaPresenter::Output& o);
};
