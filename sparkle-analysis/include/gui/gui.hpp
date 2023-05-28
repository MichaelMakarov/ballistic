#pragma once
#include <functional>
#include <qicon.h>
#include <qpixmap.h>


class QLayout;
class QGroupBox;
class QPushButton;
class QLabel;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;

using button_callback = std::function<void(bool)>;


/**
 * @brief Создание QGroupBox
 * 
 * @param title 
 * @param layout 
 * @return QGroupBox* 
 */
QGroupBox* make_groupbox(const QString& title, QLayout* layout);
/**
 * @brief Создание кнопки.
 * 
 * @param text 
 * @param callback 
 * @return QPushButton* 
 */
QPushButton* make_button(const QString& text, const button_callback& callback = {});
/**
 * @brief Создание кнопки с иконкой применить
 * 
 * @param text  
 * @param callback 
 * @return QPushButton* 
 */
QPushButton* make_apply_button(const QString& tooltip, const button_callback& callback = {});
/**
 * @brief Создание кнопки с иконкой файл
 * 
 * @param tooltip 
 * @param callback 
 * @return QPushButton* 
 */
QPushButton* make_file_button(const QString& tooltip, const button_callback& callback = {});
/**
 * @brief Создание надписи.
 * 
 * @param text 
 * @return QLabel* 
 */
QLabel* make_label(const QString& text);
/**
 * @brief Создание надписи с чиселком.
 * 
 * @param number 
 * @return QLabel* 
 */
QLabel* make_label(unsigned number);
/**
 * @brief Создание QSpinBox.
 * 
 * @param value 
 * @param minv 
 * @param maxv 
 * @param step 
 * @return QSpinBox* 
 */
QSpinBox* make_spinbox(int value, int minv, int maxv, int step);
/**
 * @brief Создание QSpinBox
 * 
 * @param value 
 * @param minv 
 * @param maxv 
 * @param step 
 * @param callback 
 * @return QSpinBox* 
 */
QSpinBox* make_spinbox(int value, int minv, int maxv, int step, const std::function<void(int)>& callback);
/**
 * @brief Создание QDoubleSpinBox;
 * 
 * @param value 
 * @param minv 
 * @param maxv 
 * @param step 
 * @return QDoubleSpinBox* 
 */
QDoubleSpinBox* make_double_spinbox(double value, double minv, double maxv, double step);
/**
 * @brief Создание QDoubleSpinBox;
 * 
 * 
 * @param value 
 * @param minv 
 * @param maxv 
 * @param step 
 * @param callback 
 * @return QDoubleSpinBox* 
 */
QDoubleSpinBox* make_double_spinbox(double value, double minv, double maxv, double step, const std::function<void(double)>& callback);
/**
 * @brief Создание виджета с путём к файлу.
 * 
 * @param filter 
 * @param title 
 * @param path 
 * @return filepath_view* 
 */
class filepath_view* make_pathview(const QString& filter, const QString& title, const QString& path, const std::function<void(const QString&)>& callback);
/**
 * @brief Создание чекбокса.
 * 
 * @param text 
 * @param check 
 * @return QCheckBox* 
 */
QCheckBox* make_checkbox(const QString& text, bool& check);
/**
 * @brief Создание компоновщика.
 * 
 * @tparam T тип компоновщика
 * @return auto 
 */
template<typename T>
auto make_layout() {
    auto layout = new T;
    layout->setSpacing(5);
    return layout;
}
/**
 * @brief Создание таблицы.
 * 
 * @param title 
 * @param model 
 * @param minsize 
 * @return QGroupBox* 
 */
QGroupBox* make_table(const QString& title, class QAbstractTableModel* model, const QSize minsize);
/**
 * @brief Создание дерева.
 * 
 */
QGroupBox* make_tree(const QString& title, class QAbstractItemModel* const model, const QSize& minsize);
/**
 * @brief Создание редактируемого текста.
 * 
 * @return QLineEdit* 
 */
class QLineEdit* make_lineedit();
/**
 * @brief Создание виджета с компоновщиком.
 * 
 * @param layout 
 * @return QWidget* 
 */
class QWidget* make_widget(QLayout* layout);