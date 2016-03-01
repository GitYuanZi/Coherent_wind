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
#include <QLabel>

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
	QString path = dirs.currentPath()+"/"+"settings.ini";				//获取初始默认路径，并添加默认配置文件
	m_setfile.test_create_file(path);									//检查settings.ini是否存在，若不存在则创建
	m_setfile.readFrom_file(path);										//读取settings.ini文件
	mysetting = m_setfile.get_setting();								//mysetting获取文件中的参数
	initial_plotValue();

	creatleftdock();													//左侧栏
	creatqwtdock();														//曲线栏
	Create_statusbar();													//状态栏

	adq_cu = CreateADQControlUnit();
	conncetADQDevice();													//连接采集卡设备
	timer_trigger_waiting = new QTimer(this);							//用于设定触发等待超时时间，在do—while循环中，如果超时还没有触发，就跳出
	//	timer2->setSingleShot(true);
	connect(timer_trigger_waiting,SIGNAL(timeout()),this,SLOT(notrig_over()));			//建立用于检查do-while的定时器

	PortDialog = new portDialog(this);									//电机设置对话框
	connect(PortDialog,SIGNAL(Motot_connect_status(bool)),this,SLOT(Motot_status(bool)));
	PortDialog->search_set_port(mysetting.SP);							//搜索并设置串口
	connect(PortDialog,SIGNAL(SendPX(float)),this,SLOT(Motor_Position(float)));
	connect(PortDialog,SIGNAL(Position_success()),this,SLOT(Motor_Arrived()));
	connect(PortDialog,SIGNAL(Position_Error()),this,SLOT(set_stop()));
	timer_judge = new QTimer(this);
	connect(timer_judge,SIGNAL(timeout()),this,SLOT(timer_count()));
	connect(timer_judge,SIGNAL(timeout()),this,SLOT(judge_collect_condition()));

	//初始状态值
	using_motor = false;												//单方向探测默认不连接电机
	isPosition_reached = false;
	onecollect_over = true;
	thread_enough = true;
	success_configure = true;
	stopped = true;														//初始状态，未进行数据采集
	num_running = 0;													//运行的数据存储线程数为0

	connect(&threadA, SIGNAL(store_finish()),this,SLOT(receive_storefinish()));
	connect(&threadB, SIGNAL(store_finish()),this,SLOT(receive_storefinish()));
	connect(&threadC, SIGNAL(store_finish()),this,SLOT(receive_storefinish()));
	connect(&threadD, SIGNAL(store_finish()),this,SLOT(receive_storefinish()));
	collect_state->setText(QString::fromLocal8Bit("未进行采集"));
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::initial_plotValue()
{
	m_paraValue.hide_grid = false;
	m_paraValue.showA = true;
	m_paraValue.showB = true;
	m_paraValue.countNum = false;
	m_paraValue.echoDistance = true;
}

