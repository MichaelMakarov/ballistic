#include <mainview.h>
#include <guisettings.h>
#include <pathview.h>
#include <mainmodel.h>
#include <tablemodel.h>
#include <plotview.h>

#include <qlayout.h>
#include <qtableview.h>
#include <qstatusbar.h>
#include <qtableview.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qdatetimeedit.h>
#include <qlabel.h>
#include <qtableview.h>
#include <qheaderview.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qfiledialog.h>
#include <qscrollarea.h>

#define try_catch(expr) try { expr } \
    catch(const std::exception& ex) { \
        show_error(QString{ "Ошибка! " } + ex.what()); \
    }

/**
 * @brief Минимальное значение даты-времени
 * 
 */
QDateTime min_datetime = QDateTime::fromTime_t(0);
/**
 * @brief Максимальное значение даты-времени
 * 
 */
QDateTime max_datetime = QDateTime::fromTime_t(std::numeric_limits<unsigned int>::max());

const char* datetime_format{ "yyyy.MM.dd HH:mm:ss" };

const char* txtfilter{ "Текстовый файл (*.txt)" };
const char* jsonfilter{ "Json файл (*.json)" };

const char* settings_filename{ "settings.json" };



mainview::mainview(QWidget* const parent) : QMainWindow(parent)
{
    setWindowTitle("Окно уточнения параметров движения и вращения");
    initialize_centralwidget();
}

void mainview::initialize_centralwidget()
{
    auto wgt = new QWidget;
    setCentralWidget(wgt);

    auto grid = new QGridLayout;
    grid->setSpacing(spacing);
    grid->setRowStretch(0, 0);
    grid->setRowStretch(1, 1);
    grid->setRowStretch(2, 1);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);
    grid->setColumnStretch(2, 0);
    wgt->setLayout(grid);

    _model = std::unique_ptr<mainmodel, simple_delete<mainmodel>>(new mainmodel);
    _comp = std::unique_ptr<computation, simple_delete<computation>>(new computation);

    initialize_tables(grid);
    initialize_filepathes(grid);
    initialize_settings(grid);
    initialize_resultoutput(grid);
}

void on_path_changed(pathview* const pv, std::string& text)
{
    QObject::connect(
        pv, &pathview::path_changed, 
        pv, [&text](const QString& path){ text = path.toStdString(); }
    );
}

QPushButton* make_button(const QString& tooltip)
{
    auto btn = new QPushButton;
    btn->setFixedSize(btn_height, btn_height);
    btn->setIcon(visual_provider::apply_icon());
    btn->setToolTip(tooltip);
    return btn;
}

QLabel* make_label(unsigned int number)
{
    auto pixmap = visual_provider::number_icon(number);
    auto lbl = new QLabel;
    lbl->setFixedSize(btn_height, btn_height);
    lbl->setPixmap(pixmap.scaled(lbl->size(), Qt::AspectRatioMode::IgnoreAspectRatio));
    return lbl;
}

