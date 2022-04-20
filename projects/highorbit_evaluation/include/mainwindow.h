#pragma once
#include <qmainwindow.h>

class mainwindow : public QMainWindow {
public:
	mainwindow(QWidget* const parent = nullptr);
private:
	void initialize_centralwidget();
	void initialize_statusbar();
	void initialize_menu();
};