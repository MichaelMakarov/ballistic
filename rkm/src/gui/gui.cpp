#include <gui.hpp>
#include <pathview.hpp>
#include <stdexcept>
#include <qresource.h>
#include <qcommonstyle.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qtableview.h>
#include <qtreeview.h>
#include <qheaderview.h>
#include <qlineedit.h>

constexpr int width{ 100 };
constexpr int height{ 30 };

/**
 * @brief Директория с ресурсами
 * 
 */
static QString directory{ ":/src/icons/" };

static QCommonStyle style;

QIcon file_icon()
{
    return style.standardIcon(QStyle::SP_DirIcon);
}

QIcon apply_icon()
{
	return QIcon(directory + "check.png");
}

QPixmap number_icon(unsigned number)
{
	QString name;
	switch (number) {
		case 1: name = "one.png"; break;
		case 2: name = "two.png"; break;
		case 3: name = "three.png"; break;
		case 4: name = "four.png"; break;
		case 5: name = "five.png"; break;
		default: throw std::invalid_argument("Иконка с таким числом отсутствует.");
	}
	return QPixmap(directory + name);
}

QGroupBox* make_groupbox(const QString& title, QLayout* layout)
{
	auto box = new QGroupBox(title);
	box->setLayout(layout);
	return box;
}

void set_callback(QPushButton* btn, const button_callback& callback)
{
	if (callback) {
		QObject::connect(btn, &QAbstractButton::clicked, btn, callback);
	}
}

QPushButton* make_button(const QString& text, const button_callback& callback)
{
	auto btn = new QPushButton(text);
	btn->setFixedSize(width, height);
	set_callback(btn, callback);
	return btn;
}

QPushButton* make_button(const QIcon& icon, const QString& tooltip, const button_callback& callback)
{
	auto btn = new QPushButton;
	btn->setFixedSize(height, height);
	btn->setIcon(icon);
	btn->setToolTip(tooltip);
	set_callback(btn, callback);
	return btn;
}

QPushButton* make_apply_button(const QString& tooltip, const button_callback& callback)
{
	return make_button(apply_icon(), tooltip, callback);
}

QPushButton* make_file_button(const QString& tooltip, const button_callback& callback)
{
	return make_button(file_icon(), tooltip, callback);
}

QLabel* make_label(const QString& text)
{
	auto lbl = new QLabel(text);
	lbl->setFixedHeight(height);
	return lbl;
}

QLabel* make_label(unsigned number)
{
	auto pixmap = number_icon(number);
	auto lbl = new QLabel;
	lbl->setFixedSize(height, height);
	lbl->setPixmap(pixmap.scaled(lbl->size(), Qt::AspectRatioMode::IgnoreAspectRatio));
	return lbl;
}

template<typename T, typename V>
T* make_spinbox(V value, V minv, V maxv, V step)
{
	auto spb = new T;
	spb->setFixedSize(width, height);
	spb->setValue(value);
	spb->setSingleStep(step);
	spb->setRange(minv, maxv);
	spb->setAlignment(Qt::AlignmentFlag::AlignRight);
	return spb;
}

QSpinBox* make_spinbox(int value, int minv, int maxv, int step)
{
	return make_spinbox<QSpinBox>(value, minv, maxv, step);
}

QSpinBox* make_spinbox(int value, int minv, int maxv, int step, const std::function<void(int)>& callback)
{
	auto spb = make_spinbox<QSpinBox>(value, minv, maxv, step);
	if (callback) {
		QObject::connect(spb, QOverload<int>::of(&QSpinBox::valueChanged), spb, callback);
	}
	return spb;
}

QDoubleSpinBox* make_double_spinbox(double value, double minv, double maxv, double step)
{
	return make_spinbox<QDoubleSpinBox>(value, minv, maxv, step);
}

QDoubleSpinBox* make_double_spinbox(double value, double minv, double maxv, double step, const std::function<void(double)>& callback)
{
	auto spb = make_spinbox<QDoubleSpinBox>(value, minv, maxv, step);
	if (callback) {
		QObject::connect(spb, QOverload<double>::of(&QDoubleSpinBox::valueChanged), spb, callback);
	}
	return spb;
}

filepath_view* make_pathview(const QString& filter, const QString& title, const QString& path, const std::function<void(const QString&)>& callback)
{
	auto view = new filepath_view(filter, title);
	view->set_path(path);
	if (callback) QObject::connect(view, &filepath_view::path_changed, view, callback);
	return view;
}

QCheckBox* make_checkbox(const QString& text, bool& check)
{
	auto chb = new QCheckBox(text);
	chb->setFixedHeight(height);
	chb->setChecked(check);
	QObject::connect(chb, &QCheckBox::stateChanged, chb, [&check](int v){ check = v == Qt::CheckState::Checked; });
	return chb;
}


QGroupBox* make_table(const QString& title, class QAbstractTableModel* model, const QSize minsize)
{
	auto layout = make_layout<QHBoxLayout>();
    auto box = make_groupbox(title, layout);
    auto table = new QTableView;
    table->setModel(model);
    table->verticalHeader()->setVisible(true);
    table->setMinimumSize(minsize);
    table->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    table->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    layout->addWidget(table);

    QObject::connect(
        model, &QAbstractTableModel::dataChanged,
        table, [table, model](const QModelIndex&, const QModelIndex&, const QVector<int>&) {
            table->resizeColumnsToContents();
            table->resizeRowsToContents();
        }
    );

    return box;
}

QGroupBox* make_tree(const QString& title, QAbstractItemModel* const model, const QSize& minsize)
{
    auto box = new QGroupBox(title);
    auto lay = new QHBoxLayout;
    box->setLayout(lay);

    auto tree = new QTreeView;
    tree->setModel(model);
    tree->setMinimumSize(minsize);
    tree->header()->setStretchLastSection(false);
    lay->addWidget(tree);

    auto resize_callback = [tree, model]() {
        for (int i{}; i < model->columnCount(); ++i) {
            tree->resizeColumnToContents(i);
        }
    };
    QObject::connect(
        model, &QAbstractItemModel::dataChanged, 
        tree, [resize_callback](const QModelIndex&, const QModelIndex&, const QVector<int>&){
            resize_callback();
        }
    );
    QObject::connect( tree, &QTreeView::expanded, tree,  [resize_callback](auto){ resize_callback(); } );

    return box;
}

QLineEdit* make_lineedit()
{
	auto edit = new QLineEdit;
	edit->setFixedHeight(height);
	edit->setMinimumWidth(width);
	return edit;
}

QWidget* make_widget(QLayout* layout)
{
	auto wgt = new QWidget;
	wgt->setLayout(layout);
	return wgt;
}