//创建左侧显示栏
void MainWindow::creatleftdock(void)
{
	dockleft_dlg = new informationleft(this);
	dockWidget = new QDockWidget;
	dockWidget->setWidget(dockleft_dlg);
	dockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
	addDockWidget(Qt::LeftDockWidgetArea, dockWidget);

	dockleft_dlg->set_currentAngle(mysetting.start_azAngle);
	dockleft_dlg->set_groupNum(mysetting.angleNum);
	dockleft_dlg->set_groupcnt(0);

	if(mysetting.isSingleCh)
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

//创建曲线显示部分
void MainWindow::creatqwtdock(void)
{
	plot1show = false;
	plot2show = false;
	if(m_paraValue.showA)
	{
		plot1show = true;
		plotWindow_1 = new PlotWindow(this);
		dockqwt_1 = new QDockWidget;
		dockqwt_1->setWidget(plotWindow_1);
		dockqwt_1->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
		addDockWidget(Qt::RightDockWidgetArea,dockqwt_1);
		plotWindow_1->setMaxX(mysetting.sampleNum,mysetting.sampleFreq,m_paraValue.countNum);
		connect(dockqwt_1,&QDockWidget::topLevelChanged,this,&MainWindow::dockview_ct1);
		if(mysetting.isSingleCh)
			plotWindow_1->set_titleName("CH1");
		else
			plotWindow_1->set_titleName("CHA");
		plotWindow_1->set_grid(m_paraValue.hide_grid);
	}

	if((!mysetting.isSingleCh)&&m_paraValue.showB)
	{
		plot2show = true;
		plotWindow_2 = new PlotWindow(this);				//创建plotWindow_2的图形区域
		dockqwt_2 = new QDockWidget;
		dockqwt_2->setWidget(plotWindow_2);
		dockqwt_2->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
		addDockWidget(Qt::RightDockWidgetArea,dockqwt_2);
		plotWindow_2->setMaxX(mysetting.sampleNum,mysetting.sampleFreq,m_paraValue.countNum);
		connect(dockqwt_2,&QDockWidget::topLevelChanged,this,&MainWindow::dockview_ct2);
		plotWindow_2->set_titleName("CHB");					//双通道B的名称
		plotWindow_2->set_grid(m_paraValue.hide_grid);
	}
	setPlotWindowVisible();
}

//创建状态栏
void MainWindow::Create_statusbar()
{
	bar = ui->statusBar;											//获取状态栏
	QFont font("Microsoft YaHei UI",9);								//设置状态栏字体大小
	bar->setFont(font);

	ADQ_state = new QLabel;											//新建标签
	ADQ_state->setMinimumSize(115,22);								//设置标签最小尺寸
	ADQ_state->setAlignment(Qt::AlignLeft);							//设置对齐方式，左侧对齐
	bar->addWidget(ADQ_state);

	motor_state =new QLabel;
	motor_state->setMinimumSize(95,22);
	motor_state->setAlignment(Qt::AlignLeft);
	bar->addWidget(motor_state);

	collect_state = new QLabel;
	collect_state->setMinimumSize(85,22);
	collect_state->setAlignment(Qt::AlignLeft);
	bar->addWidget(collect_state);

	storenum = new QLabel;
	storenum->setMinimumSize(965,22);
	storenum->setAlignment(Qt::AlignLeft);
	bar->addWidget(storenum);
}

//查找并连接ADQ212设备
void MainWindow::conncetADQDevice()
{
	int n_of_devices = ADQControlUnit_FindDevices(adq_cu);			//找到所有与电脑连接的ADQ，并创建一个指针列表，返回找到设备的总数
	int n_of_failed = ADQControlUnit_GetFailedDeviceCount(adq_cu);
	int n_of_ADQ212 = ADQControlUnit_NofADQ212(adq_cu);				//返回找到ADQ212设备的数量
	if((n_of_failed > 0)||(n_of_devices == 0))
	{
		ADQ_state->setText(QString::fromLocal8Bit("采集卡未连接"));
		return;
	}
	if(n_of_ADQ212 != 0)
		ADQ_state->setText(QString::fromLocal8Bit("采集卡已连接"));
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

void MainWindow::setPlotWindowVisible()						//设置绘图窗口的尺寸
{
	int w = width();
	int h = height();
	w = w - 265, h = (h-25-42-22-25)/2;						//高度：菜单25，工具42，状态22，左侧边栏宽度260，再空去25的高度
	if(m_paraValue.showA)
	{
		plotWindow_1->setFixedSize(w,h);
		plotWindow_1->setMaximumSize(1920,1080);			//绘图窗口的最大尺寸
	}

	if((!mysetting.isSingleCh)&&m_paraValue.showB)
	{
		plotWindow_2->setFixedSize(w,h);
		plotWindow_2->setMaximumSize(1920,1080);
	}
}

void MainWindow::resizeEvent(QResizeEvent *event)			//主窗口大小改变时，保证绘图窗口填满
{
	setPlotWindowVisible();
}

//连接USB采集卡
void MainWindow::on_action_searchDevice_triggered()
{
	conncetADQDevice();
}

//打开数据存储路径
void MainWindow::on_action_open_triggered()
{
	if(mysetting.DatafilePath.isEmpty())					//路径为空时，打开默认路径
	{
		QDir default_path;
		QString storepath = default_path.currentPath();
		QDesktopServices::openUrl(QUrl::fromLocalFile(storepath));
	}
	else													//路径存在时，打开对应的指定路径
	{
		Create_DataFolder();
		QDesktopServices::openUrl(QUrl::fromLocalFile(mysetting.DatafilePath));
	}
}

//打开参数设置对话框
void MainWindow::on_action_set_triggered()
{
	QCoreApplication::processEvents();
	ParaSetDlg = new paraDialog(this);
	ParaSetDlg->init_setting(mysetting,stopped);					//mysetting传递给设置窗口psetting
	ParaSetDlg->initial_para();										//参数显示在设置窗口上，并连接槽
	ParaSetDlg->on_checkBox_autocreate_datafile_clicked();			//更新文件存储路径
	if (ParaSetDlg->exec() == QDialog::Accepted)					// 确定键功能
	{
		mysetting =	ParaSetDlg->get_settings();						//mysetting获取修改后的参数
		dockleft_dlg->set_currentAngle(mysetting.start_azAngle);	//更新左侧栏
		dockleft_dlg->set_groupNum(mysetting.angleNum);
		dockleft_dlg->set_groupcnt(0);
		if(mysetting.isSingleCh)
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
		refresh();					//更新绘图窗口
	}
	delete ParaSetDlg;				//防止内存泄漏
}

//参数设置对话框关闭后，对绘图曲线部分进行更新
void MainWindow::refresh()
{
	if(plot1show)
	{
		delete plotWindow_1;
		delete dockqwt_1;
	}
	if(plot2show)
	{
		delete plotWindow_2;
		delete dockqwt_2;
	}
	creatqwtdock();
}

//打开电机的串口控制对话框
void MainWindow::on_action_serialport_triggered()
{
	PortDialog->initial_data(using_motor,mysetting.step_azAngle,stopped);
	if(PortDialog->exec() == QDialog::Accepted)		//从串口对话框接收连接电机bool值
		using_motor = PortDialog->get_returnMotor_connect();
}

//打开绘图设置对话框
void MainWindow::on_action_view_triggered()
{
	PlotDialog = new plotDialog(this);				//绘图设置对话框
	PlotDialog->dialog_show(m_paraValue,mysetting.isSingleCh);
	if(PlotDialog->exec() == QDialog::Accepted)
	{
		m_paraValue = PlotDialog->get_settings();
		refresh();
	}
	delete PlotDialog;
}

//采集菜单中的开始按钮
void MainWindow::on_action_start_triggered()
{
	if((num_running == 4)||(stopped == false))		//检查存储线程是否完成数据存储
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("数据存储尚未完成"));
		return;
	}
	if(mysetting.angleNum == 0)						//方向数为0时，不采集
		return;

	n_sample_skip = 1;					//采样间隔设为1，表示无采样间隔
	if(ADQ212_SetSampleSkip(adq_cu,1,n_sample_skip) == 0)
	{
		collect_state->setText(QString::fromLocal8Bit("采集停止"));
		QMessageBox::warning(this,QString::fromLocal8Bit("错误"),QString::fromLocal8Bit("采集卡连接异常"));
		return;
	}

	success_configure = adq_para_set();	//设置采集卡参数
	if(success_configure == true)		//采集卡配置成功
	{
		direction_intervalNum = mysetting.direction_intervalTime * FREQUENCY_OF_JUDGE;
		dI_timer_counter = direction_intervalNum;		//为了第一次？

		Create_DataFolder();			//创建数据存储文件夹
		num_collect = 0;
		stopped = false;				//stopped设置为false
		notrig_signal = false;
		if((mysetting.step_azAngle == 0)&&(using_motor == false))	//径向不连电机采集
		{
			isPosition_reached = true;
			timer_judge->start(PERIOD_OF_JUDGE);
		}
		else														//连接电机采集（径向OR扫描）
		{
			if(mysetting.step_azAngle != 0)
			{
				circle_intervalNum = mysetting.time_circle_interval*FREQUENCY_OF_JUDGE_PERMIN;
				cI_timer_counter = circle_intervalNum;
//				cI_timer_counter = 0;
				Num_perRound = 360/mysetting.step_azAngle;
			}
			collect_state->setText(QString::fromLocal8Bit("电机位置调整..."));
			timer_judge->start(PERIOD_OF_JUDGE);
			PortDialog->ABS_Rotate(mysetting.start_azAngle);
		}
	}
	else											//采集卡配置失败
	{
		success_configure = true;	//??
		ADQ212_DisarmTrigger(adq_cu,1);
		ADQ212_MultiRecordClose(adq_cu,1);
		QMessageBox::warning(this,QString::fromLocal8Bit("错误"),QString::fromLocal8Bit("采集卡设置失败"));
		return;
	}
}

