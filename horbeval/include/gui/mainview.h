#pragma once

#include <qmainwindow.h>

class mainmodel;
class computation;
class tle_tablemodel;
class obs_tablemodel;

template<typename T>
struct simple_delete {
    void operator()(T* const ptr) noexcept { delete ptr; }
};

class mainview : public QMainWindow {
    Q_OBJECT
public:
    mainview(QWidget* const parent = nullptr);
    ~mainview();
private:
    void initialize_centralwidget();
    void initialize_resultoutput(class QGridLayout* const grid);
    void initialize_filepathes(class QGridLayout* const grid);
    void initialize_settings(class QGridLayout* const grid);
    void initialize_object_settings(class QHBoxLayout* const grid);
    void initialize_computation(class QHBoxLayout* const grid);
    void initialize_time_settings(class QVBoxLayout* const grid);
    void initialize_button(class QVBoxLayout* const grid);    
    void initialize_tables(class QGridLayout* const grid);
    void initialize_plot(class QHBoxLayout* const grid);

    void show_message(const QString& msg, const QString& style);
    void show_error(const QString& msg);
    void show_info(const QString& msg);

    void showEvent(QShowEvent*) override;
    void closeEvent(QCloseEvent*) override;
signals:
    void settings_loaded();
    void measurements_loaded();
    void computation_performed();
private:
    std::unique_ptr<tle_tablemodel, simple_delete<tle_tablemodel>> _tlemodel;
    std::unique_ptr<obs_tablemodel, simple_delete<obs_tablemodel>> _obsmodel;
    std::unique_ptr<mainmodel, simple_delete<mainmodel>> _model;
    std::unique_ptr<computation, simple_delete<computation>> _comp; 
};










