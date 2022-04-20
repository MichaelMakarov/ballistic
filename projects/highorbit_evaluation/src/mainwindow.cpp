#include <mainwindow.h>

#include <qlayout.h>
#include <qtableview.h>
#include <qstatusbar.h>
#include <qtableview.h>


mainwindow::mainwindow(QWidget* const parent /*= nullptr*/) : QMainWindow(parent)
{
	setWindowTitle("Окно уточнения модели движения");

}

void mainwindow::initialize_centralwidget()
{
	auto wgt = new QWidget;
	setCentralWidget(wgt);

	auto grid = new QGridLayout;
	grid->setSpacing(10);

	wgt->setLayout(grid);

	auto meas_table = new QTableView;
	grid->addWidget(meas_table, 0, 0, 1, 2);
}
