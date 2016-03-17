#include "mainwindow.h"
#include <QApplication>
#include <QtCore>
int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setWindowIcon(QIcon(":/images/Convert"));
	QFont font("Microsoft YaHei UI");
	app.setFont(font);
	MainWindow w;
	w.show();
	return app.exec();
}
