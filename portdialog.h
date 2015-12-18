#ifndef PORTDIALOG_H
#define PORTDIALOG_H

#include <QDialog>
#include "serialportthread.h"

namespace Ui {
class portDialog;
}

class portDialog : public QDialog
{
	Q_OBJECT

public:
	explicit portDialog(QWidget *parent = 0);
	~portDialog();

	void inital_data(const QString &a,int b, bool c,quint32 d,bool e);//默认的设置参数

	int get_returnSet();					//将类型为QString的串口值返回到主线程中

	void search_port();

	bool get_returnMotor_connect();			//返回连接电机的bool值给主程序

	void show_PX(const QString &px_show);	//显示当前位置

	void button_enabled();

signals:
	void portdlg_send(const QString &re);	//对话框返回字符串到主程序

private slots:
//	void opposite_transaction();				//相对转动
//	void absolute_transaction();				//绝对转动

	void on_pushButton_default_clicked();		//默认键

	void on_pushButton_sure_clicked();

	void on_pushButton_cancel_clicked();

	void on_pushButton_auto_searchPort_clicked();

	void on_pushButton_opposite_clicked();

	void on_pushButton_absolute_clicked();

	void on_pushButton_setPXis0_clicked();

	void set_SP();

private:
	Ui::portDialog *ui;
	QString portTested;			//串口名
	SerialPortThread thread_dia;

	QString dialog_SP;			//速度
//	QString dialog_AC;			//加速度
//	QString dialog_DC;			//减速度
	QString dialog_PR;			//移动距离
	QString dialog_PA;			//绝对距离
	QString dialog_PX;			//当前位置
	QString request;
	int retSP;					//返回给主程序的SP值
	bool MotorConnect;			//是否连接电机
	quint32 col_num;			//采集的组数
	bool nocoll;				//是否正在采集
	QString px_str;
	int px_data;
};

#endif // PORTDIALOG_H