//采集菜单中的停止按钮
void MainWindow::on_action_stop_triggered()
{
	set_stop();
}

//数据存储文件夹的创建
void MainWindow::Create_DataFolder()
{
	QDir mypath;
	if(!mypath.exists(mysetting.DatafilePath))		//如果文件夹不存在，创建文件夹
		mypath.mkpath(mysetting.DatafilePath);
}

//方向间、圆周间间隔计数
void MainWindow::timer_count()
{
	dI_timer_counter++;
	cI_timer_counter++;
}

//定时判断是否进行下一组采集
void MainWindow::judge_collect_condition()
{
	QCoreApplication::processEvents();
	if((dI_timer_counter >= direction_intervalNum)
			&&(isPosition_reached == true)&&(onecollect_over == true))
	{
		//扫描探测时
		if(mysetting.step_azAngle != 0)
		{
			if(num_collect%Num_perRound == 0)				//即将转过一圈
			{
				if(stopped == true)
				{
					timer_judge->stop();
					onecollect_over = true;
					collect_reset();
					collect_state->setText(QString::fromLocal8Bit("采集停止"));
					QMessageBox::information(this,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("采集已停止"));
					return;
				}
				if(cI_timer_counter >= circle_intervalNum)	//达到圆周间隔时间
				{
					isPosition_reached = false;
					cI_timer_counter = 0;
				}
				else
					return;
			}
			else										//一圈内的采集正常进行
				isPosition_reached = false;
		}

		onecollect_over = false;						//单次采集开始
		dI_timer_counter = 0;							//判断次数清零
		if(stopped == false)
		{
			collect_state->setText(QString::fromLocal8Bit("数据采集中..."));
			success_configure = adq_collect();
			if(notrig_signal)
				return;

			if(success_configure == true)				//采集卡设置成功
			{
				if(mysetting.isSingleCh)				//数据上传并存储
					single_upload_store();
				else
					double_upload_store();

				if(success_configure == true)
				{
					if(thread_enough == true)
					{
						collect_state->setText(QString::fromLocal8Bit("数据上传成功..."));
						update_collect_number();		//更新当前采集信息
						//判断是否完成设置组数
						if((num_collect >= mysetting.angleNum)||(stopped == true))
							collect_over();
						onecollect_over = true;
					}
					else
					{
						timer_judge->stop();
						onecollect_over = true;
						thread_enough = true;
						collect_reset();
						collect_state->setText(QString::fromLocal8Bit("采集停止"));
						QMessageBox::warning(this,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("单文件数据量大，存储较慢，请适当降低电机转速"));
					}
				}
				else
				{
					timer_judge->stop();
					onecollect_over = true;
					success_configure = true;
					collect_reset();
					collect_state->setText(QString::fromLocal8Bit("采集停止"));
					QMessageBox::warning(this,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("采集卡出现故障"));
				}
			}
			else
			{
				timer_judge->stop();
				onecollect_over = true;
				success_configure = true;
				collect_reset();
				collect_state->setText(QString::fromLocal8Bit("采集停止"));
				QMessageBox::warning(this,QString::fromLocal8Bit("错误"),QString::fromLocal8Bit("采集卡设置失败"));
			}
		}
		else
		{
			timer_judge->stop();
			onecollect_over = true;
			collect_reset();
			collect_state->setText(QString::fromLocal8Bit("采集停止"));
			QMessageBox::information(this,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("采集已停止"));
		}
	}
}

