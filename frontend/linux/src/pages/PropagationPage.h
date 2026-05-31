#pragma once
#include <QWidget>
#include <ewpresenter/propagation_presenter.h>

class QLabel;

class PropagationPage : public QWidget
{
public:
    explicit PropagationPage(QWidget* parent = nullptr);
private:
    ewpresenter::PropagationPresenter presenter_;
    QLabel* fspl_;
    QLabel* two_ray_loss_;
    QLabel* fresnel_zone_;
    QLabel* path_loss_;
    QLabel* regime_;
    void applyOutput(const ewpresenter::PropagationPresenter::Output& o);
};
