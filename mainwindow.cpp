#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ADQAPI.h"
#include "threadstore.h"

#include <QDebug>
#include <QFileDialog>
#include <QSettings>

#include <QMessageBox>
#include <QFile>
#include <QDateTime>
#include <QtCore>

#include <QDesktopServices>
#include <QApplication>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>


MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	QDir dirs;
	QString paths = dirs.currentPath()+"/"+"settings.ini";				//获取初始默认路径，并添加默认配置文件
	m_setfile.test_create_file(paths);									//检查settings.ini是否存在，若不存在则创建
	m_setfile.readFrom_file(paths);										//读取settings.ini文件
	mysetting = m_setfile.get_setting();								//mysetting获取文件中的参数

	creatleftdock();													//左侧栏
	creatqwtdock();														//曲线栏
	conncetdevice();													//连接采集卡设备
	//	search_port();													//串口搜索
	portname = "COM3";
	connect(&thread_coll, SIGNAL(response(QString)),this,SLOT(receive_response(QString)));//用于接收线程的emit
	connect(&thread_coll, SIGNAL(portOpen()),this,SLOT(receive_portopen()));//连接串口未打开时对应的槽函数
	connect(&thread_coll, SIGNAL(timeout()),this,SLOT(receive_timeout()));//接收串口命令超时

	PortDialog = new portDialog(this);
	connect(PortDialog,SIGNAL(portdlg_send(QString)),this,SLOT(receive_portdlg(QString)));

	timer1 = new QTimer(this);											//触发各个方向上开始采集
	timer2 = new QTimer(this);											//用于判断do—while
	timer2->setSingleShot(true);
	connect(timer2,SIGNAL(timeout()),this,SLOT(collect_over()));		//建立用于检查do-while的定时器
	connect(timer1,SIGNAL(timeout()),this,SLOT(collect_cond()));		//定时器连接位置判断函数
	SP = 90;															//驱动器速度默认45
	trig_HoldOff = true;												//单方向探测默认连接电机
	stopped = true;														//初始状态，未进行数据采集
}

MainWindow::~MainWindow()
{
	DeleteADQControlUnit(adq_cu);
	delete ui;
}

//创建左侧信息显示栏
void MainWindow::creatleftdock(void)
{
	dockleft_dlg = new informationleft(this);
	dockWidget = new QDockWidget;
	dockWidget->setWidget(dockleft_dlg);
	dockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
	addDockWidget(Qt::LeftDockWidgetArea, dockWidget);

	dockleft_dlg->set_currentAngle(mysetting.start_azAngle);
	dockleft_dlg->set_groupNume(mysetting.angleNum);
	dockleft_dlg->set_groupcnt(0);

	QDateTime filepredate = QDateTime::currentDateTime();
	mysetting.dataFileName_Prefix = filepredate.toString("yyyyMMdd");	//将文件名前缀更新至最新日期

	if(mysetting.singleCh)
	{
		FileName_1 = mysetting.dataFileName_Prefix + "_ch1_" + mysetting.dataFileName_Suffix + ".wld";
		dockleft_dlg->set_filename1(FileName_1);
		dockleft_dlg->set_filename2(NULL);
	}
	else
	{
		FileName_A = mysetting.dataFileName_Prefix + "_chA_" + mysetting.dataFileName_Suffix + ".wld";
		FileName_B = mysetting.dataFileName_Prefix + "_chB_" + mysetting.dataFileName_Suffix + ".wld";
		dockleft_dlg->set_filename1(FileName_A);
		dockleft_dlg->set_filename2(FileName_B);
	}
}

