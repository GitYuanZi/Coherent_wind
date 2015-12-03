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
	currentAngle = a;
	QString str;
	str = QString::number(currentAngle) + QString::fromLocal8Bit("°");
	ui->label_currentAngle->setText(str);
	ui->Compass->setNeedle(new QwtCompassWindArrow(QwtCompassWindArrow::Style2));
	quint16 b = 360 + 90 - a;
	if(b > 360)
		b = b -360;
	ui->Compass->setValue(b);

}

void informationleft::set_groupNume(quint32 a)			//总组数
{
	groupNume = a;
}

void informationleft::set_groupcnt(quint16 a)			//采集组数/总组数 以及进度条
{
	QString str;
	groupcnt = a;
	str = QString::number(a) + "/" +QString::number(groupNume);
	ui->label_groupcnt->setText(str);
	ui->progressBar_grouppercent->setValue(groupcnt*100/groupNume);
}

void informationleft::set_filename1(QString str)
{
	ui->label_filename1->setText(str);
}

void informationleft::set_filename2(QString str)
{
	ui->label_filename2->setText(str);
}


