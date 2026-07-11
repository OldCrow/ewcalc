// MainWindow.cpp
#include "MainWindow.h"
#include "Settings.h"
#include "pages/PropagationPage.h"
#include "pages/LinkPage.h"
#include "pages/ReceiverPage.h"
#include "pages/JammingPage.h"
#include "pages/LocationPage.h"
#include "pages/RadarPage.h"
#include "pages/DigitalPage.h"
#include "pages/AntennaPage.h"
#include "pages/ReferencePage.h"

#include <QAction>
#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QFont>
#include <QHBoxLayout>
#include <QIcon>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
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
    // "find-location" isn't a standard XDG icon name and resolves to nothing
    // on most icon themes; "mark-location" is the widely-shipped equivalent.
    addPage(QStringLiteral("Location"),      QStringLiteral("mark-location"),           new LocationPage);
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

    // ── Sidebar: nav list + a visible "Reset to Defaults" button ───────────
    // Matches the always-visible placement Windows (nav-pane footer button)
    // and macOS (toolbar button) use; previously Linux only exposed this via
    // the File menu, which was easy to miss.
    auto* sidebar    = new QWidget;
    auto* sidebarBox = new QVBoxLayout(sidebar);
    sidebarBox->setContentsMargins(0, 0, 0, 0);
    sidebarBox->setSpacing(4);
    sidebarBox->addWidget(nav_, 1);

    auto* resetButton = new QPushButton(tr("Reset to Defaults\u2026"));
    resetButton->setToolTip(tr("Clear all saved inputs and restart with default values"));
    resetButton->setAccessibleName(tr("Reset all calculators to default values"));
    sidebarBox->addWidget(resetButton);

    // ── Layout ────────────────────────────────────────────────────────────────
    auto* central = new QWidget;
    auto* layout  = new QHBoxLayout(central);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);
    layout->addWidget(sidebar);
    layout->addWidget(stack_, 1);
    setCentralWidget(central);

    // ── Reset persisted inputs (issue #20): wired to both the File menu
    // item and the sidebar button below so it's reachable either way.
    auto promptResetToDefaults = [this] {
        const auto choice = QMessageBox::question(
            this, tr("Reset to Defaults"),
            tr("This clears all saved inputs and restarts EW Calculator with "
               "default values. Continue?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (choice != QMessageBox::Yes)
            return;
        AppSettings::instance().resetAll();
        QProcess::startDetached(QApplication::applicationFilePath(), QApplication::arguments().mid(1));
        QApplication::quit();
    };

    auto* fileMenu = menuBar()->addMenu(tr("&File"));
    auto* resetAction = fileMenu->addAction(tr("Reset to &Defaults\u2026"));
    resetAction->setToolTip(tr("Clear all saved inputs and restart with default values"));
    connect(resetAction, &QAction::triggered, this, promptResetToDefaults);
    connect(resetButton, &QPushButton::clicked, this, promptResetToDefaults);
}
