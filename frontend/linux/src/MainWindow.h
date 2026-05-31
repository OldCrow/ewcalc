#pragma once
/// @file MainWindow.h
/// @brief Top-level window: sidebar navigation + stacked page area.

#include <QMainWindow>

class QListWidget;
class QStackedWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    QListWidget*    nav_;
    QStackedWidget* stack_;
};
