#include "portdialog.h"
#include "ui_portdialog.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QMessageBox>

portDialog::portDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::portDialog)
{
	ui->setupUi(this);
}

portDialog::~portDialog()
{
	delete ui;
}

QString portDialog::get_returnSet()
{
	dialog_SP = QString("SP=%1;").arg(QString::number(int((ui->lineEdit_SP->text().toFloat())*800/3)));
	dialog_AC = QString("AC=%1;").arg(QString::number(int((ui->lineEdit_AC->text().toFloat())*800/3)));
	dialog_DC = QString("DC=%1;").arg(QString::number(int((ui->lineEdit_DC->text().toFloat())*800/3)));
	if(ui->radioButton_CW->isChecked())
		dialog_PR = QString("MO=1;PR=%1;").arg(QString::number(int((ui->lineEdit_PR->text().toFloat())*800/3)));
	else
		dialog_PR = QString("MO=1;PR=-%1;").arg(QString::number(int((ui->lineEdit_PR->text().toFloat())*800/3)));
	request = QString("%1%2%3%4BG;").arg(dialog_SP).arg(dialog_AC).arg(dialog_DC).arg(dialog_PR);
	return request;
}

void portDialog::search_port()								//搜索串口函数
{
	ui->pushButton_auto_searchPort->setEnabled(false);
	QSerialPort my_serial;
	foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
	{
		my_serial.close();
		my_serial.setPortName(info.portName());
		if(!my_serial.open(QIODevice::ReadWrite))
		{
//			qDebug() << "Can't open" << info.portName();
			return;
		}
		my_serial.setBaudRate(QSerialPort::Baud19200);
		my_serial.setDataBits(QSerialPort::Data8);
		my_serial.setStopBits(QSerialPort::OneStop);
		my_serial.setFlowControl(QSerialPort::NoFlowControl);

		QString test("VR;");
		QByteArray testData = test.toLocal8Bit();
		my_serial.write(testData);
		if(my_serial.waitForBytesWritten(15))
		{
			if(my_serial.waitForReadyRead(30))
			{
				QByteArray testResponse = my_serial.readAll();
				while(my_serial.waitForReadyRead(10))
					testResponse += my_serial.readAll();
				QString response(testResponse);
				if(response.left(10) == "VR;Whistle")
				{
					portTested = info.portName();
					ui->lineEdit_serialportName->setText(portTested);
				}
			}
//			else
//				qDebug() << "Timeout 1";
		}
//		else
//			qDebug() << "Timeout 2";
	}
	my_serial.close();
	if(portTested == NULL)
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("Please connect serialport correctly!"));
	ui->pushButton_auto_searchPort->setEnabled(true);
}

void portDialog::inital_data(const QString &c,quint16 a,quint16 b)			//初始数据函数
{
	ui->lineEdit_SP->setText("45");							//速度
	ui->lineEdit_AC->setText("30");							//加速度
	ui->lineEdit_DC->setText("30");							//减速度
	step = a;
	Num = b;
	portTested = c;
	ui->lineEdit_PR->setText(QString::number(step));		//移动距离
	ui->lineEdit_PA->setText(QString::number(step*Num));	//绝对距离
	ui->lineEdit_PX->setText("0");							//当前位置
	ui->radioButton_CCW->setChecked(true);					//逆时针
	ui->lineEdit_serialportName->setText(portTested);
}

void portDialog::on_pushButton_default_clicked()			//默认键
{
	ui->lineEdit_SP->setText("45");							//速度
	ui->lineEdit_AC->setText("30");							//加速度
	ui->lineEdit_DC->setText("30");							//减速度
	ui->lineEdit_PR->setText(QString::number(step));		//移动距离
	ui->lineEdit_PA->setText(QString::number(step*Num));	//绝对距离
	ui->lineEdit_PX->setText("0");							//当前位置
	ui->radioButton_CCW->setChecked(true);					//逆时针
}

void portDialog::on_pushButton_sure_clicked()				//确定键
{
	accept();
}

void portDialog::on_pushButton_cancel_clicked()				//取消键
{
	reject();
}

void portDialog::on_pushButton_auto_searchPort_clicked()	//自动检测键
{
	search_port();
}

void portDialog::on_pushButton_opposite_clicked()			//相对转动键
{
	ui->pushButton_opposite->setEnabled(false);
	dialog_SP = QString("SP=%1;").arg(QString::number(int((ui->lineEdit_SP->text().toFloat())*800/3)));
	dialog_AC = QString("AC=%1;").arg(QString::number(int((ui->lineEdit_AC->text().toFloat())*800/3)));
	dialog_DC = QString("DC=%1;").arg(QString::number(int((ui->lineEdit_DC->text().toFloat())*800/3)));
	if(ui->radioButton_CW->isChecked())
		dialog_PR = QString("MO=1;PR=%1;").arg(QString::number(int((ui->lineEdit_PR->text().toFloat())*800/3)));
	else
		dialog_PR = QString("MO=1;PR=-%1;").arg(QString::number(int((ui->lineEdit_PR->text().toFloat())*800/3)));
	request = QString("%1%2%3%4BG;PX;").arg(dialog_SP).arg(dialog_AC).arg(dialog_DC).arg(dialog_PR);
	qDebug() << "request = " << request;
	thread_dia.transaction(portTested,request);
	ui->pushButton_opposite->setEnabled(true);
}

void portDialog::on_pushButton_absolute_clicked()			//绝对转动键
{
	ui->pushButton_absolute->setEnabled(false);
	dialog_SP = QString("SP=%1;").arg(QString::number(int((ui->lineEdit_SP->text().toFloat())*800/3)));
	dialog_AC = QString("AC=%1;").arg(QString::number(int((ui->lineEdit_AC->text().toFloat())*800/3)));
	dialog_DC = QString("DC=%1;").arg(QString::number(int((ui->lineEdit_DC->text().toFloat())*800/3)));
	if(ui->radioButton_CW->isChecked())
		dialog_PA = QString("MO=1;PA=%1;").arg(QString::number(int((ui->lineEdit_PA->text().toFloat())*800/3)));
	else
		dialog_PA = QString("MO=1;PA=-%1;").arg(QString::number(int((ui->lineEdit_PA->text().toFloat())*800/3)));
	request = QString("%1%2%3%4BG;PX;").arg(dialog_SP).arg(dialog_AC).arg(dialog_DC).arg(dialog_PA);
	qDebug() << "request = " << request;
	thread_dia.transaction(portTested,request);
	ui->pushButton_absolute->setEnabled(true);
}

void portDialog::on_pushButton_setPXis0_clicked()			//设置当前位置为0键
{
	ui->pushButton_setPXis0->setEnabled(false);
	request = QString("MO=0;PX=%1;BG;").arg(QString::number(int((ui->lineEdit_PX->text().toFloat())*800/3)));
	qDebug() << "request = " << request;
	thread_dia.transaction(portTested,request);
	ui->pushButton_setPXis0->setEnabled(true);
}
