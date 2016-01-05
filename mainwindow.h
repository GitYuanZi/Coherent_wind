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
    void dockview_ct1(bool topLevel);
	void dockview_ct2(bool topLevel);				//绘图的全屏显示

	void on_action_searchDevice_triggered();		//搜索键
	void on_action_open_triggered();				//打开键
	void on_action_set_triggered();					//设置键
	void on_action_start_triggered();				//开始键
	void on_action_stop_triggered();				//停止键
	void on_action_serialport_triggered();

	void singlecollect(void);						//单通道采集及存储
	void doublecollect(void);						//双通道采集及存储
	void notrig_over();								//无外部触发信号
	void collect_reset();							//采集结束后停止时，关闭multi-record和stopped
	void collect_over();							//采集结束

	void judge_collect_condition();					//用于判断是否进行采集
	void receive_storefinish();						//存储线程完成，线程数减1
	void Motor_Position(float a);					//获取当前位置，更新圆盘Dial
	void Motor_Arrived();							//电机到达预采集方位
	void set_stop();								//stopped值置为true

protected:
	void closeEvent(QCloseEvent *event);			//程序关闭时，检查存储线程

private:
    Ui::MainWindow *ui;
    paraDialog *ParaSetDlg;
	portDialog *PortDialog;							//串口的对话

	volatile bool onecollect_over;					//用于判断单次采集是否完成
	bool connect_Motor;								//单方向采集连接电机
	bool reach_position;							//电机达到待采集方位

    ACQSETTING mysetting;

    settingFile m_setfile;

    informationleft *dockleft_dlg;
    QDockWidget *dockWidget;
	void creatleftdock(void);						//创建左侧栏

    PlotWindow *plotWindow_1;
	PlotWindow *plotWindow_2;						//曲线子窗口，通道1、2
    QDockWidget *dockqwt_1;
    QDockWidget *dockqwt_2;
	void creatqwtdock(void);						//创建曲线显示窗口
	void setPlotWindowVisible(void);				//显示曲线子窗口
	void refresh(void);								//设置完后关于绘图部分更新
	void resizeEvent(QResizeEvent *event);			//主窗口大小改变时

	QTimer *timer_judge;		//判断是否满足采集条件
	QTimer *timer2;				//判断定时器
	void singleset(void);		//单通道参数设置
	void doubleset(void);		//双通道参数设置

	threadStore threadA;		//数据储存线程
    threadStore threadB;
    threadStore threadC;
    threadStore threadD;
	int num_running;			//正在运行的线程数
	void Create_statusbar();	//设置状态栏
	QStatusBar *bar;
	QLabel *ADQ_state;			//声明标签对象，用于显示采集卡连接状态
	QLabel *motor_state;		//用于显示电机连接状态
	QLabel *collect_state;		//用于显示采集状态
	QLabel *storenum;			//用于显示存储线程状态

	QString FileName_1;			//完整文件名
	QString FileName_A;
	QString FileName_B;

	quint32 numbercollect;		//采集方向组数
	bool stopped;				//停止采集

	QString timestr;			//采集时间
	int direction_angle;		//方位角
    void *adq_cu;
    int clock_source ;
	int pll_divider;					//PLL分频数，和采样频率有关
	unsigned int number_of_records;		//脉冲数
	unsigned int samples_per_record;	//采样点
    unsigned int n_sample_skip;

	void conncetdevice(void);			//连接USB采集卡设备
	void Create_DataFolder();			//数据存储文件夹的创建
	int PX_lastData;					//采集结束或停止时的电机位置

};

#endif // MAINWINDOW_H
