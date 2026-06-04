// MainWindow.cpp
#include "MainWindow.h"
#include "pages/PropagationPage.h"
#include "pages/LinkPage.h"
#include "pages/ReceiverPage.h"
#include "pages/JammingPage.h"
#include "pages/LocationPage.h"
#include "pages/RadarPage.h"
#include "pages/DigitalPage.h"
#include "pages/AntennaPage.h"
#include "pages/ReferencePage.h"

#include <QBrush>
#include <QColor>
#include <QFont>
#include <QHBoxLayout>
#include <QIcon>
#include <QListWidget>
#include <QListWidgetItem>
#include <QStackedWidget>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , nav_(new QListWidget)
    , stack_(new QStackedWidget)
{
    setWindowTitle(QStringLiteral("EW Calculator"));
    setMinimumSize(980, 640);

    // ── Sidebar helpers ───────────────────────────────────────────────────
    nav_->setMaximumWidth(170);
    nav_->setMinimumWidth(145);
    nav_->setSpacing(1);

    // Adds a non-selectable section header row
    auto addHeader = [this](const QString& text) {
        auto* item = new QListWidgetItem(text.toUpper());
        item->setFlags(Qt::NoItemFlags);
        QFont f = item->font();
        f.setPointSizeF(f.pointSizeF() * 0.80);
        f.setBold(true);
        item->setFont(f);
        item->setForeground(QBrush(QColor(130, 130, 130)));
        nav_->addItem(item);
        navToStack_.append(-1);
    };

    // Adds a blank spacer row (provides visual gap between groups)
    auto addSpacer = [this]() {
        auto* item = new QListWidgetItem;
        item->setFlags(Qt::NoItemFlags);
        item->setSizeHint(QSize(0, 6));
        nav_->addItem(item);
        navToStack_.append(-1);
    };

    // Adds a page item with optional XDG theme icon (graceful fallback)
    auto addPage = [this](const QString& label, const QString& iconName, QWidget* page) {
        auto* item = new QListWidgetItem(label);
        const auto icon = QIcon::fromTheme(iconName);
        if (!icon.isNull())
            item->setIcon(icon);
        nav_->addItem(item);
        navToStack_.append(stack_->count());
        stack_->addWidget(page);
    };

    // ── Calculators section ─────────────────────────────────────────────
    addHeader(QStringLiteral("Calculators"));
    addPage(QStringLiteral("Propagation"),   QStringLiteral("network-wireless"),         new PropagationPage);
    addPage(QStringLiteral("Antenna"),       QStringLiteral("network-wireless"),         new AntennaPage);
    addPage(QStringLiteral("Link Budget"),   QStringLiteral("network-transmit-receive"), new LinkPage);
    addPage(QStringLiteral("Receiver"),      QStringLiteral("audio-card"),              new ReceiverPage);
    addPage(QStringLiteral("Jamming"),       QStringLiteral("emblem-important"),        new JammingPage);
    addPage(QStringLiteral("Location"),      QStringLiteral("find-location"),           new LocationPage);
    addPage(QStringLiteral("Radar"),         QStringLiteral("system-search"),           new RadarPage);
    addPage(QStringLiteral("Digital / DSSS"),QStringLiteral("media-playback-start"),    new DigitalPage);

    // ── Reference section ───────────────────────────────────────────────
    addSpacer();
    addHeader(QStringLiteral("Reference"));
    addPage(QStringLiteral("Reference"),     QStringLiteral("help-contents"),           new ReferencePage);

    // Select first real page (index 1 — after the "Calculators" header)
    nav_->setCurrentRow(1);

    // currentRowChanged maps through navToStack_ so headers/spacers are skipped
    connect(nav_, &QListWidget::currentRowChanged, this,
            [this](int row) {
                if (row < 0 || row >= navToStack_.size()) return;
                const int stackIdx = navToStack_[row];
                if (stackIdx >= 0) stack_->setCurrentIndex(stackIdx);
            });

    // ── Layout ────────────────────────────────────────────────────────────────
    auto* central = new QWidget;
    auto* layout  = new QHBoxLayout(central);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);
    layout->addWidget(nav_);
    layout->addWidget(stack_, 1);
    setCentralWidget(central);
}
