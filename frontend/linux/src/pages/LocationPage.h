#pragma once
#include <QWidget>
#include <ewpresenter/location_presenter.h>

class QLabel;

class LocationPage : public QWidget
{
public:
    explicit LocationPage(QWidget* parent = nullptr);
private:
    ewpresenter::LocationPresenter presenter_;
    QLabel* cep_aoa_;
    QLabel* cep_tdoa_;
    QLabel* cep_eep_;
    void applyOutput(const ewpresenter::LocationPresenter::Output& o);
};