//创建曲线显示窗口
void MainWindow::creatqwtdock(void)
{
	if(mysetting.singleCh)									//单通道
	{
		plotWindow_1 = new PlotWindow(this);
		dockqwt_1 = new QDockWidget;

		dockqwt_1->setWidget(plotWindow_1);
		dockqwt_1->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
		addDockWidget(Qt::RightDockWidgetArea,dockqwt_1);
		plotWindow_1->setMaxX(mysetting.sampleNum);			//x轴坐标值范围，初始坐标曲线设置
		plotWindow_1->set_titleName("CH1");					//通道名
		connect(dockqwt_1,&QDockWidget::topLevelChanged,this,&MainWindow::dockview_ct1);
		//双击显示全屏绘图
	}
	else if(mysetting.doubleCh)								//双通道
	{
		plotWindow_1 = new PlotWindow(this);
		dockqwt_1 = new QDockWidget;
		plotWindow_2 = new PlotWindow(this);
		dockqwt_2 = new QDockWidget;

		dockqwt_1->setWidget(plotWindow_1);
		dockqwt_1->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
		addDockWidget(Qt::RightDockWidgetArea,dockqwt_1);
		plotWindow_1->setMaxX(mysetting.sampleNum);
		plotWindow_1->set_titleName("CHA");
		connect(dockqwt_1,&QDockWidget::topLevelChanged,this,&MainWindow::dockview_ct1);

		dockqwt_2->setWidget(plotWindow_2);
		dockqwt_2->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
		addDockWidget(Qt::RightDockWidgetArea,dockqwt_2);
		plotWindow_2->setMaxX(mysetting.sampleNum);
		plotWindow_2->set_titleName("CHB");
		connect(dockqwt_2,&QDockWidget::topLevelChanged,this,&MainWindow::dockview_ct2);
	}
	setPlotWindowVisible();
}

void MainWindow::setPlotWindowVisible()						//设置绘图窗口的尺寸
{
	int w = width();
	int h = height();
	w = w - 265, h = (h-25-42-22-25)/2;						//高度：菜单25，工具42，状态22，左侧边栏宽度260，再空去25的高度
	if(mysetting.singleCh)
	{
		plotWindow_1->setFixedSize(w,h);
		plotWindow_1->setMaximumSize(1920,1080);			//绘图窗口的最大尺寸
	}

	if(mysetting.doubleCh)
	{
		plotWindow_1->setFixedSize(w,h);
		plotWindow_1->setMaximumSize(1920,1080);
		plotWindow_2->setFixedSize(w,h);
		plotWindow_2->setMaximumSize(1920,1080);
	}
}

void MainWindow::resizeEvent(QResizeEvent *event)			//主窗口大小改变时，保证绘图窗口填满
{
	setPlotWindowVisible();
}

void MainWindow::dockview_ct1(bool topLevel)				//用于通道1全屏
{
	if(topLevel)
		dockqwt_1->showMaximized();
}

void MainWindow::dockview_ct2(bool topLevel)				//用于通道2全屏
{
	if(topLevel)
		dockqwt_2->showMaximized();
}

void MainWindow::on_action_searchDevice_triggered()			//action_searchDevice键
{
	conncetdevice();
}

void MainWindow::on_action_open_triggered()					//action_open键
{
	if(mysetting.DatafilePath.isEmpty())					//路径为空时，打开默认路径
	{
		QDir default_path;
		QString storepath = default_path.currentPath();
		QDesktopServices::openUrl(QUrl::fromLocalFile(storepath));
	}
	else													//路径存在时，打开对应的指定路径
	{
		path_create();
		QDesktopServices::openUrl(QUrl::fromLocalFile(mysetting.DatafilePath));
	}
}

void MainWindow::on_action_saveAs_triggered()				//action_saveAs键
{
	QString fileName = QFileDialog::getSaveFileName(this,"另存","/","data files(*.wld)");
}

