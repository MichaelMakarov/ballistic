#include <plotview.h>
#include <mainmodel.h>
#include <guisettings.h>

#include <qpen.h>
#include <qlayout.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qchart.h>
#include <qchartview.h>
#include <qlineseries.h>
#include <qscatterseries.h>
#include <qvalueaxis.h>

using namespace QtCharts;

QValueAxis* make_axis(const QString& title) {
    auto axis = new QValueAxis;
    axis->setGridLineVisible(true);
    axis->setTitleText(title);
    axis->setTitleVisible(true);
    axis->setLabelFormat("%.1f");
    axis->setMinorTickCount(10);
    return axis;
}

QChartView* make_chartview(const QString& name, const QString& vlabel) {
    QPen pen;
    pen.setColor(QColor(Qt::GlobalColor::black));
    pen.setWidth(3);

    auto line_series = new QLineSeries;
    line_series->setName(name);
    line_series->setPen(pen);

    auto scatter_series = new QScatterSeries;
    scatter_series->setName(name);
    scatter_series->setPen(pen);
    scatter_series->setMarkerShape(QScatterSeries::MarkerShape::MarkerShapeCircle);
    scatter_series->setMarkerSize(10);

    auto chart = new QChart;
    chart->addSeries(line_series);
    chart->addSeries(scatter_series);

    auto x_axis = make_axis("t, сут");
    auto y_axis = make_axis(vlabel);
    chart->addAxis(x_axis, Qt::AlignmentFlag::AlignBottom);
    chart->addAxis(y_axis, Qt::AlignmentFlag::AlignLeft);
    line_series->attachAxis(x_axis);
    line_series->attachAxis(y_axis);
    scatter_series->attachAxis(x_axis);
    scatter_series->attachAxis(y_axis);

    auto plot = new QChartView(chart);
    plot->setRenderHint(QPainter::Antialiasing);
    
    return plot;
}

template<typename T>
T* cast_series(QAbstractSeries* series) {
    auto ptr = dynamic_cast<T*>(series);
    if (!ptr) throw std::bad_cast();
    return ptr;
}

auto get_series(QChartView* chartview) -> std::tuple<QLineSeries*, QScatterSeries*>
{
    auto all_series = chartview->chart()->series();
    return { 
        cast_series<QLineSeries>(all_series[0]), 
        cast_series<QScatterSeries>(all_series[1]) 
    };
}

void clear_chartview(QChartView* chartview)
{
    auto [line_series, scatter_series] = get_series(chartview);
    line_series->clear();
    scatter_series->clear();
    chartview->repaint();
}

void replot_chartview(QChartView* chartview, const residuals_info& residuals, const interval_info& interval)
{
    chartview->setWindowTitle(
        QString{ "Среднее значение = %1 СКО = %2" }
            .arg(QString::number(residuals.mean, 'f', 3), QString::number(residuals.std, 'f', 3))
    );

    auto [line_series, scatter_series] = get_series(chartview);
    line_series->clear();
    scatter_series->clear();
    if (!residuals.array.empty()) {
        double maxt{};
        for (size_t i{}; i < residuals.array.size(); ++i) {
            maxt = ((interval.orb_begin + i)->t - interval.orb_begin->t) / 86400;
            line_series->append(maxt, residuals.array[i]);
            scatter_series->append(maxt, residuals.array[i]);
        }
        auto minmaxr = std::minmax_element(std::begin(residuals.array), std::end(residuals.array));
        auto chart = chartview->chart();
        chart->axes(Qt::Orientation::Vertical).front()->setRange(0.9 * (*minmaxr.first), 1.1 * (*minmaxr.second));
        chart->axes(Qt::Orientation::Horizontal).front()->setRange(0, maxt);
    }
    chartview->update();
}

void replot_chartview(QChartView* chartview, const interval_info& interval)
{
    auto [line_series, scatter_series] = get_series(chartview);
    line_series->clear();
    scatter_series->clear();
    if (std::distance(interval.rot_begin, interval.rot_end) > 0) {
        double maxt{};
        for (auto it = interval.rot_begin; it != interval.rot_end; ++it) {
            maxt = (it->t - interval.rot_begin->t) / 86400;
            line_series->append(maxt, it->s);
            scatter_series->append(maxt, it->s);
        }
        auto minmaxr = std::minmax_element(
            interval.rot_begin, interval.rot_end, 
            [](const auto& left, const auto& right){ return left.s < right.s; }
        );
        auto chart = chartview->chart();
        chart->axes(Qt::Orientation::Vertical).front()->setRange(0.9 * (minmaxr.first->s), 1.1 * (minmaxr.second->s));
        chart->axes(Qt::Orientation::Horizontal).front()->setRange(0, maxt);
    }
    chartview->update();
}


plotview::plotview(const computation* const comp, QWidget* const parent) : QWidget(parent)
{
    _comp = comp;  

    auto grid = new QGridLayout;
    setLayout(grid);

    auto tab = new QTabWidget;
    grid->addWidget(tab);

    auto magn = make_chartview("Зв.величина", "s");
    auto init = make_chartview("Невязки", "|r|, м");
    auto prev = make_chartview("Невязки", "|r|, м");
    auto next = make_chartview("Невязки", "|r|, м");

    tab->addTab(magn, "Звёздная величина");
    tab->addTab(init, "Исходные (без уточнения)");
    tab->addTab(prev, "Уточнённые (без вращения)");
    tab->addTab(next, "Уточнённые (с вращением)");

    connect(
        this, &plotview::clear_required, 
        this, [init, prev, next, magn, this](){
            clear_chartview(magn);
            clear_chartview(init);
            clear_chartview(prev);
            clear_chartview(next);
        }
    );
    connect(
        this, &plotview::replot_required,
        this, [init, prev, next, magn, this](){
            replot_chartview(magn, _comp->interval);
            replot_chartview(init, _comp->init_motion.r, _comp->interval);
            replot_chartview(prev, _comp->prev_motion.r, _comp->interval);
            replot_chartview(next, _comp->next_motion.r, _comp->interval);
        }
    );
}

void plotview::clear()
{
    emit clear_required();
}

void plotview::replot()
{
    emit replot_required();
}