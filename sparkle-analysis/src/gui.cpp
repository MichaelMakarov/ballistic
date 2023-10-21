#include <gui.hpp>
#include <pathview.hpp>
#include <qcheckbox.h>
#include <qcommonstyle.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qresource.h>
#include <qspinbox.h>
#include <stdexcept>

constexpr int width{100};
constexpr int height{30};

/**
 * @brief Директория с ресурсами
 *
 */
static QString directory{":/src/icons/"};

static QCommonStyle style;

QIcon file_icon() {
    return style.standardIcon(QStyle::SP_DirIcon);
}

QIcon apply_icon() {
    return QIcon(directory + "check.png");
}

QPixmap number_icon(unsigned number) {
    QString name;
    switch (number) {
    case 1:
        name = "one.png";
        break;
    case 2:
        name = "two.png";
        break;
    case 3:
        name = "three.png";
        break;
    case 4:
        name = "four.png";
        break;
    case 5:
        name = "five.png";
        break;
    default:
        throw std::invalid_argument("Иконка с таким числом отсутствует.");
    }
    return QPixmap(directory + name);
}

QGroupBox *make_groupbox(const QString &title, QLayout *layout) {
    auto box = new QGroupBox(title);
    box->setLayout(layout);
    return box;
}

QPushButton *make_button(const QString &text) {
    auto btn = new QPushButton(text);
    btn->setFixedSize(width, height);
    return btn;
}

QPushButton *make_button(const QIcon &icon, const QString &tooltip) {
    auto btn = new QPushButton;
    btn->setFixedSize(height, height);
    btn->setIcon(icon);
    btn->setToolTip(tooltip);
    return btn;
}

QPushButton *make_apply_button(const QString &tooltip) {
    return make_button(apply_icon(), tooltip);
}

QPushButton *make_file_button(const QString &tooltip) {
    return make_button(file_icon(), tooltip);
}

QLabel *make_label(const QString &text) {
    auto lbl = new QLabel(text);
    lbl->setFixedHeight(height);
    return lbl;
}

QLabel *make_label(unsigned number) {
    auto pixmap = number_icon(number);
    auto lbl = new QLabel;
    lbl->setFixedSize(height, height);
    lbl->setPixmap(pixmap.scaled(lbl->size(), Qt::AspectRatioMode::IgnoreAspectRatio));
    return lbl;
}

template <typename T, typename V>
T *make_spinbox(V value, V minv, V maxv, V step) {
    auto spb = new T;
    spb->setFixedSize(width, height);
    spb->setValue(value);
    spb->setSingleStep(step);
    spb->setRange(minv, maxv);
    spb->setAlignment(Qt::AlignmentFlag::AlignRight);
    return spb;
}

QSpinBox *make_spinbox(int value, int minv, int maxv, int step) {
    return make_spinbox<QSpinBox>(value, minv, maxv, step);
}

QSpinBox *make_spinbox(int value, int minv, int maxv, int step, const std::function<void(int)> &callback) {
    auto spb = make_spinbox<QSpinBox>(value, minv, maxv, step);
    if (callback) {
        QObject::connect(spb, QOverload<int>::of(&QSpinBox::valueChanged), spb, callback);
    }
    return spb;
}

QDoubleSpinBox *make_double_spinbox(double value, double minv, double maxv, double step) {
    return make_spinbox<QDoubleSpinBox>(value, minv, maxv, step);
}

QCheckBox *make_checkbox(const QString &text) {
    auto chb = new QCheckBox(text);
    chb->setFixedHeight(height);
    return chb;
}

QLineEdit *make_lineedit() {
    auto edit = new QLineEdit;
    edit->setFixedHeight(height);
    edit->setMinimumWidth(width);
    return edit;
}

QWidget *make_widget(QLayout *layout) {
    auto wgt = new QWidget;
    wgt->setLayout(layout);
    return wgt;
}