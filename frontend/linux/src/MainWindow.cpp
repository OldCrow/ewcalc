// MainWindow.cpp
#include "MainWindow.h"
#include "pages/PropagationPage.h"
#include "pages/LinkPage.h"
#include "pages/ReceiverPage.h"
#include "pages/JammingPage.h"
#include "pages/LocationPage.h"
#include "pages/RadarPage.h"
#include "pages/DigitalPage.h"
#include "pages/ReferencePage.h"

#include <QHBoxLayout>
#include <QListWidget>
#include <QStackedWidget>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , nav_(new QListWidget)
    , stack_(new QStackedWidget)
{
    setWindowTitle(QStringLiteral("EW Calculator"));
    setMinimumSize(980, 640);

    // ── Sidebar ───────────────────────────────────────────────────────────────
    nav_->setMaximumWidth(160);
    nav_->setMinimumWidth(140);
    for (const auto* label : {
            "Propagation", "Link Budget", "Receiver",
            "Jamming",     "Location",    "Radar",
            "Digital / DSSS" })
        nav_->addItem(QString::fromUtf8(label));
    nav_->addItem(QStringLiteral("Reference"));

    // ── Pages ─────────────────────────────────────────────────────────────────
    stack_->addWidget(new PropagationPage);
    stack_->addWidget(new LinkPage);
    stack_->addWidget(new ReceiverPage);
    stack_->addWidget(new JammingPage);
    stack_->addWidget(new LocationPage);
    stack_->addWidget(new RadarPage);
    stack_->addWidget(new DigitalPage);
    stack_->addWidget(new ReferencePage);

    connect(nav_, &QListWidget::currentRowChanged,
            stack_, &QStackedWidget::setCurrentIndex);
    nav_->setCurrentRow(0);

    // ── Layout ────────────────────────────────────────────────────────────────
    auto* central = new QWidget;
    auto* layout  = new QHBoxLayout(central);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);
    layout->addWidget(nav_);
    layout->addWidget(stack_, 1);
    setCentralWidget(central);
}
