﻿#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "paradialog.h"
#include "Acqsettings.h"
#include "informationleft.h"
#include "ADQAPI.h"
#include "plotwidget.h"
#include "threadstore.h"
#include "settingfile.h"
#include "portdialog.h"
#include "plotdialog.h"

#include <QByteArray>
#include <QDataStream>
#include <QDockWidget>
#include <QtSerialPort/QSerialPort>
#include <QCloseEvent>
#include <QLabel>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	//绘图的全屏显示
    void dockview_ct1(bool topLevel);
	void dockview_ct2(bool topLevel);

	//功能键
	void on_action_searchDevice_triggered();		//搜索键
	void on_action_open_triggered();				//打开键
	void on_action_set_triggered();					//设置键
	void on_action_view_triggered();
	void on_action_start_triggered();				//开始键
	void on_action_stop_triggered();				//停止键
	void on_action_serialport_triggered();			//串口键

	void notrig_over();								//无外部触发信号
	void collect_reset();							//采集结束后停止时，关闭multi-record
	void collect_over();							//采集结束

	void time_count();								//用于计数
	void judge_collect_condition();					//用于判断是否进行采集
	void receive_storefinish();						//存储线程完成，线程数减1
	void Motor_Position(float a);					//获取当前位置，更新圆盘Dial
	void Motor_Arrived();							//电机到达预采集方位
	void Motot_status(bool a);						//状态栏显示电机连接状态
	void set_stop();								//stopped值置为true

protected:
	void closeEvent(QCloseEvent *event);			//程序关闭时，检查存储线程

private:
    Ui::MainWindow *ui;
	ACQSETTING mysetting;
	PLOTPARA m_paraValue;
	void initial_plotValue();						//绘图显示对话框的初始参数设置
	settingFile m_setfile;
	paraDialog *ParaSetDlg;							//设置对话框
	portDialog *PortDialog;							//串口对话框
	plotDialog *PlotDialog;							//绘图设置对话框

	volatile bool onecollect_over;					//用于判断单次采集是否完成
	bool connect_Motor;								//单方向采集连接电机
	bool reach_position;							//电机达到待采集方位
	bool stopped;									//停止采集
	bool success_configure;							//采集卡配置成功
	bool thread_enough;								//存储线程足够

	//USB采集卡
	void *adq_cu;
	void conncetdevice(void);						//连接USB采集卡设备

	//左侧栏
    informationleft *dockleft_dlg;
    QDockWidget *dockWidget;
	QString FileName_1;								//正在采集的文件名信息
	QString FileName_A;
	QString FileName_B;
	void creatleftdock(void);

	//曲线显示部分
    PlotWindow *plotWindow_1;
	PlotWindow *plotWindow_2;
    QDockWidget *dockqwt_1;
    QDockWidget *dockqwt_2;
	bool plot1show;
	bool plot2show;
	void creatqwtdock(void);						//创建曲线显示窗口
	void setPlotWindowVisible(void);				//显示曲线子窗口
	void refresh(void);								//设置完后关于绘图部分更新
	void resizeEvent(QResizeEvent *event);			//主窗口大小改变时

	//状态栏
	QStatusBar *bar;
	QLabel *ADQ_state;			//声明标签对象，用于显示采集卡连接状态
	QLabel *motor_state;		//用于显示电机连接状态
	QLabel *collect_state;		//用于显示采集状态
	QLabel *storenum;			//用于显示存储线程状态
	void Create_statusbar();

	QTimer *timer2;				//判断定时器
	QTimer *timer_judge;		//扫描采集时判断是否满足采集条件
	void adq_para_set();		//采集卡参数设置
	void adq_collect();			//采集卡数据采集
	void single_upload_store();	//单通道数据上传和存储
	void double_upload_store();	//双通道数据上传和存储
	void update_collet_number();//数据上传存储完成后，更新采集信息

	//采集中需要设定的其他参数
	quint32 numbercollect;		//采集方向组数
	QString timestr;			//采集时间
	int direction_angle;		//方位角
	unsigned int number_of_records;		//脉冲数
	unsigned int samples_per_record;	//采样点
	unsigned int n_sample_skip;			//采样间隔
	uint direct_interval_JudgeNum;		//方向间需要判断次数
	uint circle_interval_JudgeNum;		//圆周间需要判断次数
	uint dI_Num_Judged;					//已判断的方向间隔次数
	uint cI_Num_Judged;					//已判断的圆周间隔次数
	int PX_lastData;					//采集结束或停止时的电机位置
	void Create_DataFolder();			//数据存储文件夹的创建

	//数据线程储存
	threadStore threadA;
    threadStore threadB;
    threadStore threadC;
    threadStore threadD;
	int num_running;					//正在运行的线程数
};

#endif // MAINWINDOW_H