//更新角度、Dial显示
void MainWindow::Motor_Position(float a)
{
	PX_lastData = a;
	dockleft_dlg->set_currentAngle(a);
}

//更新状态栏中电机连接状态
void MainWindow::Motot_status(bool a)
{
	if(a == true)
		motor_state->setText(QString::fromLocal8Bit("电机已打开"));
	else
		motor_state->setText(QString::fromLocal8Bit("电机打开失败"));
}

//电机到达预采集位置
void MainWindow::Motor_Arrived()
{
	isPosition_reached = true;
}

void MainWindow::set_stop()
{
	stopped = true;
}

//采集卡参数设置
bool MainWindow::adq_para_set()
{
	int trig_mode = mysetting.trigger_mode;
	if(ADQ212_SetTriggerMode(adq_cu,1,trig_mode) == 0)
		return false;
	if(trig_mode == 3)							//电平触发
	{
		qint16 trig_level = mysetting.trigLevel;
		if(ADQ212_SetLvlTrigLevel(adq_cu,1,trig_level) == 0)
			return false;
		int trig_flank = 1;						//触发边沿 -> 上升沿
		if(ADQ212_SetLvlTrigFlank(adq_cu,1,trig_flank) == 0)
			return false;
		int trig_channel = 1;					//触发通道:1通道A,2通道B
		if(ADQ212_SetLvlTrigChannel(adq_cu,1,trig_channel) == 0)
			return false;
	}
	int clock_source = 0;						//时钟源选择0，内部时钟，内部参考
	if(ADQ212_SetClockSource(adq_cu,1,clock_source) == 0)
		return false;
	int pll_divider = int(1100/mysetting.sampleFreq);
	if(ADQ212_SetPllFreqDivider(adq_cu,1,pll_divider) == 0)
		return false;
	number_of_records = mysetting.plsAccNum;	//脉冲数
	samples_per_record = mysetting.sampleNum;	//采样点数
	if(mysetting.trigger_mode == 2)				//外部触发时，设置触发延迟
	{
		if(ADQ212_SetTriggerHoldOffSamples(adq_cu,1,mysetting.trigHoldOffSamples) == 0)
			return false;
	}
	if(ADQ212_MultiRecordSetup(adq_cu,1,number_of_records,samples_per_record) == 0)
		return false;
	return true;
}