void MainWindow::on_action_set_triggered()					//action_set键
{
	ParaSetDlg = new paraDialog(this);
	//	ParaSetDlg->setAttribute(Qt::WA_DeleteOnClose);	//退出时自动delete，防止内存泄漏
	//	ParaSetDlg->show();
	//	ParaSetDlg->setWindowTitle(QString::fromLocal8Bit("设置"));
	//	ParaSetDlg->setWindowIcon(QIcon(":/images/set"));

	ParaSetDlg->init_setting(mysetting,SP,stopped);					//mysetting传递给设置窗口psetting
	ParaSetDlg->initial_para();										//参数显示在设置窗口上，并连接槽
	ParaSetDlg->on_checkBox_autocreate_datafile_clicked();			//更新文件存储路径

	if(mysetting.singleCh)											//当单通道时打开设置对话框后，创建plotwindow2窗口，点击确定以后，用于refresh中的delete
	{
		plotWindow_2 = new PlotWindow(this);
		dockqwt_2 = new QDockWidget;
	}

	if (ParaSetDlg->exec() == QDialog::Accepted)					// 确定键功能
	{
		mysetting =	ParaSetDlg->get_setting();						//mysetting获取修改后的参数

		dockleft_dlg->set_currentAngle(mysetting.start_azAngle);	//更新左侧栏
		dockleft_dlg->set_groupNume(mysetting.angleNum);
		dockleft_dlg->set_groupcnt(0);
		if(mysetting.singleCh)
		{
			FileName_1 = mysetting.dataFileName_Prefix + "_ch1_" + mysetting.dataFileName_Suffix + ".wld";
			dockleft_dlg->set_filename1(FileName_1);
			dockleft_dlg->set_filename2(NULL);
		}
		else
		{
			FileName_A = mysetting.dataFileName_Prefix + "_chA_" + mysetting.dataFileName_Suffix + ".wld";
			FileName_B = mysetting.dataFileName_Prefix + "_chB_" + mysetting.dataFileName_Suffix + ".wld";
			dockleft_dlg->set_filename1(FileName_A);
			dockleft_dlg->set_filename2(FileName_B);
		}
		refresh();												//更新绘图窗口
		//		start_position();								//驱动器初始位置
	}
	else
		if(mysetting.singleCh)										//点击非确定键，则删除创建的plotwindow2窗口
		{
			delete plotWindow_2;
			delete dockqwt_2;
		}
	delete ParaSetDlg;												//防止内存泄漏
}

void MainWindow::start_position()							//电机转动到初始位置
{
	int startAngle = mysetting.start_azAngle*800/3;			//初始角
	QString start_data = "SP="+QString::number(SP*800/3)+";MO=1;PA="+QString::number(startAngle)+";BG;";//初始角转换为QString型
	qDebug() << "start_data = " << start_data;				//PA为绝对转动
	thread_coll.transaction(portname,start_data);			//设定驱动器的初始位置，命令为SP= ;MO=1;PA= ;BG;
}

void MainWindow::on_action_serialport_triggered()			//action_serialport键
{
//	PortDialog = new portDialog(this);
	PortDialog->inital_data(portname,SP,trig_HoldOff,mysetting.angleNum,stopped);
	if (PortDialog->exec() == QDialog::Accepted)
	{
		SP = PortDialog->get_returnSet();					//从串口对话框接收SP值
		trig_HoldOff = PortDialog->get_returnMotor_connect();//从串口对话框接收连接电机bool值
	}
//	delete PortDialog;										//防止内存泄露
}

void MainWindow::path_create()						//数据存储文件夹的创建
{
	QString currentpath = mysetting.DatafilePath;
	QDir mypath;
	if(!mypath.exists(currentpath))					//文件夹不存在，创建文件夹
		mypath.mkpath(currentpath);
}

void MainWindow::refresh()							//paradialog重新设置后，对绘图曲线部分进行更新
{
	delete plotWindow_1;
	delete dockqwt_1;
	delete plotWindow_2;
	delete dockqwt_2;
	creatqwtdock();
}

void MainWindow::on_action_start_triggered()		//采集菜单中的开始键
{
	stopped = false;								//stopped为false。能够采集
	path_create();									//数据存储文件夹的创建
	numbercollect = 0;

	clock_source = 0;								//时钟源选择0，内部时钟，内部参考
	n_sample_skip = 1;								//采样间隔设为1，表示无采样间隔
	qDebug() << "n_sample_skip = " << n_sample_skip;
	if(ADQ212_SetSampleSkip(adq_cu,1,n_sample_skip) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("SampleSkip"));
		return;
	}

	if(trig_HoldOff)								//连接电机
	{
		start_position();							//驱动器初始位置
		int pr_data = mysetting.step_azAngle*800/3;
		request_send = "PR="+QString::number(pr_data)+";BG;";//设定驱动器的PR值，命令为PR= ;BG;
		PX0 = 96001;
		onecollect_over = true;						//采集开始时，该值设定为true
		if(mysetting.singleCh)						//单通道采集
		{
			singleset();
			timer1->start(500);
		}
		else										//双通道采集
		{
			doubleset();
			timer1->start(500);
		}
	}
	else											//不连接电机
	{
		direction_angle = mysetting.start_azAngle+numbercollect*mysetting.step_azAngle;
		if(mysetting.singleCh)
		{
			singleset();
			singlecollect();
		}
		else
		{
			doubleset();
			doublecollect();
		}
	}
}

