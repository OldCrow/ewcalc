// main.cpp
#include "MainWindow.h"
#include "Settings.h"
#include <QApplication>
#include <QGuiApplication>
#include <QIcon>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("ewcalc"));
    app.setApplicationDisplayName(QStringLiteral("EW Calculator"));
    app.setApplicationVersion(QStringLiteral("1.1.0"));

    // Ties the running app to assets/linux/ewcalc.desktop: this sets the
    // Wayland app_id and the X11 desktop-entry hint, which is what the shell
    // matches on to pick the dock/taskbar icon. setWindowIcon() below only
    // covers the in-window icon — GNOME ignores it for the dock — so without
    // this the dock falls back to a generic icon even once the .desktop and
    // hicolor PNGs are installed by the .deb/.rpm.
    QGuiApplication::setDesktopFileName(QStringLiteral("ewcalc"));

    // Set the window/taskbar icon explicitly rather than relying on the
    // window manager to match WM_CLASS against the installed .desktop entry
    // (unreliable when run outside a fully XDG-integrated session, e.g. an
    // un-integrated AppImage) — falls back to a generic "gears" icon
    // otherwise. Icons are embedded via resources/icons.qrc so this works
    // regardless of installation/theme state.
    QIcon appIcon;
    for (int size : {16, 32, 48, 64, 128, 256, 512})
        appIcon.addFile(QStringLiteral(":/icons/ewcalc-%1.png").arg(size), QSize(size, size));
    app.setWindowIcon(appIcon);

    // Ensure any pending debounced settings writes are flushed before exit.
    QObject::connect(&app, &QApplication::aboutToQuit,
                      [] { AppSettings::instance().flush(); });

    MainWindow w;
    w.show();
    return app.exec();
}
