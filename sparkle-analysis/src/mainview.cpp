#include <graphic_window.hpp>
#include <gui.hpp>
#include <mainmodel.hpp>
#include <mainview.hpp>
#include <pathview.hpp>
#include <residual_graphic.hpp>
#include <settings.hpp>
#include <table_view.hpp>
#include <tree_view.hpp>

#include <qapplication.h>
#include <qcheckbox.h>
#include <qfiledialog.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qstatusbar.h>

#include <future>

namespace {
    class coroutine_event : public QEvent {
      public:
        static constexpr QEvent::Type coroutine_event_type{static_cast<QEvent::Type>(1001)};
        using coroutine_type = std::coroutine_handle<coroutine_promise<void>>;

      public:
        coroutine_event(coroutine_type handle)
            : QEvent(coroutine_event_type)
            , _handle{handle} {
        }

        coroutine_type get_coroutine_handle() const {
            return _handle;
        }

      private:
        coroutine_type _handle;
    };

    const char *txtfilter{"Текстовый файл (*.txt)"};
    const char *jsonfilter{"Json файл (*.json)"};

    constexpr auto settings_filename{"settings.conf"};

    constexpr QSize min_size{400, 200};
} // namespace

application_window::application_window()
    : _model{new computational_model} {
    setWindowTitle("Окно уточнения параметров движения");
    auto grid = make_layout<QGridLayout>();
    setCentralWidget(make_widget(grid));
    // 3 строки и 3 столбца
    grid->setRowStretch(0, 0);
    grid->setRowStretch(1, 1);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);

    // компоновщик для всех настроек
    auto layout = make_layout<QHBoxLayout>();
    layout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    grid->addLayout(layout, 0, 0, 1, 2);
    // компоновщик для путей к файлам
    auto path_layout = make_layout<QGridLayout>();
    layout->addWidget(make_groupbox("Файлы", path_layout), 1);
    path_layout->setColumnStretch(0, 0);
    path_layout->setColumnStretch(1, 1);
    path_layout->setColumnStretch(2, 0);
    // добавлем виджеты
    path_layout->addWidget(make_label(1), 0, 0);
    path_layout->addWidget(make_label(2), 1, 0);
    path_layout->addWidget(make_label(3), 2, 0);
    path_layout->addWidget(make_label(4), 3, 0);
    path_layout->addWidget(_gpt_filepath_view = new filepath_view(txtfilter, "Файл гармоник ГПЗ:"), 0, 1);
    path_layout->addWidget(_tle_filepath_view = new filepath_view(txtfilter, "Файл данных TLE:"), 1, 1);
    path_layout->addWidget(_obs_filepath_view = new filepath_view(jsonfilter, "Файл обсерваторий:"), 2, 1);
    path_layout->addWidget(_mes_filepath_view = new filepath_view(jsonfilter, "Файл измер. блеска:"), 3, 1);
    path_layout->addWidget(_load_gpt_button = make_apply_button("Загрузить гармоники ГПЗ"), 0, 2);
    path_layout->addWidget(_load_tle_button = make_apply_button("Загрузить данные TLE"), 1, 2);
    path_layout->addWidget(_load_mes_button = make_apply_button("Загрузить данные измерений"), 2, 2, 2, 1);
    // компоновщик для настроек
    auto set_layout = make_layout<QHBoxLayout>();
    layout->addWidget(make_groupbox("Настройки", set_layout), 0);
    // компоновщик для настроек вычислений
    auto comp_layout = make_layout<QGridLayout>();
    set_layout->addWidget(make_groupbox("Параметры расчёта", comp_layout));
    comp_layout->addWidget(make_label("Выбранный ТЛЕ"), 0, 0, Qt::AlignmentFlag::AlignRight);
    comp_layout->addWidget(make_label("Мерный интервал, сут"), 1, 0, Qt::AlignmentFlag::AlignRight);
    comp_layout->addWidget(_log_checkbox = make_checkbox("Логирование"), 2, 0, Qt::AlignmentFlag::AlignRight);
    comp_layout->addWidget(_tle_index_spinbox = make_spinbox(0, 0, 0, 1), 0, 1);
    comp_layout->addWidget(_interval_spinbox = make_double_spinbox(1, 0, 1e10, 1), 1, 1);
    comp_layout->addWidget(_compute_button = make_button("Рассчитать"), 2, 1, Qt::AlignmentFlag::AlignLeft);
    // таблица
    _table = new table_view(_model->get_table_data_provider(), this);
    _table->setMinimumSize(min_size);
    layout = make_layout<QHBoxLayout>();
    layout->addWidget(_table);
    grid->addWidget(make_groupbox("Данные ТЛЕ", layout), 1, 0);
    // дерево
    _tree = new tree_view(_model->get_tree_data_provider(), this);
    _tree->setMinimumSize(min_size);
    layout = make_layout<QHBoxLayout>();
    layout->addWidget(_tree);
    grid->addWidget(make_groupbox("Измерения блеска", layout), 1, 1);

    connect(_load_gpt_button, &QPushButton::clicked, this, &application_window::on_load_gpt_clicked);
    connect(_load_tle_button, &QPushButton::clicked, this, &application_window::on_load_tle_clicked);
    connect(_load_mes_button, &QPushButton::clicked, this, &application_window::on_load_measurements_clicked);
    connect(_tle_index_spinbox, QOverload<int>::of(&QSpinBox::valueChanged), this, &application_window::on_tle_index_changed);
    connect(_interval_spinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &application_window::on_interval_changed);
    connect(_compute_button, &QPushButton::clicked, this, [this](bool) { on_compute_clicked(); });
}

