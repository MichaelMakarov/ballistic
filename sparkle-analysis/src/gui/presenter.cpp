#include <presenter.hpp>
#include <formatoutput.hpp>
#include <computation.hpp>
#include <mainmodel.hpp>
#include <gui.hpp>
#include <qlayout.h>
#include <qlabel.h>
#include <qscrollarea.h>
#include <qgroupbox.h>
#include <qpen.h>
#include <qtabwidget.h>
#include <qchart.h>
#include <qchartview.h>
#include <qlineseries.h>
#include <qscatterseries.h>
#include <qvalueaxis.h>

using namespace QtCharts;

computation_presenter::computation_presenter(const computational_model *model, QWidget *const parent) : QWidget(parent)
{
    auto grid = new QHBoxLayout;
    setLayout(grid);

    auto layout = make_layout<QVBoxLayout>();
    grid->addWidget(make_groupbox("Результаты расчёта", layout));

    auto label = new QLabel;
    label->setAlignment(Qt::AlignmentFlag::AlignTop);
    auto scroll = new QScrollArea;
    scroll->setWidget(label);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
    scroll->setAlignment(Qt::AlignmentFlag::AlignTop);
    scroll->setWidgetResizable(true);
    layout->addWidget(scroll);
    _label = label;

    connect(model, &computational_model::computation_performed, this, &computation_presenter::update_content);
}

void computation_presenter::update_content(const computational_output *output)
{
    std::stringstream str;
    print_output(str, *output, false);
    _label->setText(QString::fromStdString(str.str()));
}

QValueAxis *make_axis(const QString &title)
{
    auto axis = new QValueAxis;
    axis->setGridLineVisible(true);
    axis->setTitleText(title);
    axis->setTitleVisible(true);
    axis->setLabelFormat("%.1f");
    axis->setMinorTickCount(10);
    return axis;
}

auto make_series(const QString &name, Qt::GlobalColor color)
{
    QPen pen;
    pen.setColor(QColor(color));
    pen.setWidth(3);

    auto line_series = new QLineSeries;
    line_series->setName(name);
    line_series->setPen(pen);

    auto scatter_series = new QScatterSeries;
    scatter_series->setName(name);
    scatter_series->setPen(pen);
    scatter_series->setMarkerShape(QScatterSeries::MarkerShape::MarkerShapeCircle);
    scatter_series->setMarkerSize(10);

    return std::make_tuple(line_series, scatter_series);
}

QChartView *make_chartview(const QString &name, const QString &vlabel)
{
    auto [inc_line, inc_scatter] = make_series("невязки по наклонению", Qt::GlobalColor::blue);
    auto [asc_line, asc_scatter] = make_series("невязки по восхождению", Qt::GlobalColor::green);

    auto chart = new QChart;
    chart->addSeries(inc_line);
    chart->addSeries(inc_scatter);
    chart->addSeries(asc_line);
    chart->addSeries(asc_scatter);

    auto x_axis = make_axis("t, сут");
    auto y_axis = make_axis(vlabel);
    chart->addAxis(x_axis, Qt::AlignmentFlag::AlignBottom);
    chart->addAxis(y_axis, Qt::AlignmentFlag::AlignLeft);
    inc_line->attachAxis(x_axis);
    inc_line->attachAxis(y_axis);
    inc_scatter->attachAxis(x_axis);
    inc_scatter->attachAxis(y_axis);

    auto plot = new QChartView(chart);
    plot->setRenderHint(QPainter::Antialiasing);

    return plot;
}

template <typename T>
T *cast_series(QAbstractSeries *series)
{
    auto ptr = dynamic_cast<T *>(series);
    if (!ptr)
        throw std::bad_cast();
    return ptr;
}

auto to_qstring(double value)
{
    return QString::number(value, 'f', 3);
}

// auto mean_std(const_residual_vector<2> res) -> std::tuple<double, double, double, double>;

double sec_to_days(double sec);

class series_pair
{
    QLineSeries *_line{nullptr};
    QScatterSeries *_scatter{nullptr};

public:
    series_pair() = default;
    series_pair(QLineSeries *line, QScatterSeries *scatter) : _line{line}, _scatter{scatter} {}
    series_pair(const series_pair &) noexcept = default;
    series_pair &operator=(const series_pair &) noexcept = default;

    void clear() const
    {
        _line->clear();
        _scatter->clear();
    }
    void append(double x, double y) const
    {
        _line->append(x, y);
        _scatter->append(x, y);
    }
};

template <size_t n>
class chartview_interface;

