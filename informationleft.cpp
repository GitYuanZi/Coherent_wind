#include "informationleft.h"
#include "ui_informationleft.h"

#include <QString>
#include <QDebug>

#include <qwt_compass.h>
#include <qwt_dial_needle.h>

informationleft::informationleft(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::informationleft)
{
	ui->setupUi(this);
	set_groupcnt(0);

}

informationleft::~informationleft()
{
	delete ui;
}

void informationleft::set_currentAngle(quint16 a)		//当前角度、圆盘示意图
{
	QString str = QString::number(a) + QString::fromLocal8Bit("°");
	ui->label_currentAngle->setText(str);
	ui->Dial->setNeedle(new QwtDialSimpleNeedle(QwtDialSimpleNeedle::Arrow,true,Qt::gray));
	ui->Dial->setValue(a);
}

void informationleft::set_groupNum(quint32 a)			//总组数
{
	groupNum = a;
}

void informationleft::set_groupcnt(quint16 a)			//采集组数/总组数 以及进度条
{
	QString str;
	groupcnt = a;
	str = QString::number(a) + "/" +QString::number(groupNum);
	ui->label_groupcnt->setText(str);
	ui->progressBar_grouppercent->setValue(groupcnt*100/groupNum);
}

void informationleft::set_filename1(QString str)
{
	ui->label_filename1->setText(str);
}

void informationleft::set_filename2(QString str)
{
	ui->label_filename2->setText(str);
}


