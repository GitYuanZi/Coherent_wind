#include "portdialog.h"
#include "ui_portdialog.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include <QDebug>
#include <QMessageBox>
#include <QString>

portDialog::portDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::portDialog)
{
	ui->setupUi(this);

	timer1= new QTimer(this);
	connect(timer1,SIGNAL(timeout()),this,SLOT(DemandPX()));

	connect(&thread_port,SIGNAL(response(QString)),this,SLOT(receive_response(QString)));
	connect(&thread_port,SIGNAL(S_PortNotOpen()),this,SLOT(portError_OR_timeout()));
	connect(&thread_port,SIGNAL(timeout()),this,SLOT(portError_OR_timeout()));
	handle_PX = false;
}

portDialog::~portDialog()
{
	delete ui;
}

bool portDialog::get_returnMotor_connect()					//返回连接电机bool值给主程序
{
	Set_MotorConnect = ui->checkBox_motor_connected->isChecked();
	return Set_MotorConnect;
}

void portDialog::search_set_port(int Sp)
{
	portDia_status = false;
	retSP = Sp;												//对话框接收SP值
	ui->pushButton_auto_searchPort->setEnabled(false);

	//搜索串口函数
	portname.clear();
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
					portname = info.portName();
					ui->lineEdit_serialportName->setText(portname);
					break;
				}
			}
		}
	}
	my_serial.close();

	if(portname.left(3) == "COM")
	{
		Motor_Connected = true;								//电机连接
		OpenMotor();										//打开电机
	}
	else
	{
		Motor_Connected = false;							//电机未连接
		ui->pushButton_auto_searchPort->setEnabled(true);	//未检测到串口，按钮设为使能状态
	}

	emit this->Motot_connect_status(Motor_Connected);		//主界面状态栏显示电机连接状态
}

void portDialog::initial_data( bool c, quint16 d,bool e)	//初始数据
{
	Set_MotorConnect = c;									//连接电机的bool值
	StepAngle = d;											//步进角
	Not_collect = e;										//正在采集的bool值
	portDia_status = false;

	ui->lineEdit_serialportName->setText(portname);			//串口名
	ui->lineEdit_SP->setText(QString::number(retSP));		//速度
	ui->lineEdit_AC->setText("180");						//加速度
	ui->lineEdit_DC->setText("180");						//减速度
	ui->radioButton_CW->setChecked(true);					//顺时针
	ui->lineEdit_PR->setText("0");							//移动距离
	ui->lineEdit_PA->setText("0");							//绝对距离
	ui->checkBox_motor_connected->setChecked(Set_MotorConnect);	//连接电机

	if(Not_collect  == false)								//若正在采集，界面除取消键，其他均为非使能状态
	{
		ui->groupBox->setEnabled(false);					//通信设置box
		ui->groupBox_2->setEnabled(false);					//转动参数设定box
		ui->groupBox_3->setEnabled(false);					//直接控制box
		ui->groupBox_4->setEnabled(false);					//当前位置设定box
		ui->groupBox_motor->setEnabled(false);				//单方向采集box
		ui->pushButton_default->setEnabled(false);			//默认键
		ui->pushButton_sure->setEnabled(false);				//确定键
	}
	if(StepAngle == 0)										//单方向采集，box为使能状态
		ui->groupBox_motor->setEnabled(true);
	else
		ui->groupBox_motor->setEnabled(false);
	connect(ui->lineEdit_SP,&QLineEdit::textChanged,this,&portDialog::SPchanged);
}

void portDialog::SPchanged()
{
	if(ui->lineEdit_SP->text().toInt() > 90)
		ui->lineEdit_SP->setText("90");
	else
		retSP = ui->lineEdit_SP->text().toInt();
}

void portDialog::on_pushButton_default_clicked()			//默认键
{
	portDia_status = false;
	ui->lineEdit_SP->setText(QString::number(retSP));		//速度
	ui->lineEdit_AC->setText("180");						//加速度
	ui->lineEdit_DC->setText("180");						//减速度
	ui->radioButton_CCW->setChecked(true);					//逆时针
	ui->lineEdit_PR->setText("0");							//移动距离
	ui->lineEdit_PA->setText("0");							//绝对距离
	ui->checkBox_motor_connected->setChecked(Set_MotorConnect);	//连接电机

	if(StepAngle == 0)										//单方向采集，box为使能状态
		ui->groupBox_motor->setEnabled(true);
	else
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
	search_set_port(retSP);
}

void portDialog::on_pushButton_relative_clicked()			//相对转动键
{
	ui->pushButton_auto_searchPort->setEnabled(false);
	ui->pushButton_relative->setEnabled(false);
	ui->pushButton_absolute->setEnabled(false);
	ui->pushButton_setPXis0->setEnabled(false);
	ui->pushButton_default->setEnabled(false);
	ui->pushButton_sure->setEnabled(false);
	ui->pushButton_cancel->setEnabled(false);
	portDia_status = true;
	if(ui->radioButton_CW->isChecked())
		CW_Rotate(ui->lineEdit_PR->text().toInt());
	else
		CCW_Rotate(ui->lineEdit_PR->text().toInt());
}

