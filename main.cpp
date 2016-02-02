#include "mainwindow.h"
#include <QApplication>
#include <QtCore>
int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	MainWindow w;
	w.show();
	app.setWindowIcon(QIcon(":/images/Convert"));
	QFont font("Microsoft YaHei UI");
	app.setFont(font);
	return app.exec();
}
