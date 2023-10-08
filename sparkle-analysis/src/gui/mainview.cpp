#include <mainview.hpp>
#include <gui.hpp>
#include <settings.hpp>
#include <pathview.hpp>
#include <mainmodel.hpp>
#include <datamodel.hpp>
#include <graphic_window.hpp>

#include <qlayout.h>
#include <qstatusbar.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qfiledialog.h>

const char *txtfilter{"Текстовый файл (*.txt)"};
const char *jsonfilter{"Json файл (*.json)"};

constexpr QSize min_size{400, 200};

mainview::mainview(QWidget *const parent) : QMainWindow(parent)
{
    setWindowTitle("Окно уточнения параметров движения");
    // класс для расчётов
    _model = new computational_model(this);

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
    path_layout->addWidget(_gpt = make_pathview(txtfilter, "Файл гармоник ГПЗ:", {}, [](const QString &path)
                                                { settings.gptpath = path.toStdString(); }),
                           0, 1);
    path_layout->addWidget(_tle = make_pathview(txtfilter, "Файл данных TLE:", {}, [](const QString &path)
                                                { settings.tlepath = path.toStdString(); }),
                           1, 1);
    path_layout->addWidget(_obs = make_pathview(jsonfilter, "Файл обсерваторий:", {}, [](const QString &path)
                                                { settings.obspath = path.toStdString(); }),
                           2, 1);
    path_layout->addWidget(_mes = make_pathview(jsonfilter, "Файл измер. блеска:", {}, [](const QString &path)
                                                { settings.mespath = path.toStdString(); }),
                           3, 1);
    path_layout->addWidget(make_apply_button("Загрузить гармоники ГПЗ", [this](bool)
                                             { on_load_gpt_clicked(); }),
                           0, 2);
    path_layout->addWidget(make_apply_button("Загрузить данные TLE", [this](bool)
                                             { on_load_tle_clicked(); }),
                           1, 2);
    path_layout->addWidget(make_apply_button("Загрузить данные измерений", [this](bool)
                                             { on_load_measurements_clicked(); }),
                           2, 2, 2, 1);
    // компоновщик для настроек
    auto set_layout = make_layout<QHBoxLayout>();
    layout->addWidget(make_groupbox("Настройки", set_layout), 0);
    // кмпоновщик для настроек КА
    auto obj_layout = make_layout<QGridLayout>();
    set_layout->addWidget(make_groupbox("Параметры модели КА", obj_layout));
    obj_layout->addWidget(make_label("Масса (кг) = "), 0, 0, Qt::AlignmentFlag::AlignRight);
    obj_layout->addWidget(make_label("S пов-ти (м^2) = "), 1, 0, Qt::AlignmentFlag::AlignRight);
    obj_layout->addWidget(make_label("К-т отраж. = "), 2, 0, Qt::AlignmentFlag::AlignRight);
    obj_layout->addWidget(_mass = make_double_spinbox(settings.object.mass, 1, 1e10, 1, [](double val)
                                                      { settings.object.mass = val; }),
                          0, 1);
    obj_layout->addWidget(_square = make_double_spinbox(settings.object.square, 0, 1e3, 1, [](double val)
                                                        { settings.object.square = val; }),
                          1, 1);
    obj_layout->addWidget(_refl = make_double_spinbox(settings.object.refl, 0, 1, 1e-1, [](double val)
                                                      { settings.object.refl = val; }),
                          2, 1);
    // компоновщик для настроек вычислений
    auto comp_layout = make_layout<QGridLayout>();
    set_layout->addWidget(make_groupbox("Параметры расчёта", comp_layout));
    comp_layout->addWidget(make_label("Выбранный ТЛЕ"), 0, 0, Qt::AlignmentFlag::AlignRight);
    comp_layout->addWidget(make_label("Мерный интервал, сут"), 1, 0, Qt::AlignmentFlag::AlignRight);
    comp_layout->addWidget(make_checkbox("Логирование", _logreq), 2, 0, Qt::AlignmentFlag::AlignRight);
    comp_layout->addWidget(_index = make_spinbox(0, 0, 0, 1, [this](int val)
                                                 { _model->select_tle(val); }),
                           0, 1);
    comp_layout->addWidget(_inter = make_double_spinbox(1, 0, 1e10, 1, [this](double val)
                                                        { _model->select_interval(val); }),
                           1, 1);
    comp_layout->addWidget(_comp = make_button("Рассчитать", [this](bool)
                                               { on_compute_clicked(); }),
                           2, 1, Qt::AlignmentFlag::AlignLeft);
    // таблица
    auto table = new tle_tableview(_model, this);
    table->setMinimumSize(min_size);
    layout = make_layout<QHBoxLayout>();
    layout->addWidget(table);
    grid->addWidget(make_groupbox("Данные ТЛЕ", layout), 1, 0);
    // дерево
    auto tree = new measurement_treeview(_model, this);
    tree->setMinimumSize(min_size);
    layout = make_layout<QHBoxLayout>();
    layout->addWidget(tree);
    grid->addWidget(make_groupbox("Измерения блеска", layout), 1, 1);

    connect(_model, &computational_model::tle_data_loaded, this, &mainview::on_tle_loaded);
    connect(this, &mainview::settings_loaded, this, &mainview::on_settings_loaded);
}