void MainWindow::collect_cond()						//位置判断函数，利用定时器定时发送命令
{
	QString judge("PX;");
	thread_coll.transaction(portname,judge);		//发送PX;接收返回值
	qDebug() << "judge = " << judge;
}

void MainWindow::receive_response(const QString &s)	//处理串口返回值
{
	if(s.left(2) == "PX")
	{
		QString retResponse = s;							//当前位置返回值
		qDebug() << "retResponse = " << retResponse;
		QStringList retlist = retResponse.split(";");
		QString ret1 = retlist.at(1).toLocal8Bit().data();	//获取PX的数值，即串口返回的当前位置值
		int retData = ret1.toInt();							//ret1值转换为整型
		qDebug() << "retData = " << retData;
		PX1 = retData;

		if(PX1 == PX0)										//当两个PX返回值相等，判断是否到达下一组采集位置
		{
			direction_angle = mysetting.start_azAngle+numbercollect*mysetting.step_azAngle;
			int range = direction_angle*800/3;
			if((onecollect_over == true)&&((range-120)<=retData)&&(retData<=(range+120)))//判断单次触发是否完成，串口线程是否运行完毕
			{
				if(mysetting.singleCh)
					singlecollect();
				else
					doublecollect();
			}
		}
		PX0 = PX1;
	}
	else
		if(s.left(2) == "SP")
		{
			QString req_MOPX = "MO;PX;";					//发送串口命令"MO;PX;"
			thread_coll.transaction(portname,req_MOPX);
		}
		else
			if(s.left(2) == "MO")							//接收串口命令"MO;PX;"的返回值
			{
				QString res_MOPX = s;
				QStringList reslist = res_MOPX.split(";");	//获取当前位置
				QString res = reslist.at(3).toLocal8Bit().data();
				PortDialog->show_PX(res);					//在串口对话框中显示当前位置PX
			}
			else
				if(s.left(2) != "PR")
				{
					timer1->stop();							//关闭定时器，并提示串口返回值错误
					stopped = true;							//采集停止
					QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("Value returned by serial port is incorrect"));
				}
}

void MainWindow::receive_portopen()					//串口未正确打开
{
	timer1->stop();									//关闭定时器time1,并提示未能正确打开串口
	stopped = true;									//采集停止
	QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("Serial port can't open"));
}

void MainWindow::receive_timeout()					//接收串口命令超时
{
	timer1->stop();
	stopped = true;
	QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("Receive timeout"));
}

void MainWindow::receive_portdlg(const QString &re)	//接收对话框发送的信息
{
	QString re_need = re;
	if(re_need.left(3) == "COM")
		portname = re_need;
	else
		thread_coll.transaction(portname,re_need);
}

void MainWindow::on_action_stop_triggered()			//采集菜单中的停止键
{
	stopped = true;
}

