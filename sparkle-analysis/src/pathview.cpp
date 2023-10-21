#include <gui.hpp>
#include <pathview.hpp>
#include <qfiledialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>

QString open_filedialog(const QString &filter, QWidget *const wnd) {
    return QFileDialog::getOpenFileName(wnd, "Открыть файл", {}, filter);
}

filepath_view::filepath_view(const QString &filter, const QString &title, QWidget *const parent)
    : QWidget(parent) {
    auto layout = new QHBoxLayout;
    setLayout(layout);

    auto label = make_label(title);
    label->adjustSize();
    label->setFixedWidth(label->width());
    layout->addWidget(label, 0);

    layout->addWidget(_txtbox = make_lineedit(), 1);

    auto button = make_file_button("Открыть менеджер файлов");
    layout->addWidget(button, 0);
	
    connect(button, &QPushButton::clicked, this, [this, filter](bool) {
        auto filepath = open_filedialog(filter, this);
        if (!filepath.isEmpty()) {
            _txtbox->setText(filepath);
        }
    });
}

QString filepath_view::get_path() const {
    return _txtbox->text();
}

void filepath_view::set_path(const QString &path) {
    _txtbox->setText(path);
}
