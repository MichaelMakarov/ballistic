#pragma once
#include <qwidget.h>

class computation;

class plotview : public QWidget {
    Q_OBJECT
public:
    plotview(const computation* const comp, QWidget* const parent = nullptr);
    void clear();
    void replot();
signals:
    void clear_required();
    void replot_required();
private:
    const computation* _comp;
};