void mainview::initialize_filepathes(QGridLayout* const grid)
{
    auto box = new QGroupBox("Пути к файлам");
    grid->addWidget(box, 0, 0);

    auto lay = new QGridLayout;
    lay->setSpacing(spacing);
    lay->setColumnStretch(0, 0);
    lay->setColumnStretch(1, 1);
    lay->setColumnStretch(2, 0);
    box->setLayout(lay);

    auto gptlbl = make_label(1);
    auto tlelbl = make_label(2);
    auto obslbl = make_label(3);
    auto meslbl = make_label(4);
    
    auto gptpath = new pathview(txtfilter, "Файл гармоник ГПЗ:");
    auto tlepath = new pathview(txtfilter, "Файл параметров TLE:");
    auto obspath = new pathview(jsonfilter, "Файл обсерваторий:");
    auto mespath = new pathview(jsonfilter, "Файл измер. блеска:");

	auto gptbtn = make_button("Загрузить гармоники ГПЗ");
	auto tlebtn = make_button("Загрузить данные TLE");
	auto obsbtn = make_button("Загрузить данные измерений");

    lay->addWidget(gptlbl, 0, 0);
    lay->addWidget(tlelbl, 1, 0);
    lay->addWidget(obslbl, 2, 0);
    lay->addWidget(meslbl, 3, 0);
    lay->addWidget(gptpath, 0, 1);
    lay->addWidget(tlepath, 1, 1);
    lay->addWidget(obspath, 2, 1);
    lay->addWidget(mespath, 3, 1);
    lay->addWidget(gptbtn, 0, 2);
    lay->addWidget(tlebtn, 1, 2);
    lay->addWidget(obsbtn, 2, 2, 2, 1);

    box->adjustSize();
    box->setFixedHeight(box->height());

    connect(
        gptbtn, &QAbstractButton::clicked,
        this, [this, gptpath](bool){
            try_catch(
                show_info("Происходит загрузка гармоник потенциала Земли...");
                _model->read_geopotential(gptpath->get_path());
                show_info("Гармоники потенциала Земли загружены из файла.");
            )
        }
    );
    connect(
        tlebtn, &QAbstractButton::clicked,
        this, [this, tlepath](bool){
            try_catch(
                show_info("Происходит загрузка параметров TLE...");
                _model->read_tle(tlepath->get_path());
                auto prov = _model->tle_provider();
                _tlemodel->update_data(prov->begin(), prov->end());
                show_info("TLE загружены из файла.");
                emit measurements_loaded();
            )           
        }
    );
    connect(
        obsbtn, &QAbstractButton::clicked,
        this, [this, obspath, mespath](bool){
            try_catch(
                show_info("Происходит загрузка измерений...");
                _model->read_measurements(mespath->get_path(), obspath->get_path());
                auto prov = _model->obs_provider();
                _obsmodel->update_data(prov->begin(), prov->end());
                show_info("Измерения блеска КА загружены из файла.");
            )            
        }
    );
    connect(
        this, &mainview::settings_loaded,
        this, [gptpath, tlepath, obspath, mespath]() {
            gptpath->set_path(QString::fromStdString(gl_settings.gptpath));
            tlepath->set_path(QString::fromStdString(gl_settings.tlepath));
            obspath->set_path(QString::fromStdString(gl_settings.obspath));
            mespath->set_path(QString::fromStdString(gl_settings.mespath));
        }
    );
    on_path_changed(gptpath, gl_settings.gptpath);
    on_path_changed(tlepath, gl_settings.tlepath);
    on_path_changed(obspath, gl_settings.obspath);
    on_path_changed(mespath, gl_settings.mespath);
}

void mainview::initialize_settings(QGridLayout* const grid)
{
    auto box = new QGroupBox("Настройки");
    grid->addWidget(box, 0, 1);
    
    auto lay = new QHBoxLayout;
    lay->setSpacing(spacing);
    lay->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    box->setLayout(lay);

    initialize_object_settings(lay);
    initialize_computation(lay);

    box->adjustSize();
    box->setFixedSize(box->size());
}

void on_parameter_changed(QDoubleSpinBox* const box, double& value)
{
    QObject::connect(
        box, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        box, [&value](double d){ value = d; }
    );
}

QDoubleSpinBox* make_spinbox() {
    auto spb = new QDoubleSpinBox;
    spb->setFixedSize(btn_width, btn_height);
    spb->setAlignment(Qt::AlignmentFlag::AlignRight);
    return spb;
}

QLabel* make_label(const QString& text) 
{
    auto lbl = new QLabel(text);
    lbl->setFixedHeight(btn_height);
    return lbl;
}

