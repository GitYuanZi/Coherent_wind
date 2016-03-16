#ifndef PORTDIALOG_H
#define PORTDIALOG_H

#include <QDialog>
#include "serialportthread.h"
#include <QTimer>

const int MOTOR_RESOLUTION = 96000;			//电机分辨率

namespace Ui {
class portDialog;
}

class portDialog : public QDialog
{
	Q_OBJECT

public:
	explicit portDialog(QWidget *parent = 0);
	~portDialog();

	void initial_data(bool c,quint16 d,bool e);//默认的设置参数
	void search_set_port(int Sp);
	bool get_returnMotor_connect();			//返回连接电机的bool值给主程序
	void show_PX(float px_show);			//显示当前位置
	void update_status();					//更新按钮状态

	void ABS_Rotate(int Pa);				//绝对转动
	void CW_Rotate(int Pr);					//相对正转
	void CCW_Rotate(int Pr);				//相对反转
	void SetPX(int Px);						//设置当前位置PX
	void SetSP(int Sp);						//设置电机速度SP
	void OpenMotor();						//打开电机

signals:
	void portdlg_send(const QString &re);	//对话框返回字符串到主程序
	void SendPX(float a);					//PX值发送给主程序
	void Position_success();				//电机到达预定位置
	void Position_Error();					//电机未达到预定位置
	void Motot_connect_status(bool motorConnect);	//电机连接状态


private slots:
	void on_pushButton_default_clicked();	//默认键
	void on_pushButton_sure_clicked();		//确定键
	void on_pushButton_cancel_clicked();	//取消键
	void on_pushButton_auto_searchPort_clicked();
	void on_pushButton_relative_clicked();	//相对转动键
	void on_pushButton_absolute_clicked();	//绝对转动键
	void on_pushButton_setPXis0_clicked();	//设置当前位置为0键
	void SPchanged();						//SP值改变

	void receive_response(const QString &s);
	void portError_OR_timeout();			//串口未连接或接收命令超时
	void DemandPX();						//查询PX

private:
	Ui::portDialog *ui;
	QString portname;			//串口名
	SerialPortThread thread_port;
	int retSP;					//电机的SP值
	bool Set_MotorConnect;		//是否需要连接电机
	quint16 StepAngle;			//采集的组数
	bool Not_collect;			//是否正在采集
	int PX_data;				//PX的int类型
	bool Motor_Connected;		//电机实际连接状态


	QString Order_str;			//待发送给电机的字符串命令
	QTimer *timer1;				//定时器，用于定时检查当前位置
	bool portDia_status;		//串口对话框打开状态
	bool handle_PX;				//正在处理PX值
};

#endif // PORTDIALOG_H
