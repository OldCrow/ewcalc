#pragma once
#include <QWidget>

#include <cstddef>

/// Renders one ewpresenter::refdata page (#74: the data layer publishes
/// several — quick-values, ref-propagation, ref-db-units, ...). MainWindow
/// adds one ReferencePage per refdata::pages() entry, keyed by index; the
/// page's title lives on its sidebar row (see MainWindow.cpp's addPage),
/// matching how every other page is titled — ReferencePage itself renders
/// only the page's sections.
class ReferencePage : public QWidget
{
public:
    explicit ReferencePage(std::size_t pageIndex, QWidget* parent = nullptr);
};