void mainview::initialize_object_settings(QHBoxLayout* const grid)
{
    auto box = new QGroupBox("Параметры модели КА");
    grid->addWidget(box);

    auto lay = new QGridLayout;
    lay->setSpacing(spacing);
    lay->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    box->setLayout(lay);

    auto lbl_mass = make_label("Масса (кг) = ");
    auto lbl_size = make_label("Радиус диска (м) = ");
    auto lbl_refl = make_label("К-т отражения = ");
    
    constexpr auto maxval = std::numeric_limits<double>::max();
    auto spb_mass = make_spinbox();
    spb_mass->setRange(0, maxval);    

    auto spb_size = make_spinbox();
    spb_size->setRange(0, maxval);

    auto spb_refl = make_spinbox();
    spb_refl->setRange(0, 1);
    spb_refl->setSingleStep(0.01);

    lay->addWidget(lbl_mass, 0, 0, Qt::AlignmentFlag::AlignRight);
    lay->addWidget(lbl_size, 1, 0, Qt::AlignmentFlag::AlignRight);
    lay->addWidget(lbl_refl, 2, 0, Qt::AlignmentFlag::AlignRight);
    lay->addWidget(spb_mass, 0, 1, Qt::AlignmentFlag::AlignLeft);
    lay->addWidget(spb_size, 1, 1, Qt::AlignmentFlag::AlignLeft);
    lay->addWidget(spb_refl, 2, 1, Qt::AlignmentFlag::AlignLeft);

    connect(
        this, &mainview::settings_loaded,
        this, [spb_mass, spb_size, spb_refl]() {
            spb_mass->setValue(gl_settings.object.mass);
            spb_size->setValue(gl_settings.object.rad);
            spb_refl->setValue(gl_settings.object.refl);
        }
    );
    on_parameter_changed(spb_mass, gl_settings.object.mass);
    on_parameter_changed(spb_size, gl_settings.object.rad);
    on_parameter_changed(spb_refl, gl_settings.object.refl);
}

void mainview::initialize_computation(QHBoxLayout* const grid)
{
    auto lay = new QVBoxLayout;
    lay->setSpacing(spacing);
    lay->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    grid->addLayout(lay);

    initialize_time_settings(lay);
    initialize_button(lay);
}

void mainview::initialize_time_settings(QVBoxLayout* const grid)
{
    auto box = new QGroupBox("Мерный интервал");
    grid->addWidget(box, 0, Qt::AlignmentFlag::AlignTop);

    auto lay = new QHBoxLayout;
    lay->setSpacing(spacing);
    box->setLayout(lay);

    auto label = new QLabel(" - ");

    auto left = new QDateTimeEdit;
    left->setDisplayFormat(datetime_format);
    left->setFixedHeight(btn_height);
    left->setDateTime(QDateTime::currentDateTime());
    left->setDateTimeRange(min_datetime, left->dateTime());

    auto right = new QDateTimeEdit;
    right->setDisplayFormat(datetime_format);
    right->setFixedHeight(btn_height);
    right->setDateTime(left->dateTime());
    right->setDateTimeRange(right->dateTime(), max_datetime);

    lay->addWidget(left);
    lay->addWidget(label, 0, Qt::AlignmentFlag::AlignCenter);
    lay->addWidget(right);

    box->adjustSize();
    box->setFixedSize(box->size());
    
    connect(
        left, &QDateTimeEdit::dateTimeChanged,
        this, [right, this](const QDateTime& dt) {
            right->setMinimumDateTime(dt);
            _model->set_interval(dt, right->dateTime());
            _tlemodel->update_visual(dt, right->dateTime());
            _obsmodel->update_visual(dt, right->dateTime());
        }
    );
    connect(
        right, &QDateTimeEdit::dateTimeChanged,
        this, [left, this](const QDateTime& dt) {
            left->setMaximumDateTime(dt);
            _model->set_interval(left->dateTime(), dt);
            _tlemodel->update_visual(left->dateTime(), dt);
            _obsmodel->update_visual(left->dateTime(), dt);
        }
    );
    connect(
        this, &mainview::measurements_loaded,
        this, [this, left, right](){
            left->setDateTime(_model->tn());
            right->setDateTime(_model->tk());
        }
    );
}

void mainview::initialize_button(QVBoxLayout* const grid)
{
    auto lay = new QHBoxLayout;
    lay->setSpacing(spacing);
    grid->addLayout(lay);

    auto checkbox = new QCheckBox("Логирование");
    checkbox->setFixedHeight(btn_height);
    checkbox->setChecked(false);

    auto button = new QPushButton("Расcчитать");
    button->setFixedSize(btn_width, btn_height);

    lay->addWidget(checkbox);
    lay->addWidget(button, 0, Qt::AlignmentFlag::AlignLeft);

    connect(
        button, &QAbstractButton::clicked,
        this, [this, checkbox, button](bool) {
            button->setEnabled(false);
            QString filename;
            if (checkbox->isChecked()) {
                filename = QFileDialog::getSaveFileName(
                    this, "Выбор файла логирования", {}, txtfilter
                );
                if (filename.isEmpty()) show_error("Файл логирования не выбран.");
            }
            try_catch(
                _model->compute(_comp.get(), filename);
            )
            button->setEnabled(true);
            emit computation_performed();
        }
    );
}

