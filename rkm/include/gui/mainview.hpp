#pragma once
#include <qmainwindow.h>

class mainview : public QMainWindow {
    Q_OBJECT
public:
    explicit mainview(QWidget* const parent = nullptr);
private:
    void on_load_tle_clicked() const;
    void on_load_gpt_clicked() const;
    void on_load_measurements_clicked() const;
    void on_compute_clicked();

    void on_tle_loaded() const;
    void on_settings_loaded() const;    

    void show_message(const QString& msg, const QString& style) const;
    void show_error(const QString& msg) const;
    void show_info(const QString& msg) const;

    void showEvent(QShowEvent*) override;
    void closeEvent(QCloseEvent*) override;
signals:
    void settings_loaded() const;
private:
    class computational_model* _model; 
    class filepath_view *_gpt, *_tle, *_obs, *_mes;
    class QDoubleSpinBox *_mass, *_square, *_refl, *_inter;
    class QSpinBox *_index;
    class QPushButton* _comp;
    bool _logreq{ false };
};


