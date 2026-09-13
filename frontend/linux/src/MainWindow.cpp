// MainWindow.cpp
#include "MainWindow.h"
#include "Settings.h"
#include "pages/PropagationPage.h"
#include "pages/LinkPage.h"
#include "pages/ReceiverPage.h"
#include "pages/JammingPage.h"
#include "pages/LocationPage.h"
#include "pages/RadarPage.h"
#include "pages/DetectionPage.h"
#include "pages/DopplerPage.h"
#include "pages/DigitalPage.h"
#include "pages/AntennaPage.h"
#include "pages/ReferencePage.h"

#include <ewpresenter/reference_data.h>

#include <cstddef>

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
#include <QPainter>
#include <QPixmap>
#include <QProcess>
#include <QPushButton>
#include <QStackedWidget>
#include <QString>
#include <QStyle>
#include <QVBoxLayout>
#include <QWidget>

namespace {

/// Sidebar icon per stable refdata page id (#74). A reference page whose
/// domain mirrors a calculator reuses that calculator's exact icon name —
/// "network-wireless", matching the Propagation row below — so the two read
/// as the same concept; quick-values keeps the pre-#74 Reference icon;
/// ref-db-units gets its own distinct icon; an unknown future id falls back
/// to the Reference icon. addPage() applies the plain-then-"-symbolic"
/// theme fallback uniformly, so only the plain name is chosen here.
QString referencePageIcon(const QString& pageId)
{
    if (pageId == QStringLiteral("ref-propagation"))
        return QStringLiteral("network-wireless");
    if (pageId == QStringLiteral("ref-antennas"))
        return QStringLiteral("network-wireless"); // matches the Antenna row
    if (pageId == QStringLiteral("ref-link"))
        return QStringLiteral("network-transmit-receive"); // matches the Link Budget row
    if (pageId == QStringLiteral("ref-bands"))
        return QStringLiteral("accessories-character-map");
    if (pageId == QStringLiteral("ref-receiver"))
        return QStringLiteral("audio-card"); // matches the Receiver row
    if (pageId == QStringLiteral("ref-jamming"))
        return QStringLiteral("emblem-important"); // matches the Jamming row
    if (pageId == QStringLiteral("ref-location"))
        return QStringLiteral("mark-location"); // matches the Location row
    if (pageId == QStringLiteral("ref-radar-det"))
        return QStringLiteral("system-search"); // matches the Radar row
    if (pageId == QStringLiteral("ref-doppler"))
        return QStringLiteral("media-playlist-shuffle"); // matches the Doppler & Resolution row
    if (pageId == QStringLiteral("ref-digital"))
        return QStringLiteral("media-playback-start"); // matches the Digital / DSSS row
    if (pageId == QStringLiteral("ref-rcs"))
        return QStringLiteral("airplane-mode");
    if (pageId == QStringLiteral("ref-glossary"))
        return QStringLiteral("accessories-dictionary");
    if (pageId == QStringLiteral("ref-db-units"))
        return QStringLiteral("accessories-calculator");
    return QStringLiteral("help-contents");
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , nav_(new QListWidget)
    , stack_(new QStackedWidget)
{
    setWindowTitle(QStringLiteral("EW Calculator"));
    setMinimumSize(980, 640);
    // The minimum is the floor at which every page is still usable, not a
    // comfortable default: at exactly 980 the widest Reference rows overflow
    // and the page opens with a horizontal scrollbar. Without an explicit
    // resize() the window launches at its minimum, so set a roomier default.
    resize(1180, 800);

    // ── Sidebar helpers ───────────────────────────────────────────────────
    // Wide enough for the longest label ("Doppler & Resolution") *plus* a row
    // icon; at the previous 170 the label clipped and the nav grew its own
    // horizontal scrollbar on themes that supply icons for every row.
    nav_->setMaximumWidth(210);
    nav_->setMinimumWidth(185);
    nav_->setSpacing(1);

    // Every row's label must start at the same x. The item delegate sizes a
    // row's decoration from the icon's actualSize(), so an icon the theme
    // ships only at a smaller *fixed* size (Yaru's "system-search" exists
    // solely as a 16 px legacy entry, against the 24 px list-icon size) gets a
    // narrower slot and shifts that row's label left. Pin the view's icon size
    // so addPage() can pad such icons up to it.
    const int navIconPx = nav_->style()->pixelMetric(QStyle::PM_ListViewIconSize, nullptr, nav_);
    const QSize navIconSize(navIconPx, navIconPx);
    nav_->setIconSize(navIconSize);

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
    auto addPage = [this, navIconSize](const QString& label, const QString& iconName, QWidget* page) {
        auto* item = new QListWidgetItem(label);
        // Current GNOME icon themes (Adwaita 46+, Yaru) ship most of these
        // names only in their "-symbolic" form; the full-colour legacy names
        // resolve to nothing, and QIcon::fromTheme fails silently, leaving
        // the row icon-less. Try the plain name first (themes that still
        // carry full-colour variants keep them) and fall back to symbolic.
        auto icon = QIcon::fromTheme(iconName);
        if (icon.isNull())
            icon = QIcon::fromTheme(iconName + QStringLiteral("-symbolic"));
        // Pad a smaller-than-slot icon onto a transparent canvas of the pinned
        // size, centred (QIcon::paint's default alignment) — the row keeps a
        // full-width decoration without upscaling the icon into a blur.
        if (!icon.isNull() && icon.actualSize(navIconSize) != navIconSize) {
            const qreal dpr = nav_->devicePixelRatioF();
            QPixmap canvas(navIconSize * dpr);
            canvas.setDevicePixelRatio(dpr);
            canvas.fill(Qt::transparent);
            QPainter painter(&canvas);
            icon.paint(&painter, QRect(QPoint(0, 0), navIconSize));
            painter.end();
            icon = QIcon(canvas);
        }
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
    addPage(QStringLiteral("Comms Jamming"), QStringLiteral("emblem-important"),        new JammingPage);
    // "find-location" isn't a standard XDG icon name and resolves to nothing
    // on most icon themes; "mark-location" is the widely-shipped equivalent.
    addPage(QStringLiteral("Location"),      QStringLiteral("mark-location"),           new LocationPage);
    addPage(QStringLiteral("Radar"),         QStringLiteral("system-search"),           new RadarPage);
    addPage(QStringLiteral("Detection"),     QStringLiteral("edit-find"),               new DetectionPage);
    addPage(QStringLiteral("Doppler & Resolution"), QStringLiteral("media-playlist-shuffle"), new DopplerPage);
    addPage(QStringLiteral("Digital / DSSS"),QStringLiteral("media-playback-start"),    new DigitalPage);

    // ── Reference section ───────────────────────────────────────────────
    // One sidebar row per refdata page (#74): the sidebar renders whatever
    // ewpresenter::refdata publishes, so a new data-layer page becomes a nav
    // entry with no MainWindow change.
    addSpacer();
    addHeader(QStringLiteral("Reference"));
    const auto refPages = ewpresenter::refdata::pages();
    for (std::size_t i = 0; i < refPages.size(); ++i) {
        const auto& page = refPages[i];
        addPage(QString::fromUtf8(page.title),
                referencePageIcon(QString::fromUtf8(page.id)),
                new ReferencePage(i));
    }

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
