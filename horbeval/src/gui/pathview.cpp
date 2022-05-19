#include <pathview.h>
#include <guisettings.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qfiledialog.h>


QString open_filedialog(const QString& filter, QWidget* const wnd)
{
	return QFileDialog::getOpenFileName(wnd, "Открыть файл", {}, filter);
}

pathview::pathview(const QString& filter, const QString& title, QWidget* const parent) : QWidget(parent)
{
	auto layout = new QHBoxLayout;
	layout->setMargin(0);
	layout->setStretch(0, 0);
	layout->setStretch(1, 1);
	layout->setStretch(2, 0);

	setLayout(layout);

	auto label = new QLabel(title);
	label->adjustSize();
	label->setFixedHeight(btn_height);
	label->setFixedWidth(label->width());
	layout->addWidget(label);

	_txtbox = new QLineEdit;
	_txtbox->setFixedHeight(btn_height);
	_txtbox->setMinimumWidth(btn_width);
	layout->addWidget(_txtbox);

	auto button = new QPushButton;
	button->setFixedSize(btn_height, btn_height);
	button->setIcon(visual_provider::file_icon());
	button->setToolTip("Открыть менеджер файлов");
	layout->addWidget(button);

	connect(
		button, &QAbstractButton::clicked,
		this, [this, filter](bool) {
			auto filepath = open_filedialog(filter, this);
			if (!filepath.isEmpty()) {
				_txtbox->setText(filepath);
			}
		}
	);
	connect(
		_txtbox, &QLineEdit::textChanged,
		this, [this](const QString& path) { emit path_changed(path); }
	);
}

QString pathview::get_path() const
{
	return _txtbox->text();
}

void pathview::set_path(const QString& path)
{
	_txtbox->setText(path);
}
