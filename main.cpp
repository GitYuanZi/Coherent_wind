#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
    MainWindow w;
    w.show();
	app.setWindowIcon(QIcon(":/images/Convert"));
	return app.exec();
}
