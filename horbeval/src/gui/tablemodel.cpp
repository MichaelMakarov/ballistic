#include <tablemodel.h>
#include <guisettings.h>

#include <qdatetime.h>

#include <assertion.h>

time_h to_time(const QDateTime& dt)
{
    calendar c;
    c.year = dt.date().year();
    c.month = dt.date().month();
    c.day = dt.date().day();
    c.hour = dt.time().hour();
    c.minute = dt.time().minute();
    c.second = dt.time().second();
    c.millisecond = dt.time().msec();
    return make_time(c);
}

int tle_tablemodel::columnCount(const QModelIndex&) const 
{
    return 7;
}

QString number_to_str(double num)
{
    return QString::number(num, 'f', ordinary_prec);
}

QString tle_tablemodel::data(int row, int col) const
{
    auto& obs = *(_beg + row);
    switch (col) {
        case 0: return QString::fromStdString(format("%", obs.t));
        case 1: return number_to_str(obs.v[0]);
        case 2: return number_to_str(obs.v[1]);
        case 3: return number_to_str(obs.v[2]);
        case 4: return number_to_str(obs.v[3]);
        case 5: return number_to_str(obs.v[4]);
        case 6: return number_to_str(obs.v[5]);
        default: return {};
    }
}

int tle_tablemodel::alignment(int col) const
{
    int alignment = Qt::AlignmentFlag::AlignVCenter;
    switch (col) {
        case 0: 
            alignment += Qt::AlignmentFlag::AlignLeft;
            break;
        case 1: case 2: case 3: case 4: case 5: case 6:
            alignment += Qt::AlignmentFlag::AlignRight;
            break;
    }
    return alignment;
}

QString tle_tablemodel::header(int col) const
{
    switch (col) {
        case 0: return "T";
        case 1: return "X, м";
        case 2: return "Y, м";
        case 3: return "Z, м";
        case 4: return "Vx, м/с";
        case 5: return "Vy, м/с";
        case 6: return "Vz, м/с";
        default: return {};
    }
}

int obs_tablemodel::columnCount(const QModelIndex&) const 
{
    return 6;
}

QString obs_tablemodel::data(int row, int col) const
{
    auto& obs = *(_beg + row);
    switch (col) {
        case 0: return QString::fromStdString(obs.id);
        case 1: return QString::fromStdString(format("%", obs.t));
        case 2: return number_to_str(obs.o[0]);
        case 3: return number_to_str(obs.o[1]);
        case 4: return number_to_str(obs.o[2]);
        case 5: return number_to_str(obs.s);
        default: return {};
    }
}

int obs_tablemodel::alignment(int col) const
{
    int alignment = Qt::AlignmentFlag::AlignVCenter;
    switch (col) {
        case 0: case 1:
            alignment += Qt::AlignmentFlag::AlignLeft;
            break;
        case 2: case 3: case 4: case 5:
            alignment += Qt::AlignmentFlag::AlignRight;
            break;
    }
    return alignment;
}

QString obs_tablemodel::header(int col) const
{
    switch (col) {
        case 0: return "ID";
        case 1: return "T";
        case 2: return "X, м";
        case 3: return "Y, м";
        case 4: return "Z, м";
        case 5: return "Зв. величина";
        default: return {};
    }
}

