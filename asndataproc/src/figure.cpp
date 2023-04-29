#include <figure.hpp>
#include <qchartview.h>
#include <qchart.h>
#include <qlineseries.h>
#include <qscatterseries.h>
#include <qvalueaxis.h>
#include <qtabwidget.h>
#include <qapplication.h>
#include <formatting.hpp>
#include <mutex>

using namespace QtCharts;

auto make_line_series(QString const &name, QColor const &color)
{
    auto series = new QLineSeries;
    series->setName(name);
    series->setPen(QPen(color, 3));
    return series;
}

auto make_scatter_series(QString const &name, QColor const &color)
{
    auto series = new QScatterSeries;
    series->setName(name);
    series->setPen(QPen(color, 3));
    series->setMarkerShape(QScatterSeries::MarkerShape::MarkerShapeCircle);
    series->setMarkerSize(10);
    return series;
}

auto make_axis(QString const &name)
{
    auto axis = new QValueAxis;
    axis->setGridLineVisible(true);
    axis->setTitleText(name);
    axis->setTitleVisible(true);
    axis->setLabelFormat("%.1f");
    axis->setMinorTickCount(1);
    return axis;
}

auto get_radius_color()
{
    return QColor(Qt::GlobalColor::red);
}

auto get_velocity_color()
{
    return QColor(Qt::GlobalColor::blue);
}

auto make_chartview(double const *x, double const *y, std::size_t count)
{
    auto x_axis = make_axis("№");
    auto y_axis = make_axis("|r|, м");
    auto line_series = make_line_series("рад.-вектор", get_radius_color());
    auto chart = new QChart;
    chart->addAxis(x_axis, Qt::AlignmentFlag::AlignBottom);
    chart->addAxis(y_axis, Qt::AlignmentFlag::AlignLeft);
    chart->addSeries(line_series);
    line_series->attachAxis(x_axis);
    line_series->attachAxis(y_axis);
    double xmin, xmax, ymin, ymax;
    xmin = xmax = x[0];
    ymin = ymax = y[0];
    for (std::size_t i{}; i < count; ++i)
    {
        line_series->append(x[i], y[i]);
        xmin = std::min(xmin, x[i]);
        xmax = std::max(xmax, x[i]);
        ymin = std::min(ymin, y[i]);
        ymax = std::max(ymax, y[i]);
    }
    double dx = (xmax - xmin) * 1e-2;
    double dy = (ymax - ymin) * 1e-2;
    x_axis->setRange(xmin - dx, xmax + dx);
    y_axis->setRange(ymin - dy, ymax + dy);
    auto view = new QChartView(chart);
    view->setRenderHint(QPainter::RenderHint::Antialiasing);
    return view;
}

std::unique_ptr<QApplication> figure_provider::app;

void figure_provider::initialize(int argc, char **argv)
{
    if (!app)
    {
        app = std::make_unique<QApplication>(argc, argv);
    }
}

void figure_provider::show_residuals(double const *x1, double const *y1, double const *x2, double const *y2, std::size_t count)
{
    auto tab = new QTabWidget;
    tab->setWindowTitle("Отображение невязок");
    tab->addTab(make_chartview(x1, y1, count), "Первая итерация");
    tab->addTab(make_chartview(x2, y2, count), "Последняя итерация");
    tab->setMinimumSize(500, 500);
    tab->showMaximized();
    auto ret = app->exec();
    if (0 != ret)
    {
        throw_runtime_error("Application execution finished with code % not equal to 0.", ret);
    }
}