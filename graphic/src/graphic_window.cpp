#include <graphic_window.hpp>

#include <qgridlayout.h>
#include <qlineseries.h>
#include <qvalueaxis.h>
#include <qchartview.h>
#include <qchart.h>

using namespace QtCharts;

QLineSeries *make_line_series(points_info const &info)
{
    auto series = new QLineSeries;
    series->setColor(QColor(Qt::GlobalColor::blue));
    for (std::size_t i{}; i < info.count; ++i)
    {
        series->append(info.xarr[i], info.yarr[i]);
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
    auto xaxis = make_value_axis(info.xaxis);
    chart->addAxis(xaxis, Qt::AlignmentFlag::AlignBottom);
    auto yaxis = make_value_axis(info.yaxis);
    chart->addAxis(yaxis, Qt::AlignmentFlag::AlignLeft);
    series->attachAxis(xaxis);
    series->attachAxis(yaxis);
    auto chart_view = new QChartView(chart);
    chart_view->setRenderHint(QPainter::Antialiasing);
    return chart_view;
}

graphic_window::graphic_window(QWidget *parent) : QWidget(parent)
{
    setLayout(new QGridLayout);
}

void graphic_window::add_figure(unsigned row, unsigned column, figure_info const &info)
{
    auto grid = get_layout();
    grid->setRowStretch(row, 1);
    grid->setColumnStretch(column, 1);
    grid->addWidget(make_chart_view(info), row, column);
}

QGridLayout *graphic_window::get_layout() const
{
    return qobject_cast<QGridLayout *>(layout());
}
