#pragma once
#include <QWidget>
#include <ewpresenter/jamming_presenter.h>

class QLabel;

class JammingPage : public QWidget
{
public:
    explicit JammingPage(QWidget* parent = nullptr);
private:
    ewpresenter::JammingPresenter presenter_;
    QLabel* js_ratio_;
    QLabel* signal_at_rx_;
    QLabel* jammer_at_rx_;
    QLabel* optimum_bw_;
    QLabel* duty_cycle_;
    void applyOutput(const ewpresenter::JammingPresenter::Output& o);
};
