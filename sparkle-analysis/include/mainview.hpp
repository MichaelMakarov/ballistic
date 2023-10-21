#pragma once

#include <qmainwindow.h>

#include <async_task.hpp>

class application_window : public QMainWindow {
  public:
    application_window();
    ~application_window();

  private:
    void on_load_tle_clicked(bool = true);
    void on_load_gpt_clicked(bool = true);
    void on_load_measurements_clicked(bool = true);
    async_task<void> on_compute_clicked();

    void on_tle_index_changed(int);
    void on_interval_changed(double);

    void load_settings();
    void save_settings();

    void show_message(const QString &msg, const QString &style);
    void show_error(const QString &msg);
    void show_info(const QString &msg);

    void showEvent(QShowEvent *) override;
    void closeEvent(QCloseEvent *) override;
    bool event(QEvent *) override;

  private:
    class computational_model *_model;
    class filepath_view *_gpt_filepath_view;
    class filepath_view *_tle_filepath_view;
    class filepath_view *_obs_filepath_view;
    class filepath_view *_mes_filepath_view;
    class table_view *_table;
    class tree_view *_tree;
    class QDoubleSpinBox *_mass, *_square, *_refl;
    class QDoubleSpinBox *_interval_spinbox;
    class QSpinBox *_tle_index_spinbox;
    class QPushButton *_load_gpt_button;
    class QPushButton *_load_tle_button;
    class QPushButton *_load_mes_button;
    class QPushButton *_compute_button;
    class QCheckBox *_log_checkbox;
};