void portDialog::on_pushButton_absolute_clicked()			//绝对转动键
{
	ui->pushButton_auto_searchPort->setEnabled(false);
	ui->pushButton_relative->setEnabled(false);
	ui->pushButton_absolute->setEnabled(false);
	ui->pushButton_setPXis0->setEnabled(false);
	ui->pushButton_default->setEnabled(false);
	ui->pushButton_sure->setEnabled(false);
	ui->pushButton_cancel->setEnabled(false);
	portDia_status = true;
	ABS_Rotate(ui->lineEdit_PA->text().toInt());
}

void portDialog::on_pushButton_setPXis0_clicked()			//设置当前位置为0键
{
	ui->pushButton_auto_searchPort->setEnabled(false);
	ui->pushButton_relative->setEnabled(false);
	ui->pushButton_absolute->setEnabled(false);
	ui->pushButton_setPXis0->setEnabled(false);
	ui->pushButton_default->setEnabled(false);
	ui->pushButton_sure->setEnabled(false);
	ui->pushButton_cancel->setEnabled(false);
	portDia_status = true;
	SetPX(ui->lineEdit_PX->text().toInt());
}

void portDialog::show_PX(float px_show)
{
	ui->lineEdit_PX->setText(QString::number(px_show,'f',2));
}

void portDialog::update_status()
{
	ui->pushButton_auto_searchPort->setEnabled(true);
	ui->pushButton_relative->setEnabled(true);
	ui->pushButton_absolute->setEnabled(true);
	ui->pushButton_setPXis0->setEnabled(true);
	ui->pushButton_default->setEnabled(true);
	ui->pushButton_sure->setEnabled(true);
	ui->pushButton_cancel->setEnabled(true);
	portDia_status = false;
}

void portDialog::ABS_Rotate(int Pa)
{
	PX_data = Pa;
	Order_str = "PA="+QString::number(Pa*MOTOR_RESOLUTION/360)+";SP="+QString::number(retSP*MOTOR_RESOLUTION/360)+";BG;";
	thread_port.transaction(portname,Order_str);
}

void portDialog::CW_Rotate(int Pr)
{
	PX_data = PX_data + Pr;
	Order_str = "PR="+QString::number(Pr*MOTOR_RESOLUTION/360)+";SP="+QString::number(retSP*MOTOR_RESOLUTION/360)+";BG;";
	thread_port.transaction(portname,Order_str);
}

void portDialog::CCW_Rotate(int Pr)
{
	PX_data = PX_data - Pr;
	Order_str = "PR=-"+QString::number(Pr*MOTOR_RESOLUTION/360)+";BG;";
	thread_port.transaction(portname,Order_str);
}

void portDialog::SetPX(int Px)
{
	Order_str = "MO=0;PX="+QString::number(Px*MOTOR_RESOLUTION/360)+";MO=1;";
	thread_port.transaction(portname,Order_str);
}

void portDialog::SetSP(int Sp)
{
	Order_str = "SP="+QString::number(Sp*MOTOR_RESOLUTION/360)+";AC=48000;DC=48000;PX;";
	thread_port.transaction(portname,Order_str);
}

void portDialog::OpenMotor()
{
	Order_str = "MO=1;";
	thread_port.transaction(portname,Order_str);
}

void portDialog::DemandPX()
{
	if(handle_PX == false)
	{
		handle_PX = true;
		Order_str = "PX;MS;";
		thread_port.transaction(portname,Order_str);
	}
}

void portDialog::receive_response(const QString &s)
{
	QString res = s;
	if((s.left(2) == "PA")||(s.left(2) == "PR"))
		timer1->start(60);									//打开定时器timer1
	else
		if(s.left(2) == "PX")								//获取当前位置值
		{
			handle_PX = true;								//正在处理PX;MS;命令
			QStringList list = res.split(";");
			QString ret1 = list.at(1).toLocal8Bit().data();	//PX值
			QString ret2 = list.at(3).toLocal8Bit().data();	//MS值
			if(ret2 == "0")
				timer1->stop();								//电机停止转动，关闭定时器

			float ret1_data = ret1.toFloat()*360/MOTOR_RESOLUTION;
			emit this->SendPX(ret1_data);					//更新左侧栏圆盘

			if(portDia_status)
			{
				show_PX(ret1_data);							//对话框显示当前PX值
				if(ret2 == "0")
					update_status();
			}
			else
				if(ret2 == "0")								//电机停止转动下
				{
					int ret1_int = ret1_data + 0.5;
					if(((PX_data-3) <= ret1_int)&&(ret1_int <= (PX_data+3)))
						emit this->Position_success(ret1_int);
					else
						emit this->Position_Error();		//电机位置错误
				}
			handle_PX = false;								//PX;MS;命令处理完毕
		}
		else
			if(s.left(2) == "MO")
			{			
				if(s.left(4) == "MO=1")						//MO=1,打开电机后，设置速度SP
					SetSP(retSP);
				if((s.left(4) == "MO=0")&&portDia_status)	//MO=0,设置当前位置后，更新对话框中的按钮
					update_status();
			}
}

void portDialog::portError_OR_timeout()						//串口未能打开或连接超时
{
	timer1->stop();											//关闭定时器
	Motor_Connected = false;								//电机未连接
	emit this->Motot_connect_status(Motor_Connected);		//主界面状态栏显示电机未能正确连接
	if(portDia_status)
		update_status();									//更新对话框按钮
}