application_window::~application_window() {
    delete _model;
}

void application_window::on_load_gpt_clicked(bool) {
    try {
        show_info("Происходит загрузка гармоник потенциала Земли...");
        _model->read_gpt(_gpt_filepath_view->get_path().toStdString());
        show_info("Гармоники потенциала Земли загружены из файла.");
    } catch (const std::exception &error) {
        show_error(error.what());
    }
}

void application_window::on_load_tle_clicked(bool) {
    try {
        show_info("Происходит загрузка данных TLE...");
        _model->read_tle(_tle_filepath_view->get_path().toStdString());
        show_info("TLE загружены из файла.");
        _tle_index_spinbox->setRange(1, static_cast<int>(_model->get_tle_count()));
        _tle_index_spinbox->setEnabled(_model->get_tle_count() > 0);
        _table->update_content();
    } catch (const std::exception &error) {
        show_error(error.what());
    }
}

void application_window::on_load_measurements_clicked(bool) {
    try {
        show_info("Происходит загрузка измерений блеска...");
        _model->read_measurements(_obs_filepath_view->get_path().toStdString(), _mes_filepath_view->get_path().toStdString());
        show_info("Измерения блеска КА загружены из файла.");
        _tree->update_content();
    } catch (const std::exception &error) {
        show_error(error.what());
    }
}

async_task<void> application_window::on_compute_clicked() {
    _compute_button->setEnabled(false);
    QString filename;
    try {
        if (_log_checkbox->isChecked()) {
            filename = QFileDialog::getSaveFileName(this, "Выбор файла логирования", {}, "(*.log)");
            if (filename.isEmpty()) {
                throw std::runtime_error("Файл логирования не выбран.");
            }
        }
        coroutine_awaiter<void> awaiter;
        auto fut = std::async(std::launch::async, [this, filename, &awaiter] {
            _model->compute(filename.toStdString());
            QApplication::postEvent(this, new coroutine_event(awaiter._handle));
        });
        co_await awaiter;
        fut.wait();
        show_info("Расчёт завершён. Промежуточные вычисления записаны в " + filename);
        auto wnd = make_residuals_window(*_model->get_residuals_provider());
        wnd->setParent(this);
        wnd->setWindowFlag(Qt::WindowType::Window, true);
        wnd->setWindowTitle("Окно отображения невязок");
        wnd->showMaximized();
    } catch (const std::exception &error) {
        show_error(error.what() + ("Промежуточные вычисления записаны в " + filename));
    }
    _compute_button->setEnabled(true);
}

void application_window::on_tle_index_changed(int index) {
    _model->select_tle(static_cast<std::size_t>(index));
}

void application_window::on_interval_changed(double days) {
    _model->select_interval(days);
}

void application_window::load_settings() {
    try {
        auto settings = load_project_settings_from_json(settings_filename);
        _gpt_filepath_view->set_path(settings.gptpath.c_str());
        _tle_filepath_view->set_path(settings.tlepath.c_str());
        _obs_filepath_view->set_path(settings.obspath.c_str());
        _mes_filepath_view->set_path(settings.mespath.c_str());
        on_load_gpt_clicked();
        on_load_tle_clicked();
        on_load_measurements_clicked();
        show_info("Настройки приложения восстановлены.");
    } catch (const std::exception &ex) {
        show_error(QString("Не удалось восстановить настройки приложения. ") + ex.what());
    }
}

void application_window::save_settings() {
    try {
        project_settings settings;
        settings.gptpath = _gpt_filepath_view->get_path().toStdString();
        settings.tlepath = _tle_filepath_view->get_path().toStdString();
        settings.obspath = _obs_filepath_view->get_path().toStdString();
        settings.mespath = _mes_filepath_view->get_path().toStdString();
        save_project_settings_to_json(settings_filename, settings);
    } catch (std::exception const &) {
    }
}

void application_window::show_message(const QString &msg, const QString &style) {
    auto bar = statusBar();
    bar->setStyleSheet(style);
    bar->showMessage(msg);
}

void application_window::show_error(const QString &msg) {
    show_message(msg, "color: red");
}

void application_window::show_info(const QString &msg) {
    show_message(msg, "color: blue");
}

void application_window::showEvent(QShowEvent *event) {
    load_settings();
    QMainWindow::showEvent(event);
}

void application_window::closeEvent(QCloseEvent *event) {
    save_settings();
    QMainWindow::closeEvent(event);
}

bool application_window::event(QEvent *event) {
    switch (event->type()) {
    case coroutine_event::coroutine_event_type: {
        auto ptr = dynamic_cast<coroutine_event *>(event);
        auto handle = ptr->get_coroutine_handle();
        handle();
        handle.destroy();
    }
    default:
        return QMainWindow::event(event);
    }
    return false;
}