template <>
class chartview_interface<1>
{
    series_pair _series;
    QChartView *_view;

public:
    chartview_interface(QChartView *view) : _view{view}
    {
        auto series = view->chart()->series();
        _series = series_pair{cast_series<QLineSeries>(series[0]), cast_series<QScatterSeries>(series[1])};
    }
    void clear() const { _series.clear(); }
    void update(const measuring_interval &inter) const
    {
        if (inter.points_count() > 0)
        {
            auto &fm = std::begin(inter).measurement();
            double xmax{}, ymin{fm.m}, ymax{fm.m};
            for (auto iter = std::begin(inter); iter != std::end(inter); ++iter)
            {
                auto &cm = iter.measurement();
                xmax = sec_to_days(cm.t - fm.t);
                _series.append(xmax, cm.m);
                ymin = std::min(ymin, cm.m);
                ymax = std::max(ymax, cm.m);
            }
            auto chart = _view->chart();
            chart->axes(Qt::Orientation::Horizontal).front()->setRange(0, xmax);
            chart->axes(Qt::Orientation::Vertical).front()->setRange(0.9 * ymin, 1.1 * ymax);
        }
        _view->update();
    }
};

template <>
class chartview_interface<2>
{
    series_pair _first, _second;
    QChartView *_view;

public:
    chartview_interface(QChartView *view) : _view{view}
    {
        auto series = view->chart()->series();
        _first = series_pair{cast_series<QLineSeries>(series[0]), cast_series<QScatterSeries>(series[1])};
        _second = series_pair{cast_series<QLineSeries>(series[2]), cast_series<QScatterSeries>(series[3])};
    }
    void clear() const
    {
        _first.clear();
        _second.clear();
    }
    // void update(const measuring_interval &inter, const_residual_vector<2> resid) const
    // {
    //     const QString fmt{"Накл: ср.знач = %1 СКО = %2\nВосх: ср.знач = %3 СКО = %4"};
    //     auto [mean_incl, std_incl, mean_asc, std_asc] = mean_std(resid);
    //     _view->setWindowTitle(fmt.arg(to_qstring(mean_incl), to_qstring(std_incl), to_qstring(mean_asc), to_qstring(std_asc)));
    //     if (resid.count() > 0) {
    //         auto& fm = std::begin(inter).measurement();
    //         double xmax{ }, ymin{ std::numeric_limits<double>::max() }, ymax{};
    //         auto miter = std::begin(inter);
    //         for (size_t i{}; i < resid.count(); ++i) {
    //             auto arr = resid.point(i);
    //             ymin = std::min(std::min(arr[0], arr[1]), ymin);
    //             ymax = std::max(std::min(arr[0], arr[1]), ymax);
    //             xmax = sec_to_days(miter.measurement().t - fm.t);
    //             _first.append(xmax, arr[0]);
    //             _second.append(xmax, arr[1]);
    //         }
    //         auto chart = _view->chart();
    //         chart->axes(Qt::Orientation::Horizontal).front()->setRange(0, xmax);
    //         chart->axes(Qt::Orientation::Vertical).front()->setRange(0.9 * ymin, 1.1 * ymax);
    //     }
    //     _view->update();
    // }
};

graphics_view::graphics_view(const computational_model *model, QWidget *const parent) : QWidget(parent)
{
    auto grid = new QGridLayout;
    setLayout(grid);

    auto tab = new QTabWidget;
    grid->addWidget(tab);

    const char *res_label{"Невязки"};
    const char *res_value_label{"|r|, град"};

    _magn = make_chartview("Зв.величина", "m");
    _init = make_chartview(res_label, res_value_label);
    _base = make_chartview(res_label, res_value_label);
    _next = make_chartview(res_label, res_value_label);

    tab->addTab(_magn, "Звёздная величина");
    tab->addTab(_init, "Исходные (без уточнения)");
    tab->addTab(_base, "Уточнённые (без вращения)");
    tab->addTab(_next, "Уточнённые (с вращением)");

    connect(model, &computational_model::computation_performed, this, &graphics_view::update_content);
}

void graphics_view::update_content(const computational_output *ptr)
{
    chartview_interface<1> magn{_magn};
    chartview_interface<2> init{_init};
    chartview_interface<2> base{_base};
    chartview_interface<2> next{_next};
    magn.clear();
    init.clear();
    base.clear();
    next.clear();

    if (ptr->inter)
    {
        auto &inter = *ptr->inter;
        magn.update(inter);
        if (ptr->basic)
        {
            auto &basic = *ptr->basic;
            if (!basic.l.empty())
            {
                // init.update(inter, const_residual_vector<2>{ &basic.l.front().residuals });
                // base.update(inter, const_residual_vector<2>{ &basic.l.back().residuals });
            }
        }
    }
}