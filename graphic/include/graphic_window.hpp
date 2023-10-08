#pragma once

#include <figure_info.hpp>

#include <qwidget.h>

class graphic_window : public QWidget
{
public:
    graphic_window(QWidget *parent = nullptr);

    void add_figure(unsigned row, unsigned column, figure_info const &info);

private:
    void save_graphic();

private:
    class QGridLayout *_grid;
    QWidget *_wgt;
};