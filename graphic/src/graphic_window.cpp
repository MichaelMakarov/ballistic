#include <graphic_window.hpp>

#include <qmenubar.h>
#include <qfiledialog.h>
#include <qgridlayout.h>
#include <qlineseries.h>
#include <qvalueaxis.h>
#include <qchartview.h>
#include <qchart.h>

#include <iostream>

namespace
{
    using namespace QtCharts;

    QLineSeries *make_line_series(points_info const &info)
    {
        auto series = new QLineSeries;
        series->setColor(QColor(Qt::GlobalColor::blue));
        for (std::size_t i{}; i < info.count; ++i)
        {
            series->append(info.x_array[i], info.y_array[i]);
        }
        return series;
    }

    QValueAxis *make_value_axis(axis_info const &info)
    {
        auto axis = new QValueAxis;
        axis->setTitleText(QString::fromStdString(info.name));
        return axis;
    }

    QChartView *make_chart_view(figure_info const &info)
    {
        auto chart = new QChart;
        chart->setTitle(QString::fromStdString(info.title));
        auto series = make_line_series(info.points);
        chart->addSeries(series);
        auto xaxis = make_value_axis(info.x_axis);
        chart->addAxis(xaxis, Qt::AlignmentFlag::AlignBottom);
        auto yaxis = make_value_axis(info.y_axis);
        chart->addAxis(yaxis, Qt::AlignmentFlag::AlignLeft);
        series->attachAxis(xaxis);
        series->attachAxis(yaxis);
        auto chart_view = new QChartView(chart);
        chart_view->setRenderHint(QPainter::Antialiasing);
        return chart_view;
    }

    char const *grid_property_name{"graphic grid"};
    char const *widget_propoerty_name{"graphic widget"};
}

graphic_window::graphic_window(QWidget *parent) : QWidget(parent)
{
    auto layout = new QVBoxLayout;
    setLayout(layout);

    auto menu_bar = new QMenuBar;
    menu_bar->addAction("Сохранить в png", this, &graphic_window::save_graphic);
    layout->addWidget(menu_bar, 0);

    _wgt = new QWidget;
    _wgt->setLayout(_grid = new QGridLayout);
    layout->addWidget(_wgt, 1);
}

void graphic_window::add_figure(unsigned row, unsigned column, figure_info const &info)
{
    _grid->setRowStretch(row, 1);
    _grid->setColumnStretch(column, 1);
    _grid->addWidget(make_chart_view(info), row, column);
}

void graphic_window::save_graphic()
{
    QPixmap pixmap = _wgt->grab();
    auto filename = QFileDialog::getSaveFileName(this, "Графики", QString(), "Файл изображения (*.png)");
    if (!filename.isEmpty())
    {
        if (!pixmap.save(filename))
        {
            std::cerr << "Failed to save graphic to " << filename.toStdString() << std::endl;
        }
    }
}
