// main.cpp
#include "MainWindow.h"
#include "Settings.h"
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("ewcalc"));
    app.setApplicationDisplayName(QStringLiteral("EW Calculator"));
    app.setApplicationVersion(QStringLiteral("0.7.0"));

    // Ensure any pending debounced settings writes are flushed before exit.
    QObject::connect(&app, &QApplication::aboutToQuit,
                      [] { AppSettings::instance().flush(); });

    MainWindow w;
    w.show();
    return app.exec();
}