//采集卡触发进行采集、并更新左侧采集文件和方位信息
bool MainWindow::adq_collect()
{
	if(ADQ212_DisarmTrigger(adq_cu,1) == 0)			//Disarm unit
		return false;
	if(ADQ212_ArmTrigger(adq_cu,1) == 0)			//Arm unit
		return false;

	dockleft_dlg->set_groupcnt(num_collect+1);					//进度条更新
	if(mysetting.isSingleCh)										//单通道文件名更新
		dockleft_dlg->set_filename1(FileName_1);
	else															//双通道文件名更新
	{
		dockleft_dlg->set_filename1(FileName_A);
		dockleft_dlg->set_filename2(FileName_B);
	}

	QDateTime collectTime = QDateTime::currentDateTime();			//采集开始时间
	timestr = collectTime.toString("yyyy/MM/dd hh:mm:ss");
	qDebug() << "Please trig your device to collect data.";
	int trigged;
	timer_trigger_waiting->start(TRIGGER_WAIT_TIME);
	do
	{
		QCoreApplication::processEvents();
		trigged = ADQ212_GetTriggedAll(adq_cu,1);					//Trigger unit
		if(timer_trigger_waiting->remainingTime() == 0)
		{
			notrig_signal = true;
			return false;
		}
	}while(trigged == 0);
	timer_trigger_waiting->stop();
	qDebug() << "Device trigged.";

	if((mysetting.step_azAngle != 0)&&((num_collect+1)< mysetting.angleNum)&&(stopped == false))
		PortDialog->CW_Rotate(mysetting.step_azAngle);
	return true;
}

