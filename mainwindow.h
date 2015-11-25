#ifndef MAINWINDOW_H
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
#include "serialportthread.h"

#include <QByteArray>
#include <QDataStream>
#include <QDockWidget>
#include <QtSerialPort/QSerialPort>

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
	void on_action_saveAs_triggered();				//另存为键
	void on_action_set_triggered();					//设置键
	void on_action_start_triggered();				//开始键
	void on_action_stop_triggered();				//停止键
	void on_action_serialport_triggered();

	void singlecollect(void);						//单通道采集及存储
	void collect_cond();
	void doublecollect(void);						//双通道采集及存储
	void collect_over();							//采集结束用于关闭multi-record
	void receive_response(const QString &s);		//串口线程发送命令后的返回值

private:
    Ui::MainWindow *ui;
    paraDialog *ParaSetDlg;
	portDialog *PortDialog;							//串口的对话框
	SerialPortThread thread_coll;					//串口读写线程

	int SP;											//驱动器速度
	QString portname;								//连接的串口名
	QString request_send;							//发送给串口的命令
	volatile bool onecollect_over;					//用于判断单次采集是否完成
	void search_port();								//串口搜索
	void start_position();							//驱动器的初始角位置
	int PX0, PX1;									//驱动器返回的PX值

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

	QTimer *timer1;									//通道定时器
	QTimer *timer2;									//判断定时器
	void singleset(void);							//单通道参数设置
	void doubleset(void);							//双通道参数设置

	threadStore threadA;							//数据储存线程
    threadStore threadB;
    threadStore threadC;
    threadStore threadD;

	QString strs;									//文件名可自动加1
    int strsuffix;
	int numbercollect;								//采集方向组数
	volatile bool stopped;							//用于判断是否点击了停止采集

	QString timestr;								//采集时间
	uint direction_angle;							//方位角
    void *adq_cu;
    int trig_mode;
    int trig_level;
    int trig_flank;
    int trig_channel;
    int clock_source ;
	int pll_divider;								//和采样频率有关
	unsigned int number_of_records;					//脉冲数
	unsigned int samples_per_record;				//采样点
    unsigned int n_sample_skip;

	void conncetdevice(void);						//连接设备
	void path_create();								//数据存储文件夹的创建
};

#endif // MAINWINDOW_H
