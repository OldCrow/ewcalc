#pragma once
/// @file MainWindow.h
/// @brief Top-level window: sidebar navigation + stacked page area.

#include <QMainWindow>
#include <QVector>

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
    /// Maps nav_ row index to stack_ page index. -1 for non-selectable rows
    /// (section headers and spacers).
    QVector<int>    navToStack_;
};