//单通道数据上传、存储和显示
void MainWindow::single_upload_store()
{
	qDebug() << "Collecting data,plesase wait...";
	int *data_1_ptr_addr0 = ADQ212_GetPtrDataChA(adq_cu,1);
	collect_state->setText(QString::fromLocal8Bit("数据上传中..."));
	qint16 *rd_data1 = new qint16[samples_per_record*number_of_records];
	for(unsigned int i = 0; i < number_of_records; i++)				//写入采集数据
	{
		int rn1 = 0;
		unsigned int samples_to_collect = samples_per_record;
		while(samples_to_collect > 0)
		{
			QCoreApplication::processEvents();
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
				delete[] rd_data1;
				success_configure = false;
				return;
			}
		}
	}

	if(plot1show)
		plotWindow_1->datashow(rd_data1,mysetting.sampleNum,mysetting.plsAccNum);//绘图窗口显示最后一组脉冲
	if(!threadA.isRunning())
	{
		num_running++;										//线程数加1，状态栏显示线程数
		storenum->setText(QString::fromLocal8Bit("正在运行的线程数为")+QString::number(num_running));
		threadA.fileDataPara(mysetting);					//mysetting值传递给threadstore
		threadA.otherpara(timestr,direction_angle);			//时间，方位角传递给threadstore
		threadA.s_memcpy(rd_data1);							//采样数据传递给threadstore
		threadA.start();									//启动threadstore线程
	}
	else
		if(!threadB.isRunning())
		{
			num_running++;
			storenum->setText(QString::fromLocal8Bit("正在运行的线程数为")+QString::number(num_running));
			threadB.fileDataPara(mysetting);
			threadB.otherpara(timestr,direction_angle);
			threadB.s_memcpy(rd_data1);
			threadB.start();
		}
		else
			if(!threadC.isRunning())
			{
				num_running++;
				storenum->setText(QString::fromLocal8Bit("正在运行的线程数为")+QString::number(num_running));
				threadC.fileDataPara(mysetting);
				threadC.otherpara(timestr,direction_angle);
				threadC.s_memcpy(rd_data1);
				threadC.start();
			}
			else
				if(!threadD.isRunning())
				{
					num_running++;
					storenum->setText(QString::fromLocal8Bit("正在运行的线程数为")+QString::number(num_running));
					threadD.fileDataPara(mysetting);
					threadD.otherpara(timestr,direction_angle);
					threadD.s_memcpy(rd_data1);
					threadD.start();
				}
				else										//四个线程都在运行时，停止采集
				{
					delete[] rd_data1;
					thread_enough = false;
					return;
				}

	delete[] rd_data1;
}

//双通道数据上传和存储
void MainWindow::double_upload_store()
{
	int *data_a_ptr_addr0 = ADQ212_GetPtrDataChA(adq_cu,1);
	int *data_b_ptr_addr0 = ADQ212_GetPtrDataChB(adq_cu,1);
	qDebug() << "Collecting data,plesase wait...";
	collect_state->setText(QString::fromLocal8Bit("数据上传中..."));
	qint16 *rd_dataa = new qint16[samples_per_record*number_of_records];
	qint16 *rd_datab = new qint16[samples_per_record*number_of_records];
	for(unsigned int i = 0; i < number_of_records; i++)
	{
		int rna = 0;	//采样点数计数值，每次读1个page，计数值一直增加
		int rnb = 0;
		unsigned int samples_to_collect = samples_per_record;
		while(samples_to_collect > 0)
		{
			QCoreApplication::processEvents();
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
				delete[] rd_dataa;
				delete[] rd_datab;
				success_configure = false;
				return;
			}
		}
	}

	if(plot1show)
		plotWindow_1->datashow(rd_dataa,mysetting.sampleNum,mysetting.plsAccNum);	//绘图窗口显示cha最后一组脉冲
	if(plot2show)
		plotWindow_2->datashow(rd_datab,mysetting.sampleNum,mysetting.plsAccNum);	//绘图窗口显示chb最后一组脉冲
	if(!threadA.isRunning())
	{
		num_running++;
		storenum->setText(QString::fromLocal8Bit("正在运行的线程数为") + QString::number(num_running));
		threadA.fileDataPara(mysetting);					//mysetting值传递给threadstore
		threadA.otherpara(timestr,direction_angle);			//组数，时间，方位角传递给threadstore
		threadA.d_memcpy(rd_dataa,rd_datab);				//采样数据传递给threadstore
		threadA.start();									//启动threadstore线程
	}
	else
		if(!threadB.isRunning())
		{
			num_running++;
			storenum->setText(QString::fromLocal8Bit("正在运行的线程数为")+QString::number(num_running));
			threadB.fileDataPara(mysetting);
			threadB.otherpara(timestr,direction_angle);
			threadB.d_memcpy(rd_dataa,rd_datab);
			threadB.start();
		}
		else
			if(!threadC.isRunning())
			{
				num_running++;
				storenum->setText(QString::fromLocal8Bit("正在运行的线程数为")+QString::number(num_running));
				threadC.fileDataPara(mysetting);
				threadC.otherpara(timestr,direction_angle);
				threadC.d_memcpy(rd_dataa,rd_datab);
				threadC.start();
			}
			else
				if(!threadD.isRunning())
				{
					num_running++;
					storenum->setText(QString::fromLocal8Bit("正在运行的线程数为")+QString::number(num_running));
					threadD.fileDataPara(mysetting);
					threadD.otherpara(timestr,direction_angle);
					threadD.d_memcpy(rd_dataa,rd_datab);
					threadD.start();
				}
				else	//四个线程都在运行时，停止采集
				{
					delete[] rd_dataa;
					delete[] rd_datab;
					thread_enough = false;
					return;
				}

	delete[] rd_dataa;
	delete[] rd_datab;
}

