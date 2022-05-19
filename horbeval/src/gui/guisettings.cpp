#include <guisettings.h>
#include <qresource.h>

#include <stdexcept>

static QString directory{ ":/src/icons/" };

QCommonStyle visual_provider::_style;

QIcon visual_provider::file_icon()
{
    return _style.standardIcon(QStyle::SP_DirIcon);
}

QIcon visual_provider::apply_icon()
{
	return QIcon(directory + "check.png");
}

QPixmap visual_provider::number_icon(unsigned int number)
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
