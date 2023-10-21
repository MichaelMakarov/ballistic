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

QGroupBox *make_groupbox(const QString &title, QLayout *layout);

QPushButton *make_button(const QString &text);

QPushButton *make_apply_button(const QString &tooltip);

QPushButton *make_file_button(const QString &tooltip);

QLabel *make_label(const QString &text);

QLabel *make_label(unsigned number);

QSpinBox *make_spinbox(int value, int minv, int maxv, int step);

QDoubleSpinBox *make_double_spinbox(double value, double minv, double maxv, double step);

QCheckBox *make_checkbox(const QString &text);

template <typename T>
auto make_layout() {
    auto layout = new T;
    layout->setSpacing(5);
    return layout;
}

class QLineEdit *make_lineedit();

class QWidget *make_widget(QLayout *layout);