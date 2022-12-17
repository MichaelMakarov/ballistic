#pragma once
#include <qwidget.h>

class computational_output;
class computational_model;

class computation_presenter : public QWidget {
public:
    computation_presenter(const computational_model* model, QWidget* const parent = nullptr);
    void update_content(const computational_output*);
private:
    class QLabel* _label;
};

namespace QtCharts {
    class QChartView;
}

class graphics_view : public QWidget {
public:
    graphics_view(const computational_model* model, QWidget* const parent = nullptr);
    void update_content(const computational_output*);
private:
    QtCharts::QChartView * _magn, * _init, * _base, * _next;
};