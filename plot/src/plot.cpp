#include <plot.hpp>
#include <qapplication.h>
#include <mutex>
#include <memory>
#include <qchart.h>
#include <qchartview.h>
#include <qlineseries.h>
#include <qscatterseries.h>
#include <qvalueaxis.h>

using namespace QtCharts;

namespace
{
    std::unique_ptr<QApplication> app_ptr;
    std::mutex sync_obj;

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

    auto make_chartview(graphic_info const &info)
    {
        auto x_axis = make_axis(info.xlbl);
        auto y_axis = make_axis(info.ylbl);
        auto line_series = make_line_series("рад.-вектор", QColor(Qt::GlobalColor::black));
        auto chart = new QChart;
        chart->addAxis(x_axis, Qt::AlignmentFlag::AlignBottom);
        chart->addAxis(y_axis, Qt::AlignmentFlag::AlignLeft);
        chart->addSeries(line_series);
        line_series->attachAxis(x_axis);
        line_series->attachAxis(y_axis);
        double xmin, xmax, ymin, ymax;
        xmin = xmax = info.x[0];
        ymin = ymax = info.y[0];
        for (std::size_t i{}; i < info.count; ++i)
        {
            line_series->append(info.x[i], info.y[i]);
            xmin = std::min(xmin, info.x[i]);
            xmax = std::max(xmax, info.x[i]);
            ymin = std::min(ymin, info.y[i]);
            ymax = std::max(ymax, info.y[i]);
        }
        double dx = (xmax - xmin) * 1e-2;
        double dy = (ymax - ymin) * 1e-2;
        x_axis->setRange(xmin - dx, xmax + dx);
        y_axis->setRange(ymin - dy, ymax + dy);
        auto view = new QChartView(chart);
        view->setRenderHint(QPainter::RenderHint::Antialiasing);
        view->setWindowTitle(info.title);
        return view;
    }
}

void graphic_app::init(int argc, char **argv)
{
    std::lock_guard<std::mutex> lock{sync_obj};
    app_ptr.reset(new QApplication(argc, argv));
}

void graphic_app::show(graphic_info const *infos, unsigned count)
{
    std::vector<std::unique_ptr<QWidget>> windows(count);
    for (unsigned i{}; i < count; ++i)
    {
        windows[i].reset(make_chartview(infos[i]));
        windows[i]->show();
    }
    auto res = app_ptr->exec();
    if (0 != res)
    {
        throw std::runtime_error("An error occured while graphic application ran. Exec returned " + std::to_string(res) + ".");
    }
}
