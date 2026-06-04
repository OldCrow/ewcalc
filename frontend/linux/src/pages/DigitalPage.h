#pragma once
#include <QWidget>
#include <ewpresenter/digital_presenter.h>

class QLabel;

class DigitalPage : public QWidget
{
public:
    explicit DigitalPage(QWidget* parent = nullptr);
private:
    ewpresenter::DigitalPresenter presenter_;
    QLabel* eb_no_;
    QLabel* snr_from_eb_no_;
    QLabel* process_gain_;
    QLabel* jamming_margin_;
    QLabel* required_js_;
    void applyOutput(const ewpresenter::DigitalPresenter::Output& o);
};
