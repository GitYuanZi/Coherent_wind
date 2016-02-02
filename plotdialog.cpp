#include "plotdialog.h"
#include "ui_plotdialog.h"
#include <QDebug>

plotDialog::plotDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::plotDialog)
{
	ui->setupUi(this);
}

plotDialog::~plotDialog()
{
	delete ui;
}

//对话框显示
void plotDialog::dialog_show(const PLOTPARA &setting, bool is_1Ch)
{
	p_paraValue = setting;
	p_ch = !is_1Ch;
	ui->checkBox_hide_grid->setChecked(p_paraValue.hide_grid);
	if(!is_1Ch)										//双通道
	{
		ui->checkBox_showA->setEnabled(true);
		ui->checkBox_showB->setEnabled(true);
		ui->checkBox_showA->setChecked(p_paraValue.showA);
		ui->checkBox_showB->setChecked(p_paraValue.showB);
	}
	else
	{

		ui->checkBox_showA->setEnabled(true);
		ui->checkBox_showB->setEnabled(false);
		ui->checkBox_showA->setChecked(p_paraValue.showA);
		ui->checkBox_showB->setChecked(false);
	}
	ui->radioButton_countNum->setChecked(p_paraValue.countNum);
	ui->radioButton_echoDistance->setChecked(p_paraValue.echoDistance);

	connect(ui->checkBox_hide_grid,&QCheckBox::clicked,this,&plotDialog::set_hide_grid);
	connect(ui->checkBox_showA,&QCheckBox::clicked,this,&plotDialog::set_showA);
	connect(ui->checkBox_showB,&QCheckBox::clicked,this,&plotDialog::set_showB);
	connect(ui->radioButton_countNum,&QRadioButton::clicked,this,&plotDialog::set_countNum);
	connect(ui->radioButton_echoDistance,&QRadioButton::clicked,this,&plotDialog::set_echoDistance);
}

PLOTPARA plotDialog::get_settings()
{
	return p_paraValue;
}

//确定键
void plotDialog::on_pushButton_sure_clicked()
{
	accept();
}

//取消键
void plotDialog::on_pushButton_cancel_clicked()
{
	reject();
}

void plotDialog::set_hide_grid()
{
	p_paraValue.hide_grid = ui->checkBox_hide_grid->isChecked();
}

void plotDialog::set_showA()
{
	p_paraValue.showA = ui->checkBox_showA->isChecked();
//	if((p_paraValue.showB == false)&&(p_paraValue.showA == false))
//	{
//		ui->checkBox_showB->setChecked(true);
//		p_paraValue.showB = ui->checkBox_showB->isChecked();
//	}
}

void plotDialog::set_showB()
{
	p_paraValue.showB = ui->checkBox_showB->isChecked();
//	if((p_paraValue.showA == false)&&(p_paraValue.showB == false))
//	{
//		ui->checkBox_showA->setChecked(true);
//		p_paraValue.showA = ui->checkBox_showA->isChecked();
//	}
}

void plotDialog::set_countNum()
{
	p_paraValue.countNum = true;
	p_paraValue.echoDistance = false;
}

void plotDialog::set_echoDistance()
{
	p_paraValue.echoDistance = true;
	p_paraValue.countNum = false;
}