void MainWindow::singleset()						//单通道参数设置
{
	trig_mode = 3;
	if(ADQ212_SetTriggerMode(adq_cu,1,trig_mode) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("TrigMode"));
		return;
	}
	trig_level = mysetting.triggleLevel;
	if(ADQ212_SetLvlTrigLevel(adq_cu,1,trig_level) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("TrigLevel"));
		return;
	}
	trig_flank = 1;
	if(ADQ212_SetLvlTrigFlank(adq_cu,1,trig_flank) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("TrigFlank"));
		return;
	}
	trig_channel = 1;
	if(ADQ212_SetLvlTrigChannel(adq_cu,1,trig_channel) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("TrigChannel"));
		return;
	}

	qDebug() << "trig_mode = " << trig_mode;		//触发模式
	qDebug() << "clock_source = " << clock_source;
	if(ADQ212_SetClockSource(adq_cu,1,clock_source) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("ClockSource"));
		return;
	}

	int f = mysetting.sampleFreq;
	switch (f) {
	case 550:
		pll_divider = 2;
		break;
	case 367:
		pll_divider = 3;
		break;
	case 275:
		pll_divider = 4;
		break;
	case 220:
		pll_divider = 5;
		break;
	case 183:
		pll_divider = 6;
		break;
	case 157:
		pll_divider = 7;
		break;
	case 138:
		pll_divider = 8;
		break;
	case 122:
		pll_divider = 9;
		break;
	case 110:
		pll_divider = 10;
		break;
	case 100:
		pll_divider = 11;
		break;
	default:
		break;
	}
	if(ADQ212_SetPllFreqDivider(adq_cu,1,pll_divider) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("SampleFreq"));
		return;
	}

	number_of_records = mysetting.plsAccNum;							//脉冲数
	samples_per_record = mysetting.sampleNum;							//采样点数

	if(ADQ212_MultiRecordSetup(adq_cu,1,number_of_records,samples_per_record) == 0)	//设置内存缓冲区
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("SampleNum"));
		return;
	}
	qDebug() << "singlecollect over";
}

void MainWindow::singlecollect()									//单通道采集和存储
{
	onecollect_over = false;										//单次采集开始
	if(direction_angle > 360)
		direction_angle = direction_angle%360;

	dockleft_dlg->set_currentAngle(direction_angle);
	dockleft_dlg->set_groupcnt(numbercollect+1);

	if(ADQ212_DisarmTrigger(adq_cu,1) == 0)							//解除触发，内存计数重置Disarm unit
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("Collect failed1"));
		return;
	}

	QDateTime collectTime = QDateTime::currentDateTime();			//采集开始时间
	timestr = collectTime.toString("yyyy/MM/dd hh:mm:ss");

	if(ADQ212_ArmTrigger(adq_cu,1) == 0)							//Arm unit
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("Collect failed2"));
		return;
	}

	qDebug() << "Please trig your device to collect data.";
	int trigged;
	timer2->start(4000);
	do
	{
		trigged = ADQ212_GetTriggedAll(adq_cu,1);						//Trigger unit
		if(timer2->remainingTime() == 0)
			return;
	}while(trigged == 0);
	timer2->stop();

	qDebug() << "Device trigged.";
	int *data_1_ptr_addr0 = ADQ212_GetPtrDataChA(adq_cu,1);
	qDebug() << "Collecting data,plesase wait...";

	if(trig_HoldOff)
		thread_coll.transaction(portname,request_send);				//采集卡触发完成，驱动器开始转动

	qint16 *rd_data1 = new qint16[samples_per_record*number_of_records];
	for(unsigned int i = 0; i < number_of_records; i++)				//写入采集数据
	{
		int rn1 = 0;
		unsigned int samples_to_collect = samples_per_record;
		while(samples_to_collect > 0)
		{
			int collect_result = ADQ212_CollectRecord(adq_cu,1,i);
			unsigned int samples_in_buffer = qMin(ADQ212_GetSamplesPerPage(adq_cu,1),samples_to_collect);

			if(collect_result)
			{
				for(unsigned int j = 0; j < samples_in_buffer; j++)
				{
					rd_data1[i*samples_per_record + rn1] = *(data_1_ptr_addr0 + j);
					rn1++;
				}
				samples_to_collect -= samples_in_buffer;
			}
			else
			{
				qDebug() << "Collect next data page failed!";
				samples_to_collect = 0;
				i = number_of_records;
			}
		}
	}
	
	if(!threadA.isRunning())
	{
		threadA.fileDataPara(mysetting);					//mysetting值传递给threadstore
		threadA.otherpara(mysetting.dataFileName_Suffix,timestr,direction_angle);	//组数，时间，方位角传递给threadstore
		threadA.s_memcpy(rd_data1);							//采样数据传递给threadstore
		threadA.start();									//启动threadstore线程
	}
	else
		if(!threadB.isRunning())
		{
			threadB.fileDataPara(mysetting);
			threadB.otherpara(mysetting.dataFileName_Suffix,timestr,direction_angle);
			threadB.s_memcpy(rd_data1);
			threadB.start();
		}
		else
			if(!threadC.isRunning())
			{
				threadC.fileDataPara(mysetting);
				threadC.otherpara(mysetting.dataFileName_Suffix,timestr,direction_angle);
				threadC.s_memcpy(rd_data1);
				threadC.start();
			}
			else
			{
				threadD.fileDataPara(mysetting);
				threadD.otherpara(mysetting.dataFileName_Suffix,timestr,direction_angle);
				threadD.s_memcpy(rd_data1);
				threadD.start();
			}

	plotWindow_1->datashow(rd_data1,mysetting.sampleNum,mysetting.plsAccNum);//绘图窗口显示最后一组脉冲
	delete[] rd_data1;

	numbercollect++;												//下一组采集组数
	if((numbercollect >= mysetting.angleNum)||(stopped == true))	//判断是否完成设置组数
		collect_over();
	else
	{
		int filenumber = mysetting.dataFileName_Suffix.toInt();
		int len = mysetting.dataFileName_Suffix.length();
		filenumber++ ;
		if(len<=8)
		{
			mysetting.dataFileName_Suffix.sprintf("%08d", filenumber);
			mysetting.dataFileName_Suffix = mysetting.dataFileName_Suffix.right(len);
		}
		FileName_1 = mysetting.dataFileName_Prefix + "_ch1_" + mysetting.dataFileName_Suffix + ".wld";
		dockleft_dlg->set_filename1(FileName_1);
		dockleft_dlg->set_filename2(NULL);							//更新文件名
	}
	onecollect_over = true;											//单次采集完成
}

