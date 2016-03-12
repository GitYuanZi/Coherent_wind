#include "helpdialog.h"
#include "ui_helpdialog.h"

helpDialog::helpDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::helpDialog)
{
	ui->setupUi(this);
}

helpDialog::~helpDialog()
{
	delete ui;
}
void helpDialog::on_pushButton_OK_clicked()
{
	close();
}
