#pragma once
#include <qtableview.h>
#include <qtreeview.h>

class computational_model;

/**
 * @brief Таблица для данных ТЛЕ
 * 
 */
class tle_tableview : public QTableView {
public:
    tle_tableview(const computational_model* model, QWidget* parent = nullptr);
private:
    void resize_to_contents();
};
/**
 * @brief Дерево для данных измерений
 * 
 */
class measurement_treeview : public QTreeView {
public:
    measurement_treeview(const computational_model* model, QWidget* parent = nullptr);
private:
    void resize_to_contents();
};