void MainWindow::collect_over()										//采集结束处理函数
{
	timer1->stop();
	stopped = true;
	ADQ212_DisarmTrigger(adq_cu,1);
	ADQ212_MultiRecordClose(adq_cu,1);

	ADQ212_GetTrigPoint(adq_cu,1);
	ADQ212_GetTriggedCh(adq_cu,1);
	int overflow = ADQ212_GetOverflow(adq_cu,1);
	if(overflow == 1)
		qDebug() << "Sample overflow in batch.";
	qDebug() << "Collect finished.";
	QMessageBox::information(this,QString::fromLocal8Bit("Information"),QString::fromLocal8Bit("Collect finished."));		//设置界面提示窗口，提示采集完成
	//	DeleteADQControlUnit(adq_cu);
}

void MainWindow::doubleset()										//双通道采集
{
	trig_mode = 2;
	if(ADQ212_SetTriggerMode(adq_cu,1,trig_mode) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("TrigMode"));
		return;
	}

	qDebug() << "trig_mode = " << trig_mode;						//触发模式
	qDebug() << "clock_source = " << clock_source;
	if(ADQ212_SetClockSource(adq_cu,1,clock_source) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("ClockSource"));
		return;
	}
	int f = mysetting.sampleFreq;
	switch (f) {
	case 550:
		pll_divider = 2;
		break;
	case 367:
		pll_divider = 3;
		break;
	case 275:
		pll_divider = 4;
		break;
	case 220:
		pll_divider = 5;
		break;
	case 183:
		pll_divider = 6;
		break;
	case 157:
		pll_divider = 7;
		break;
	case 138:
		pll_divider = 8;
		break;
	case 122:
		pll_divider = 9;
		break;
	case 110:
		pll_divider = 10;
		break;
	case 100:
		pll_divider = 11;
		break;
	default:
		break;
	}
	if(ADQ212_SetPllFreqDivider(adq_cu,1,pll_divider) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("Samplefreq"));
		return;
	}

	number_of_records = mysetting.plsAccNum;
	samples_per_record = mysetting.sampleNum;
	if(ADQ212_SetTriggerHoldOffSamples(adq_cu,1,mysetting.triggerHoldOffSamples) == 0)			//设置触发延迟
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("TriggerHoldOffSamples"));
		return;
	}
	if(ADQ212_MultiRecordSetup(adq_cu,1,number_of_records,samples_per_record) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("SampleNum"));
		return;
	}
}

