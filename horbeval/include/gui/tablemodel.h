#pragma once
#include <observation.h>

#include <maths.h>

#include <qabstractitemmodel.h>
#include <qcolor.h>

/**
 * @brief Базовый класс можели данных для таблицы 
 * 
 * @tparam T тип объекта данных
 */
template<typename T>
class tablemodel : public QAbstractTableModel {
public:
    using QAbstractTableModel::QAbstractTableModel;
    /**
     * @brief Обновление данных.
     * 
     * @param beg итератор на начало
     * @param end итератор на конец
     */
    void update_data(iterator_t<T> beg, iterator_t<T> end);
    /**
     * @brief Обновление диапазона по времени
     * 
     * @param tn нач. время
     * @param tk кон. время
     */
    void update_visual(const QDateTime& tn, const QDateTime& tk);
    /**
     * @brief Кол-во строк в таблице
     */
    int rowCount(const QModelIndex&) const override;

    QVariant data(const QModelIndex&, int) const override;
    QVariant headerData(int, Qt::Orientation, int) const;
protected:
    virtual QString data(int row, int col) const = 0;
    virtual int alignment(int col) const = 0;
    virtual void color(int row, int col, QVariant& out) const;
    virtual QString header(int col) const = 0;
protected:
    iterator_t<T> _beg, _end;
    time_h _tn, _tk;
};

/**
 * @brief Модель данных TLE (орбитальных измерений)
 * 
 */
class tle_tablemodel : public tablemodel<orbit_observation> {
public:
    using tablemodel<orbit_observation>::tablemodel;

    int columnCount(const QModelIndex&) const override;
    QString data(int row, int col) const override;
    int alignment(int col) const override;
    QString header(int col) const override;
};

/**
 * @brief Модель данных измерений блеска (вращательных параметров)
 * 
 */
class obs_tablemodel : public tablemodel<rotation_observation> {
public:
    using tablemodel<rotation_observation>::tablemodel;

    int columnCount(const QModelIndex&) const override;
    QString data(int row, int col) const override;
    int alignment(int col) const override;
    QString header(int col) const override;   
};


template<typename T>
inline void tablemodel<T>::update_data(iterator_t<T> beg, iterator_t<T> end)
{
    beginResetModel();
    _beg = beg;
    _end = end;
    endResetModel();
    emit dataChanged({}, {}, {});
}


time_h to_time(const QDateTime& dt);

template<typename T>
inline void tablemodel<T>::update_visual(const QDateTime& tn, const QDateTime& tk)
{
    beginResetModel();
    _tn = to_time(tn);
    _tk = to_time(tk);
    endResetModel();
}

template<typename T>
inline int tablemodel<T>::rowCount(const QModelIndex&) const
{
    return static_cast<int>(std::distance(_beg, _end));
}

template<typename T>
inline QVariant tablemodel<T>::data(const QModelIndex& index, int role) const
{ 
    QVariant out;
    switch (role) {
        case Qt::ItemDataRole::DisplayRole: out = data(index.row(), index.column()); break;
        case Qt::ItemDataRole::TextAlignmentRole: out = alignment(index.column()); break;
        case Qt::ItemDataRole::BackgroundRole: color(index.row(), index.column(), out); break;
    }
    return out;
}

template<typename T>
inline QVariant tablemodel<T>::headerData(int section, Qt::Orientation orientation, int role) const 
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        return header(section);
    } else {
        return QAbstractTableModel::headerData(section, orientation, role);
    }
}

template<typename T>
inline void tablemodel<T>::color(int row, int col, QVariant& out) const
{
    if (col == 0) {
        if (!interval{ _tn, _tk }((_beg + row)->t)) out = QColor(Qt::GlobalColor::red);
    }
}