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

int portDialog::get_returnSet()								//返回SP值给主程序
{
	retSP = ui->lineEdit_SP->text().toInt();
	return retSP;
}

bool portDialog::get_returnMotor_connect()					//返回连接电机bool值给主程序
{
	HoldOff = ui->checkBox_motor_connected->isChecked();
	return HoldOff;
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
			return;
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
					break;
				}
			}
		}
	}
	my_serial.close();
	if(portTested == NULL)
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("Please connect serialport correctly!"));
	emit this->portdlg_send(portTested);
	ui->pushButton_auto_searchPort->setEnabled(true);
}

void portDialog::inital_data(const QString &a,int b, bool c, int d,bool e)//初始数据函数
{
	portTested = a;
	retSP = b;
	HoldOff = c;
	col_num = d;
	nocoll = e;
	ui->lineEdit_serialportName->setText(portTested);		//串口名
	ui->lineEdit_SP->setText(QString::number(retSP));		//速度
	ui->lineEdit_AC->setText("180");						//加速度
	ui->lineEdit_DC->setText("180");						//减速度
	ui->radioButton_CCW->setChecked(true);					//逆时针
	ui->lineEdit_PR->setText("0");							//移动距离
	ui->lineEdit_PA->setText("0");							//绝对距离
	ui->lineEdit_PX->setText("0");							//当前位置
	if(!nocoll)												//若正在采集，界面除取消键，其他均为非使能状态
	{
		ui->groupBox->setEnabled(false);
		ui->groupBox_2->setEnabled(false);
		ui->groupBox_3->setEnabled(false);
		ui->groupBox_4->setEnabled(false);
		ui->groupBox_motor->setEnabled(false);
		ui->pushButton_default->setEnabled(false);
		ui->pushButton_sure->setEnabled(false);
	}
	ui->checkBox_motor_connected->setChecked(HoldOff);		//连接电机
	if(!(col_num == 1))
		ui->groupBox_motor->setEnabled(false);
}

void portDialog::on_pushButton_default_clicked()			//默认键
{
	ui->lineEdit_SP->setText(QString::number(retSP));		//速度
	ui->lineEdit_AC->setText("180");						//加速度
	ui->lineEdit_DC->setText("180");						//减速度
	ui->radioButton_CCW->setChecked(true);					//逆时针
	ui->lineEdit_PR->setText("0");							//移动距离
	ui->lineEdit_PA->setText("0");							//绝对距离
	ui->lineEdit_PX->setText("0");							//当前位置
	ui->checkBox_motor_connected->setChecked(HoldOff);		//连接电机
	if(!(col_num == 1))
		ui->groupBox_motor->setEnabled(false);
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
//	dialog_AC = QString("AC=%1;").arg(QString::number(int((ui->lineEdit_AC->text().toFloat())*800/3)));
//	dialog_DC = QString("DC=%1;").arg(QString::number(int((ui->lineEdit_DC->text().toFloat())*800/3)));
	if(ui->radioButton_CW->isChecked())
		dialog_PR = QString("MO=1;PR=-%1;").arg(QString::number(int((ui->lineEdit_PR->text().toFloat())*800/3)));
	else
		dialog_PR = QString("MO=1;PR=%1;").arg(QString::number(int((ui->lineEdit_PR->text().toFloat())*800/3)));
	request = QString("%1AC=48000;DC=48000;%2BG;PX;").arg(dialog_SP).arg(dialog_PR);
	qDebug() << "request = " << request;
//	thread_dia.transaction(portTested,request);
	emit this->portdlg_send(request);
	ui->pushButton_opposite->setEnabled(true);
}

void portDialog::on_pushButton_absolute_clicked()			//绝对转动键
{
	ui->pushButton_absolute->setEnabled(false);
	dialog_SP = QString("SP=%1;").arg(QString::number(int((ui->lineEdit_SP->text().toFloat())*800/3)));
//	dialog_AC = QString("AC=%1;").arg(QString::number(int((ui->lineEdit_AC->text().toFloat())*800/3)));
//	dialog_DC = QString("DC=%1;").arg(QString::number(int((ui->lineEdit_DC->text().toFloat())*800/3)));
	if(ui->radioButton_CW->isChecked())
		dialog_PA = QString("MO=1;PA=-%1;").arg(QString::number(int((ui->lineEdit_PA->text().toFloat())*800/3)));
	else
		dialog_PA = QString("MO=1;PA=%1;").arg(QString::number(int((ui->lineEdit_PA->text().toFloat())*800/3)));
	request = QString("%1AC=48000;DC=48000;%2BG;PX;").arg(dialog_SP).arg(dialog_PA);
	qDebug() << "request = " << request;
//	thread_dia.transaction(portTested,request);
	emit this->portdlg_send(request);
	ui->pushButton_absolute->setEnabled(true);
}

void portDialog::on_pushButton_setPXis0_clicked()			//设置当前位置为0键
{
	ui->pushButton_setPXis0->setEnabled(false);
	request = "MO=0;PX=0;MO=1;";
	qDebug() << "request = " << request;
//	thread_dia.transaction(portTested,request);
	emit this->portdlg_send(request);
	ui->pushButton_setPXis0->setEnabled(true);
}

void portDialog::show_PX(const QString &px_show)
{
	QString strPx = px_show;
	ui->lineEdit_PX->setText(strPx);
}