void MainWindow::doublecollect()									//双通道采集和存储
{
	onecollect_over = false;
	if(direction_angle > 360)
		direction_angle = direction_angle%360;
	dockleft_dlg->set_currentAngle(direction_angle);
	dockleft_dlg->set_groupcnt(numbercollect+1);

	if(ADQ212_DisarmTrigger(adq_cu,1) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("Collect failed"));
		return;
	}

	QDateTime collectTime = QDateTime::currentDateTime();
	timestr = collectTime.toString("yyyy/MM/dd hh:mm:ss");

	if(ADQ212_ArmTrigger(adq_cu,1) == 0)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("Collect failed"));
		return;
	}

	qDebug() << "Please trig your device to collect data.";
	int trigged;
	timer2->start(4000);
	do
	{
		trigged = ADQ212_GetTriggedAll(adq_cu,1);
		if (timer2->remainingTime() == 0)
			return;
	}while(trigged == 0);
	timer2->stop();

	qDebug() << "Device trigged.";

	int *data_a_ptr_addr0 = ADQ212_GetPtrDataChA(adq_cu,1);
	int *data_b_ptr_addr0 = ADQ212_GetPtrDataChB(adq_cu,1);

	qDebug() << "Collecting data,plesase wait...";

	if(trig_HoldOff)
		thread_coll.transaction(portname,request_send);							//采集卡触发完成，驱动器开始转动

	qint16 *rd_dataa = new qint16[samples_per_record*number_of_records];
	qint16 *rd_datab = new qint16[samples_per_record*number_of_records];
	for(unsigned int i = 0; i < number_of_records; i++)
	{
		int rna = 0;
		int rnb = 0;
		unsigned int samples_to_collect = samples_per_record;
		while(samples_to_collect > 0)
		{
			int collect_result = ADQ212_CollectRecord(adq_cu,1,i);
			unsigned int samples_in_buffer = qMin(ADQ212_GetSamplesPerPage(adq_cu,1),samples_to_collect);

			if(collect_result)
			{
				for(unsigned int j = 0; j < samples_in_buffer; j++)
				{
					rd_dataa[i*samples_per_record + rna] = *(data_a_ptr_addr0 + j);
					rd_datab[i*samples_per_record + rnb] = *(data_b_ptr_addr0 + j);
					rna++;
					rnb++;
				}
				samples_to_collect -= samples_in_buffer;
			}
			else
			{
				qDebug() << "Collect next data page failed!";
				samples_to_collect = 0;
				i = number_of_records;
			}
		}
	}

	if(!threadA.isRunning())
	{
		threadA.fileDataPara(mysetting);					//mysetting值传递给threadstore
		threadA.otherpara(mysetting.dataFileName_Suffix,timestr,direction_angle);	//组数，时间，方位角传递给threadstore
		threadA.d_memcpy(rd_dataa,rd_datab);				//采样数据传递给threadstore
		threadA.start();									//启动threadstore线程
	}
	else
		if(!threadB.isRunning())
		{
			threadB.fileDataPara(mysetting);
			threadB.otherpara(mysetting.dataFileName_Suffix,timestr,direction_angle);
			threadB.d_memcpy(rd_dataa,rd_datab);
			threadB.start();
		}
		else
			if(!threadC.isRunning())
			{
				threadC.fileDataPara(mysetting);
				threadC.otherpara(mysetting.dataFileName_Suffix,timestr,direction_angle);
				threadC.d_memcpy(rd_dataa,rd_datab);
				threadC.start();
			}
			else
			{
				threadD.fileDataPara(mysetting);
				threadD.otherpara(mysetting.dataFileName_Suffix,timestr,direction_angle);
				threadD.d_memcpy(rd_dataa,rd_datab);
				threadD.start();
			}
	plotWindow_1->datashow(rd_dataa,mysetting.sampleNum,mysetting.plsAccNum);	//绘图窗口显示a最后一组脉冲
	delete[] rd_dataa;
	plotWindow_2->datashow(rd_datab,mysetting.sampleNum,mysetting.plsAccNum);	//绘图窗口显示b最后一组脉冲
	delete[] rd_datab;

	numbercollect++;
	if((numbercollect >= mysetting.angleNum)||(stopped == true))				//判断是否完成设置组数
		collect_over();
	else
	{
		int filenumber = mysetting.dataFileName_Suffix.toInt();
		int len = mysetting.dataFileName_Suffix.length();
		filenumber++ ;
		if(len<=8)
		{
			mysetting.dataFileName_Suffix.sprintf("%08d", filenumber);
			mysetting.dataFileName_Suffix = mysetting.dataFileName_Suffix.right(len);
		}
		FileName_A = mysetting.dataFileName_Prefix + "_chA_" + mysetting.dataFileName_Suffix + ".wld";
		FileName_B = mysetting.dataFileName_Prefix + "_chB_" + mysetting.dataFileName_Suffix + ".wld";
		dockleft_dlg->set_filename1(FileName_A);
		dockleft_dlg->set_filename2(FileName_B);
	}
	onecollect_over = true;
}