//采集数据上传和存储完成后，更新当前采集文件、序号信息
void MainWindow::update_collect_number()
{
	int filenumber = mysetting.dataFileName_Suffix.toInt();
	int len = mysetting.dataFileName_Suffix.length();
	filenumber++ ;
	if(len<=8)
	{
		mysetting.dataFileName_Suffix.sprintf("%08d", filenumber);
		mysetting.dataFileName_Suffix = mysetting.dataFileName_Suffix.right(len);
	}
	if(mysetting.isSingleCh)
		FileName_1 = mysetting.dataFileName_Prefix + "_ch1_" + mysetting.dataFileName_Suffix + ".wld";
	else
	{
		FileName_A = mysetting.dataFileName_Prefix + "_chA_" + mysetting.dataFileName_Suffix + ".wld";
		FileName_B = mysetting.dataFileName_Prefix + "_chB_" + mysetting.dataFileName_Suffix + ".wld";
	}
	num_collect++;		//下一组采集组数
}

//采集卡未检测到外部触发信号
void MainWindow::notrig_over()
{
	timer_judge->stop();
	timer_trigger_waiting->stop();
	collect_reset();
	onecollect_over = true;
	collect_state->setText(QString::fromLocal8Bit("采集停止"));
	QMessageBox::information(this,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("采集卡未接收到触发信号"));
}

//采集结束
void MainWindow::collect_over()
{
	timer_judge->stop();
	collect_reset();
	collect_state->setText(QString::fromLocal8Bit("采集完成"));
	QMessageBox::information(this,QString::fromLocal8Bit("信息"),QString::fromLocal8Bit("采集结束"));
}

//采集停止或结束时更新电机位置、采集卡信息
void MainWindow::collect_reset()
{
	if((PX_lastData>=360)&&(mysetting.step_azAngle != 0))
	{
		PX_lastData = PX_lastData%360;
		PortDialog->SetPX(PX_lastData);
	}
	stopped = true;
	ADQ212_DisarmTrigger(adq_cu,1);
	ADQ212_MultiRecordClose(adq_cu,1);
}

//程序关闭时，检查存储线程是否完成数据存储
void MainWindow::closeEvent(QCloseEvent *event)
{
	if((num_running != 0)||(stopped == false))
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("数据存储尚未完成"));
		event->ignore();
	}
	else
	{
		DeleteADQControlUnit(adq_cu);
		event->accept();
	}
}

//线程存储完成，线程数减1
void MainWindow::receive_storefinish()
{
	num_running--;
	storenum->setText(QString::fromLocal8Bit("正在运行的线程数为")+QString::number(num_running));
}