void mainview::initialize_resultoutput(QGridLayout* const grid)
{
    auto box = new QGroupBox("Результаты расчёта");
    box->setFixedWidth(300);
    grid->addWidget(box, 0, 2, 2, 1);
    auto lay = new QVBoxLayout;
    box->setLayout(lay);
    
    auto lbl_result = new QLabel;

    auto scroll = new QScrollArea;
    scroll->setWidget(lbl_result);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
    scroll->setAlignment(Qt::AlignmentFlag::AlignTop);
    scroll->setWidgetResizable(true);
    lay->addWidget(scroll);

    connect(
        this, &mainview::computation_performed,
        this, [lbl_result, this](){
            lbl_result->setText(QString::fromStdString(_comp->to_string()));
        }
    );
}

QGroupBox* make_table(const QString& title, QAbstractTableModel* const model)
{
    auto box = new QGroupBox(title);
    auto lay = new QHBoxLayout;
    box->setLayout(lay);

    auto table = new QTableView;
    table->setModel(model);
    table->verticalHeader()->setVisible(true);
    table->setMinimumSize(400, 200);
    lay->addWidget(table);

    QObject::connect(
        model, &QAbstractTableModel::dataChanged,
        table, [table, model](const QModelIndex&, const QModelIndex&, const QVector<int>&) {
            table->resizeColumnsToContents();
            table->resizeRowsToContents();
            int minw{};
            for (int i{}; i < model->columnCount(); ++i) {
                minw = std::max(minw, table->columnWidth(i));
            }
            auto header = table->horizontalHeader();
            for (int i{}; i < model->columnCount(); ++i) {
                header->setMinimumSectionSize(minw);
                header->setSectionResizeMode(i, QHeaderView::ResizeMode::Stretch);
            }
        }
    );

    return box;
}

void mainview::initialize_tables(QGridLayout* const grid)
{
    _tlemodel = std::unique_ptr<tle_tablemodel, simple_delete<tle_tablemodel>>(new tle_tablemodel);
    _obsmodel = std::unique_ptr<obs_tablemodel, simple_delete<obs_tablemodel>>(new obs_tablemodel);

    grid->addWidget(make_table("Орбитальные измерения (TLE)", _tlemodel.get()), 1, 0, 1, 2);

    auto lay = new QHBoxLayout;
    lay->setSpacing(spacing);
    grid->addLayout(lay, 2, 0, 1, 3);

    lay->addWidget(make_table("Измерения блеска КА", _obsmodel.get()));

    auto plot = new plotview(_comp.get());
    lay->addWidget(plot);

    connect(
        this, &mainview::computation_performed,
        plot, [plot]() {
            plot->replot();
        }
    );
}

void mainview::show_message(const QString& msg, const QString& style)
{
    auto bar = statusBar();
    bar->setStyleSheet(style);
    bar->showMessage(msg);
}

void mainview::show_error(const QString& msg)
{
    show_message(msg, "color: red");
}

void mainview::show_info(const QString& msg)
{
    show_message(msg, "color: blue");
}

void mainview::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    project_settings read(const QString& filename);
    try {
        gl_settings = read(settings_filename);
        emit settings_loaded();
        show_info("Настройки приложения восстановлены.");
    } catch (const std::exception& error) {
        show_error(QString("Не удалось восстановить настройки приложения. ") + error.what());
    }
}

void mainview::closeEvent(QCloseEvent* event)
{
    QMainWindow::closeEvent(event);
    void write(const QString& filename, const project_settings& p);
    try {
        write(settings_filename, gl_settings);
    } catch (const std::exception&) {}
}

mainview::~mainview()
{
    _tlemodel = nullptr;
    _obsmodel = nullptr;
}