void MainWindow::conncetdevice()									//查找连接ADQ212设备
{
	adq_cu = CreateADQControlUnit();								//用于查找和建立与ADQ设备之间的连接
	qDebug() << "adq_cu = " << adq_cu;
	int n_of_devices = 0;
	int n_of_failed = 0;
	n_of_devices = ADQControlUnit_FindDevices(adq_cu);				//找到所有与电脑连接的ADQ，并创建一个指针列表，返回找到设备的总数
	int n_of_ADQ212 = ADQControlUnit_NofADQ212(adq_cu);				//返回找到ADQ212设备的数量
	n_of_failed = ADQControlUnit_GetFailedDeviceCount(adq_cu);		//返回找到的单元数量

	if(n_of_failed > 0)
		QMessageBox::warning(this,QString::fromLocal8Bit("Warning"),QString::fromLocal8Bit("设备启动失败"));
	if(n_of_devices == 0)
		QMessageBox::information(this,QString::fromLocal8Bit("Warning"),QString::fromLocal8Bit("No ADQ devices found"));

	if(n_of_ADQ212 != 0)
	{
		char *BSN = ADQ212_GetBoardSerialNumber(adq_cu,1);			//设备序列号
		int *Revision = ADQ212_GetRevision(adq_cu,1);				//返回设备的revision
		qDebug() << BSN;
		qDebug() << Revision[0];
		qDebug() << Revision[1];
		qDebug() << Revision[2];									//0-2是FPGA#2(comm)的信息，3-5是FPGA#1(alg)的信息
		qDebug() << Revision[3];									//revision数
		qDebug() << Revision[4];									//0表示SVN Managed，1表示Local Copy
		qDebug() << Revision[5];									//0表示SVN Updated，1表示Mixed Revision
		QMessageBox::information(this,QString::fromLocal8Bit("Information"),QString::fromLocal8Bit("1 ADQ device found."));
	}
}

void MainWindow::search_port()										//搜索串口，确定串口名
{
	QSerialPort my_serial;
	foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
	{
		my_serial.close();
		my_serial.setPortName(info.portName());
		if(!my_serial.open(QIODevice::ReadWrite))
		{
			return;
		}
		my_serial.setBaudRate(QSerialPort::Baud19200);
		my_serial.setDataBits(QSerialPort::Data8);
		my_serial.setStopBits(QSerialPort::OneStop);
		my_serial.setFlowControl(QSerialPort::NoFlowControl);

		QString test("VR;");
		QByteArray testData = test.toLocal8Bit();
		my_serial.write(testData);
		if(my_serial.waitForBytesWritten(20))
		{
			if(my_serial.waitForReadyRead(30))
			{
				QByteArray testResponse = my_serial.readAll();
				while(my_serial.waitForReadyRead(15))
					testResponse += my_serial.readAll();
				QString response(testResponse);
				if(response.left(10) == "VR;Whistle")
				{
					portname = info.portName();
					break;											//确定连接串口后，跳出foreach循环
				}
			}
		}
	}
	my_serial.close();
	qDebug() << "portName = " << portname;
	if(portname == NULL)
		QMessageBox::warning(this,QString::fromLocal8Bit("Error"),QString::fromLocal8Bit("Please connect serialport correctly!"));
}
