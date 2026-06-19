// main.cpp
#include "MainWindow.h"
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("ewcalc"));
    app.setApplicationDisplayName(QStringLiteral("EW Calculator"));
    app.setApplicationVersion(QStringLiteral("0.6.6"));

    MainWindow w;
    w.show();
    return app.exec();
}