void mainview::on_load_gpt_clicked() const
{
    try
    {
        show_info("Происходит загрузка гармоник потенциала Земли...");
        _model->read_gpt();
        show_info("Гармоники потенциала Земли загружены из файла.");
    }
    catch (const std::exception &error)
    {
        show_error(error.what());
    }
}

void mainview::on_load_tle_clicked() const
{
    try
    {
        show_info("Происходит загрузка данных TLE...");
        _model->read_tle();
        show_info("TLE загружены из файла.");
    }
    catch (const std::exception &error)
    {
        show_error(error.what());
    }
}

void mainview::on_load_measurements_clicked() const
{
    try
    {
        show_info("Происходит загрузка измерений блеска...");
        _model->read_measurements();
        show_info("Измерения блеска КА загружены из файла.");
    }
    catch (const std::exception &error)
    {
        show_error(error.what());
    }
}

auto make_graphic_window(optimization_logger const *logger)
{
    auto constexpr aname{"|a|, град"};
    auto constexpr iname{"|i|, град"};
    auto wnd = new graphic_window;
    figure_info info;
    info.xaxis.name = "Невязки";
    auto residuals = logger->get_first_iteration_residuals();
    std::vector<double> xarr(residuals.size());
    std::vector<double> yarr(residuals.size());
    info.points.count = residuals.size();
    info.points.xarr = xarr.data();
    info.points.yarr = yarr.data();

    info.title = "Невязки по склонению на первой итерации";
    info.yaxis.name = iname;
    for (std::size_t i{}; i < residuals.size(); ++i)
    {
        xarr[i] = i;
        yarr[i] = math::rad_to_deg(residuals[i].i);
    }
    wnd->add_figure(0, 0, info);

    info.title = "Невязки по восхождению на первой итерации";
    info.yaxis.name = aname;
    for (std::size_t i{}; i < residuals.size(); ++i)
    {
        yarr[i] = math::rad_to_deg(residuals[i].a);
    }
    wnd->add_figure(0, 1, info);

    info.title = "Невязки по восхождению на последней итерации";
    residuals = logger->get_last_iteration_residuals();
    for (std::size_t i{}; i < residuals.size(); ++i)
    {
        yarr[i] = math::rad_to_deg(residuals[i].a);
    }
    wnd->add_figure(1, 1, info);

    info.title = "Невязки по склонению на последней итерации";
    info.yaxis.name = iname;
    for (std::size_t i{}; i < residuals.size(); ++i)
    {
        yarr[i] = math::rad_to_deg(residuals[i].i);
    }
    wnd->add_figure(1, 0, info);

    return wnd;
}

void mainview::on_compute_clicked()
{
    auto button = _comp;
    button->setEnabled(false);
    QString filename;
    try
    {
        if (_logreq)
        {
            filename = QFileDialog::getSaveFileName(this, "Выбор файла логирования", {}, "(*.log)");
            if (filename.isEmpty())
            {
                throw std::runtime_error("Файл логирования не выбран.");
            }
        }
        _model->compute(filename.toStdString());
        show_info("Расчёт завершён. Промежуточные вычисления записаны в " + filename);
        auto wnd = make_graphic_window(_model->get_logger());
        wnd->setParent(this);
        wnd->setWindowFlag(Qt::WindowType::Window, true);
        wnd->setWindowTitle("Окно отображения невязок");
        wnd->showMaximized();
    }
    catch (const std::exception &error)
    {
        show_error(error.what() + ("Промежуточные вычисления записаны в " + filename));
    }
    button->setEnabled(true);
}

void mainview::on_tle_loaded() const
{
    _index->setRange(1, _model->tle_count());
    _index->setEnabled(_model->tle_count() > 0);
}

void mainview::on_settings_loaded() const
{
    _gpt->set_path(settings.gptpath.c_str());
    _tle->set_path(settings.tlepath.c_str());
    _obs->set_path(settings.obspath.c_str());
    _mes->set_path(settings.mespath.c_str());
    _mass->setValue(settings.object.mass);
    _square->setValue(settings.object.square);
    _refl->setValue(settings.object.refl);
    on_load_gpt_clicked();
    on_load_tle_clicked();
    on_load_measurements_clicked();
}

void mainview::show_message(const QString &msg, const QString &style) const
{
    auto bar = statusBar();
    bar->setStyleSheet(style);
    bar->showMessage(msg);
}

void mainview::show_error(const QString &msg) const
{
    show_message(msg, "color: red");
}

void mainview::show_info(const QString &msg) const
{
    show_message(msg, "color: blue");
}

project_settings read_settings();
void write_settings(project_settings const &);

void mainview::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    try
    {
        settings = read_settings();
        emit settings_loaded();
        show_info("Настройки приложения восстановлены.");
    }
    catch (const std::exception &error)
    {
        show_error(QString("Не удалось восстановить настройки приложения. ") + error.what());
    }
}

void mainview::closeEvent(QCloseEvent *event)
{
    try
    {
        write_settings(settings);
    }
    catch (const std::exception &)
    {
    }
    QMainWindow::closeEvent(event);
}
