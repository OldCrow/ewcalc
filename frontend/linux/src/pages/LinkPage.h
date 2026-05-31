#pragma once
#include <QWidget>
#include <ewpresenter/link_presenter.h>

class QLabel;

class LinkPage : public QWidget
{
public:
    explicit LinkPage(QWidget* parent = nullptr);
private:
    ewpresenter::LinkPresenter presenter_;
    QLabel* received_power_;
    QLabel* path_loss_;
    QLabel* fresnel_zone_;
    QLabel* regime_;
    QLabel* link_margin_;
    QLabel* effective_range_;
    QLabel* range_regime_;
    void applyOutput(const ewpresenter::LinkPresenter::Output& o);
};
