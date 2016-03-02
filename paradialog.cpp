#include "paradialog.h"
#include "ui_paradialog.h"
#include "Acqsettings.h"
#include "Windows.h"

#include <QDebug>
#include <QFileDialog>
#include <QSettings>
#include <QDateTime>
#include <QMessageBox>
#include <QString>
#include <QDir>

#define c 3														//光速

paraDialog::paraDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::paraDialog)
{
	ui->setupUi(this);
}

paraDialog::~paraDialog()
{
	delete ui;
}

void paraDialog::init_setting(const ACQSETTING &setting, bool sop)
{
	psetting = setting;
	defaulsetting = setting;
	dlg_setfile.init_fsetting(psetting);						//把psetting传递给fsetting
	nocollecting = sop;											//程序未进行采集
}

void paraDialog::initial_para()
{
	update_show();

	connect(ui->lineEdit_laserRPF,&QLineEdit::textChanged,this,&paraDialog::set_laserRPF);						//激光重频
	connect(ui->lineEdit_laserPulseWidth,&QLineEdit::textChanged,this,&paraDialog::set_laserPulseWidth);		//激光脉宽
	connect(ui->lineEdit_laserWaveLength,&QLineEdit::textChanged,this,&paraDialog::set_laserWaveLength);		//激光波长
	connect(ui->lineEdit_AOM_Freq,&QLineEdit::textChanged,this,&paraDialog::set_AOM_Freq);						//AOM移频量

	connect(ui->lineEdit_elevationAngle,&QLineEdit::textChanged,this,&paraDialog::set_elevationAngle);			//俯仰角 -> 探测方式
	connect(ui->lineEdit_start_azAngle,&QLineEdit::textChanged,this,&paraDialog::set_start_azAngle);			//起始角
	connect(ui->lineEdit_step_azAngle,&QLineEdit::textChanged,this,&paraDialog::set_step_azAngle);				//步进角 -> 探测方式
	connect(ui->lineEdit_step_azAngle,&QLineEdit::textChanged,this,&paraDialog::set_SP_Interval);				//步进角 -> 速度和圆周间间隔使能状态
	connect(ui->lineEdit_step_azAngle,&QLineEdit::textChanged,this,&paraDialog::set_circleNum);					//步进角 -> 圆周数
	connect(ui->lineEdit_step_azAngle,&QLineEdit::textChanged,this,&paraDialog::set_angleNum);					//步进角 -> 方向数
	connect(ui->lineEdit_angleNum,&QLineEdit::textChanged,this,&paraDialog::set_circleNum);						//方向数 -> 圆周数
	connect(ui->lineEdit_circleNum, &QLineEdit::textChanged,this,&paraDialog::set_angleNum);					//圆周数 -> 方向数
	connect(ui->radioButton_anglekey,&QRadioButton::clicked,this,&paraDialog::set_anglekey);					//方向键
	connect(ui->radioButton_circlekey,&QRadioButton::clicked,this,&paraDialog::set_circlekey);					//圆周键
	connect(ui->lineEdit_SP,&QLineEdit::textChanged,this,&paraDialog::set_motorSP);								//最高转速 -> 探测时间

	connect(ui->radioButton_singleCh,&QRadioButton::clicked,this,&paraDialog::set_singleCh);					//单通道
	connect(ui->radioButton_doubleCh,&QRadioButton::clicked,this,&paraDialog::set_doubleCh);					//双通道
	connect(ui->comboBox_trig_mode,&QComboBox::currentTextChanged,this,&paraDialog::set_trig_mode);
	connect(ui->lineEdit_trigLevel_OR_holdOff,&QLineEdit::textChanged,this,&paraDialog::set_trigLevel_OR_holdOff);//触发电平、触发延迟
	connect(ui->lineEdit_direct_interval,&QLineEdit::textChanged,this,&paraDialog::set_time_direct_interval);
	connect(ui->lineEdit_circle_interval,&QLineEdit::textChanged,this,&paraDialog::set_time_circle_interval);
	connect(ui->comboBox_sampleFreq,&QComboBox::currentTextChanged,this,&paraDialog::set_sampleFreq);			//采样频率 -> 采样点数
	connect(ui->lineEdit_detRange,&QLineEdit::textChanged,this,&paraDialog::set_detRange);						//探测距离 -> 采样点数
	connect(ui->lineEdit_sampleNum,&QLineEdit::textChanged,this,&paraDialog::set_filesize);						//采样点数 -> 数据量
	connect(ui->lineEdit_plsAccNum,&QLineEdit::textChanged,this,&paraDialog::set_plsAccNum);					//脉冲数	  -> 数据量

	connect(ui->lineEdit_dataFileName_Suffix,&QLineEdit::textChanged,this,&paraDialog::set_dataFileName_Suffix);//后缀名
	connect(ui->checkBox_channelA,&QCheckBox::clicked,this,&paraDialog::set_channelA);							//通道A
	connect(ui->checkBox_channelB,&QCheckBox::clicked,this,&paraDialog::set_channelB);							//通道B
}

void paraDialog::update_show()
{
	//激光参数
	ui->lineEdit_laserRPF->setText(QString::number(psetting.laserRPF));
	ui->lineEdit_laserPulseWidth->setText(QString::number(psetting.laserPulseWidth));
	ui->lineEdit_laserWaveLength->setText(QString::number(psetting.laserWaveLength));
	ui->lineEdit_AOM_Freq->setText(QString::number(psetting.AOM_Freq));

	//扫描参数
	ui->lineEdit_elevationAngle->setText(QString::number(psetting.elevationAngle));
	ui->lineEdit_start_azAngle->setText(QString::number(psetting.start_azAngle));
	ui->lineEdit_step_azAngle->setText(QString::number(psetting.step_azAngle));
	ui->groupBox_2->setEnabled(true);
	ui->radioButton_anglekey->setChecked(psetting.anglekey);
	ui->radioButton_circlekey->setChecked(psetting.circlekey);
	if(psetting.anglekey)
	{
		ui->lineEdit_circleNum->setEnabled(false);
		ui->lineEdit_angleNum->setEnabled(true);
	}
	else
	{
		ui->lineEdit_circleNum->setEnabled(true);
		ui->lineEdit_angleNum->setEnabled(false);
	}
	ui->lineEdit_angleNum->setText(QString::number(psetting.angleNum));
	ui->lineEdit_circleNum->setText(QString::number(psetting.circleNum));
	ui->lineEdit_SP->setText(QString::number(psetting.SP));

	//采样配置——采集模式
	ui->radioButton_singleCh->setChecked(psetting.isSingleCh);
	ui->radioButton_doubleCh->setChecked(!psetting.isSingleCh);
	//采样配置——触发设置
	trig_conversion = false;
	if(psetting.trigger_mode == 3)
	{
		ui->comboBox_trig_mode->setCurrentText(QString::fromLocal8Bit("电平触发"));
		ui->label_level_OR_holdOff->setText(QString::fromLocal8Bit("触发电平"));
		ui->lineEdit_trigLevel_OR_holdOff->setText(QString::number(psetting.trigLevel));
		ui->pushButton_conversion->setText(QString::fromLocal8Bit("LSB"));
	}
	else
	{
		ui->comboBox_trig_mode->setCurrentText(QString::fromLocal8Bit("外部触发"));
		ui->label_level_OR_holdOff->setText(QString::fromLocal8Bit("触发延迟"));
		ui->lineEdit_trigLevel_OR_holdOff->setText(QString::number(psetting.trigHoldOffSamples));
		ui->pushButton_conversion->setText(QString::fromLocal8Bit("点"));
	}
	//采样配置——定时设置
	ui->lineEdit_direct_interval->setText(QString::number(psetting.direction_intervalTime,'f',2));
	ui->lineEdit_circle_interval->setText(QString::number(psetting.time_circle_interval,'f',2));
	//采样配置——采样参数
	ui->comboBox_sampleFreq->setCurrentText(QString::number(psetting.sampleFreq));
	ui->lineEdit_detRange->setText(QString::number(psetting.detRange/1000));
	ui->lineEdit_sampleNum->setText(QString::number(psetting.sampleNum));
	ui->lineEdit_plsAccNum->setText(QString::number(psetting.plsAccNum));

	//文件存储
	ui->lineEdit_DatafilePath->setText(psetting.DatafilePath);
	ui->checkBox_autocreate_datafile->setChecked(psetting.autocreate_datafile);
	if(psetting.isSingleCh)
	{
		ui->checkBox_channelA->setChecked(false);
		ui->checkBox_channelB->setChecked(false);
		ui->checkBox_channelA->setEnabled(false);
		ui->checkBox_channelB->setEnabled(false);
		update_s_filename();
	}
	else
	{
		ui->checkBox_channelA->setChecked(psetting.channel_A);
		ui->checkBox_channelB->setChecked(psetting.channel_B);
		ui->checkBox_channelA->setEnabled(true);
		ui->checkBox_channelB->setEnabled(true);
		update_d_filename();
	}
	ui->lineEdit_dataFileName_Prefix->setText(psetting.dataFileName_Prefix);
	on_pushButton_dataFileName_sch_clicked();												//更新最小序号，即后缀

	//下方参考信息——探测方式
	show_detect_mode();
	//下方参考信息——预估探测时间
	set_dect_time();
	//下方参考信息——更新文件数据量
	direct_size = 68+psetting.plsAccNum*psetting.sampleNum*2;								//单位B
	if(psetting.isSingleCh)
		single_filesize();
	else
		double_filesize();
}

void paraDialog::update_s_filename()
{
	ui->lineEdit_dataFileName_ch1->setText(psetting.dataFileName_Prefix + "_ch1_" + psetting.dataFileName_Suffix + ".wld");
	ui->lineEdit_dataFileName_chA->setText(NULL);
	ui->lineEdit_dataFileName_chB->setText(NULL);
}

void paraDialog::update_d_filename()
{
	ui->lineEdit_dataFileName_ch1->setText(NULL);
	ui->lineEdit_dataFileName_chA->setText(psetting.dataFileName_Prefix + "_chA_" + psetting.dataFileName_Suffix + ".wld");
	ui->lineEdit_dataFileName_chB->setText(psetting.dataFileName_Prefix + "_chB_" + psetting.dataFileName_Suffix + ".wld");
}

void paraDialog::show_detect_mode()
{
	if(psetting.elevationAngle == 0)
	{
		if(psetting.step_azAngle == 0)
			ui->lineEdit_detectDir->setText(QString::fromLocal8Bit("水平单向探测"));
		else
			ui->lineEdit_detectDir->setText(QString::fromLocal8Bit("水平扫描探测，每周方向数")+QString::number(360/psetting.step_azAngle));
	}
	else
		if(psetting.step_azAngle == 0)
			ui->lineEdit_detectDir->setText(QString::fromLocal8Bit("单向径向探测"));
		else
			ui->lineEdit_detectDir->setText(QString::fromLocal8Bit("圆锥扫描探测，每周方向数")+QString::number(360/psetting.step_azAngle));
}

//探测时间=电机转动时间+触发时间+上传时间+方向间间隔时间+圆周间间隔时间
void paraDialog::set_dect_time()
{
	int time_need = psetting.angleNum*psetting.step_azAngle/psetting.SP +
					psetting.angleNum*psetting.sampleNum/(psetting.sampleFreq*1000000) +
					psetting.angleNum*ui->lineEdit_sglfilesize->text().toFloat()/25;
	if(psetting.step_azAngle == 0)
		time_need = time_need + (psetting.angleNum-1)*psetting.direction_intervalTime;
	else
		time_need = time_need + (psetting.angleNum-1)*psetting.direction_intervalTime
					+ psetting.time_circle_interval*60*psetting.angleNum/(360/psetting.step_azAngle);
	if(time_need < 1)
		ui->lineEdit_totalTime->setText("<1s");									//在1s以下
	else
		if(time_need < 60)														//在1min以下
			ui->lineEdit_totalTime->setText(QString::number(time_need)+"s");
		else
			if(time_need < 3600)												//在1h以下
			{
				int m = time_need/60;
				int s = time_need%60;
				if(s == 0)
					ui->lineEdit_totalTime->setText(QString::number(m)+"min");	//无秒
				else
					ui->lineEdit_totalTime->setText(QString::number(m)+"min"+QString::number(s)+"s");
			}
			else																//在1h以上
				{
					int h = time_need/3600;
					int remain = time_need%3600;
					if(remain == 0)
						ui->lineEdit_totalTime->setText(QString::number(h)+"h");//无分无秒
					else
					{
						int m = remain/60;
						int s = remain%60;
						if(s == 0)
							ui->lineEdit_totalTime->setText(QString::number(h)+"h"+QString::number(m)+"m");//无秒
						else
							ui->lineEdit_totalTime->setText(QString::number(h)+"h"+QString::number(m)+"m"+QString::number(s)+"s");
					}
				}
}

void paraDialog::single_filesize()
{
	filesize_over();
	ui->lineEdit_sglfilesize->setText(QString::number(direct_size/(1024*1024),'f',2));		//单位MB
	ui->lineEdit_totalsize->setText(QString::number(psetting.angleNum*direct_size/(1024*1024),'f',2));
	set_dect_time();
}

void paraDialog::double_filesize()
{
	filesize_over();
	ui->lineEdit_sglfilesize->setText(QString::number(2*direct_size/(1024*1024),'f',2));
	ui->lineEdit_totalsize->setText(QString::number(2*psetting.angleNum*direct_size/(1024*1024),'f',2));
	set_dect_time();
}

void paraDialog::filesize_over()
{
	if(direct_size > 170*1024*1024)
	{
		ui->pushButton_save->setEnabled(false);
		ui->pushButton_sure->setEnabled(false);
		ui->comboBox_sampleFreq->setStyleSheet("color: red;""font-size:12pt;""font-family:'Microsoft YaHei UI';");
		ui->lineEdit_detRange->setStyleSheet("color: red;""font-size:12pt;""font-family:'Microsoft YaHei UI';");
		ui->lineEdit_plsAccNum->setStyleSheet("color: red;""font-size:12pt;""font-family:'Microsoft YaHei UI';");
	}
	else
	{
		ui->pushButton_save->setEnabled(true);
		if(nocollecting == false)																		//若程序采集，确定键为非使能状态
			ui->pushButton_sure->setEnabled(false);
		else
			ui->pushButton_sure->setEnabled(true);
		ui->comboBox_sampleFreq->setStyleSheet("color: black;""font-size:12pt;""font-family:'Microsoft YaHei UI';");
		ui->lineEdit_detRange->setStyleSheet("color: black;""font-size:12pt;""font-family:'Microsoft YaHei UI';");
		ui->lineEdit_plsAccNum->setStyleSheet("color: black;""font-size:12pt;""font-family:'Microsoft YaHei UI';");
	}
}

void paraDialog::set_laserRPF()														//psetting获取编辑框值
{
	psetting.laserRPF = ui->lineEdit_laserRPF->text().toInt();
}

void paraDialog::set_laserPulseWidth()												//psetting获取编辑框值
{
	psetting.laserPulseWidth = ui->lineEdit_laserPulseWidth->text().toInt();
}

void paraDialog::set_laserWaveLength()												//psetting获取编辑框值
{
	psetting.laserWaveLength = ui->lineEdit_laserWaveLength->text().toInt();
}

void paraDialog::set_AOM_Freq()														//psetting获取编辑框值
{
	psetting.AOM_Freq = ui->lineEdit_AOM_Freq->text().toInt();
}

void paraDialog::set_elevationAngle()												//俯仰角 决定探测方向是水平还是径向
{
	psetting.elevationAngle = ui->lineEdit_elevationAngle->text().toInt();
	show_detect_mode();																//参考信息中的探测方式
}

void paraDialog::set_start_azAngle()												//psetting获取编辑框值
{
	psetting.start_azAngle = ui->lineEdit_start_azAngle->text().toInt();
}

void paraDialog::set_step_azAngle()													//步进角 是否为0决定是不是圆周扫描，大小决定每周扫描数//psetting获取编辑框值
{
	psetting.step_azAngle = ui->lineEdit_step_azAngle->text().toInt();
	show_detect_mode();
	set_dect_time();
}

void paraDialog::set_SP_Interval()
{
	if(ui->lineEdit_step_azAngle->text().toInt() == 0)
	{
		ui->lineEdit_SP->setEnabled(false);
		ui->radioButton_anglekey->setChecked(true);
		ui->radioButton_circlekey->setEnabled(false);
		ui->radioButton_circlekey->setChecked(false);
		set_anglekey();
		ui->lineEdit_circle_interval->setEnabled(false);
	}
	else
	{
		ui->lineEdit_SP->setEnabled(true);
		ui->radioButton_circlekey->setEnabled(true);
		ui->lineEdit_circle_interval->setEnabled(true);
	}
}

//设置圆周数输入框的显示数值
void paraDialog::set_circleNum()													//圆周数 影响方向数,psetting获取编辑框值
{
	if(psetting.circlekey == true)
		psetting.circleNum = ui->lineEdit_circleNum->text().toFloat();
	else
	{
		psetting.angleNum = ui->lineEdit_angleNum->text().toInt();
		psetting.step_azAngle = ui->lineEdit_step_azAngle->text().toInt();
		psetting.circleNum = (float)psetting.angleNum/360*psetting.step_azAngle;
		ui->lineEdit_circleNum->setText(QString::number(psetting.circleNum,'f',2));
	}
	set_dect_time();																//预估时间
	if(ui->radioButton_singleCh->isChecked())										//总数据量
		ui->lineEdit_totalsize->setText(QString::number(psetting.angleNum*direct_size/(1024*1024),'f',2));
	else
		ui->lineEdit_totalsize->setText(QString::number(2*psetting.angleNum*direct_size/(1024*1024),'f',2));
}

//设置方向数输入框的显示数值
void paraDialog::set_angleNum()														//方向数 决定圆周数，影响总数据量（双通道乘2），探测总时间//psetting获取编辑框值
{
	if(psetting.anglekey == true)
		psetting.angleNum = ui->lineEdit_angleNum->text().toInt();
	else
	{
		psetting.circleNum = ui->lineEdit_circleNum->text().toFloat();
		psetting.step_azAngle = ui->lineEdit_step_azAngle->text().toInt();
		psetting.angleNum = (int)(psetting.circleNum*360/psetting.step_azAngle);
		ui->lineEdit_angleNum->setText(QString::number(psetting.angleNum));
	}
	set_dect_time();																//预估时间
	if(ui->radioButton_singleCh->isChecked())										//总数据量
		ui->lineEdit_totalsize->setText(QString::number(psetting.angleNum*direct_size/(1024*1024),'f',2));
	else
		ui->lineEdit_totalsize->setText(QString::number(2*psetting.angleNum*direct_size/(1024*1024),'f',2));
}

void paraDialog::set_anglekey()														//方向键
{
	psetting.anglekey = true;
	psetting.circlekey = false;
	ui->lineEdit_angleNum->setEnabled(true);
	ui->lineEdit_circleNum->setEnabled(false);
}

void paraDialog::set_circlekey()													//圆周键
{
	psetting.anglekey = false;
	psetting.circlekey = true;
	ui->lineEdit_angleNum->setEnabled(false);
	ui->lineEdit_circleNum->setEnabled(true);
}

void paraDialog::set_motorSP()														//电机转速
{
	if(ui->lineEdit_SP->text().toInt() > 90)
	{
		QMessageBox::warning(this,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("最高转速不能超过每秒90度"));
		ui->lineEdit_SP->setText(NULL);
	}
	else
	{
		psetting.SP = ui->lineEdit_SP->text().toInt();
		set_dect_time();
	}
}

void paraDialog::set_singleCh()														//单通道 影响触发电平，以及ch1的文件名编辑框、数据量
{
	psetting.isSingleCh = true;
//	psetting.doubleCh = false;
	ui->checkBox_channelA->setChecked(false);
	ui->checkBox_channelB->setChecked(false);
	ui->checkBox_channelA->setEnabled(false);
	ui->checkBox_channelB->setEnabled(false);
	update_s_filename();
	single_filesize();
	on_pushButton_dataFileName_sch_clicked();										//自动搜索单通文件的最小序号
}

void paraDialog::set_doubleCh()														//双通道 影响触发电平，以及chA、B文件名编辑框数据量
{
	psetting.isSingleCh = false;
//	psetting.doubleCh = true;
	ui->checkBox_channelA->setChecked(psetting.channel_A);
	ui->checkBox_channelB->setChecked(psetting.channel_B);
	ui->checkBox_channelA->setEnabled(true);
	ui->checkBox_channelB->setEnabled(true);
	update_d_filename();
	double_filesize();
	on_pushButton_dataFileName_sch_clicked();										//自动搜索双通道文件的最小序号
}

void paraDialog::set_trig_mode()
{
	trig_conversion = false;
	if(ui->comboBox_trig_mode->currentText() == QString::fromLocal8Bit("电平触发"))
	{
		psetting.trigger_mode = 3;
		ui->label_level_OR_holdOff->setText(QString::fromLocal8Bit("触发电平"));
		ui->lineEdit_trigLevel_OR_holdOff->setText(QString::number(psetting.trigLevel));
		ui->pushButton_conversion->setText(QString::fromLocal8Bit("LSB"));
	}
	else
	{
		psetting.trigger_mode = 2;
		ui->label_level_OR_holdOff->setText(QString::fromLocal8Bit("触发延迟"));
		ui->lineEdit_trigLevel_OR_holdOff->setText(QString::number(psetting.trigHoldOffSamples));
		ui->pushButton_conversion->setText(QString::fromLocal8Bit("点"));
	}
}

void paraDialog::set_trigLevel_OR_holdOff()											//psetting获取编辑框值
{
	if(trig_conversion == false)
	{
		if(psetting.trigger_mode == 3)													//电平触发
		{
			if((2047 <= ui->lineEdit_trigLevel_OR_holdOff->text().toInt())||(ui->lineEdit_trigLevel_OR_holdOff->text().toInt() <= -2048))
			{
				QMessageBox::warning(this,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("触发电平范围为-2048LSB至2047LSB"));
				ui->lineEdit_trigLevel_OR_holdOff->setText(NULL);
			}
			else
				psetting.trigLevel = ui->lineEdit_trigLevel_OR_holdOff->text().toInt();
		}
		else																			//外部触发
			psetting.trigHoldOffSamples = ui->lineEdit_trigLevel_OR_holdOff->text().toInt();
	}
	else
	{
		if(psetting.trigger_mode == 3)
		{
			if((1099 <= ui->lineEdit_trigLevel_OR_holdOff->text().toFloat())||(ui->lineEdit_trigLevel_OR_holdOff->text().toFloat() <= -1100))
			{
				QMessageBox::warning(this,QString::fromLocal8Bit("提示"),QString::fromLocal8Bit("触发电平范围为-1100mV至1099mV"));
				ui->lineEdit_trigLevel_OR_holdOff->setText(NULL);
			}
			else
			{
				if(ui->lineEdit_trigLevel_OR_holdOff->text().toFloat() >= 0)
					psetting.trigLevel = 2048*ui->lineEdit_trigLevel_OR_holdOff->text().toFloat()/1100 + 0.05;
				else
					psetting.trigLevel = 2048*ui->lineEdit_trigLevel_OR_holdOff->text().toFloat()/1100 - 0.05;
			}
		}
		else
			psetting.trigHoldOffSamples = 550*ui->lineEdit_trigLevel_OR_holdOff->text().toFloat()/1000 + 0.05;
	}
}

void paraDialog::set_time_direct_interval()
{
	psetting.direction_intervalTime = ui->lineEdit_direct_interval->text().toFloat();
	set_dect_time();
}

void paraDialog::set_time_circle_interval()
{
	psetting.time_circle_interval = ui->lineEdit_circle_interval->text().toFloat();
	set_dect_time();
}

void paraDialog::set_sampleFreq()													//采样频率 影响采样点数、单文件量、总数据量//psetting获取编辑框值
{
	psetting.sampleFreq = ui->comboBox_sampleFreq->currentText().toInt();
	psetting.sampleNum = psetting.sampleFreq*2*psetting.detRange/300;
	direct_size = 68+psetting.plsAccNum*psetting.sampleNum*2;						//单个方向上的数据量
	ui->lineEdit_sampleNum->setText(QString::number(psetting.sampleNum));			//采样点数
}

void paraDialog::set_detRange()														//探测距离 影响采样点数、单文件量、总数据量//psetting获取编辑框值
{
	psetting.detRange = 1000*(ui->lineEdit_detRange->text().toFloat());
	psetting.sampleNum = psetting.sampleFreq*2*psetting.detRange/300;
	direct_size = 68+psetting.plsAccNum*psetting.sampleNum*2;						//单个方向上的数据量
	ui->lineEdit_sampleNum->setText(QString::number(psetting.sampleNum));			//采样点数
}

void paraDialog::set_filesize()														//参考信息中的单文件量和总数据量
{
	if(ui->radioButton_singleCh->isChecked())
		single_filesize();
	else
		double_filesize();
}

void paraDialog::set_plsAccNum()													//脉冲数影响单文件量、总数据量（双通道乘2）//psetting获取编辑框值
{
	psetting.plsAccNum = ui->lineEdit_plsAccNum->text().toInt();
	direct_size = 68+psetting.plsAccNum*psetting.sampleNum*2;
	set_filesize();
}

void paraDialog::set_dataFileName_Suffix()											//文件的后缀名
{
	psetting.dataFileName_Suffix = ui->lineEdit_dataFileName_Suffix->text();
	if(ui->radioButton_singleCh->isChecked())
		update_s_filename();
	else
		update_d_filename();
}

void paraDialog::set_channelA()
{
	psetting.channel_A = ui->checkBox_channelA->isChecked();
}

void paraDialog::set_channelB()
{
	psetting.channel_B = ui->checkBox_channelB->isChecked();
}

void paraDialog::on_pushButton_conversion_clicked()
{
	if(trig_conversion == false)
	{
		trig_conversion = true;
		if(psetting.trigger_mode == 3)
		{

			ui->lineEdit_trigLevel_OR_holdOff->setText(QString::number((float)1100*psetting.trigLevel/2048,'f',2));
			ui->pushButton_conversion->setText(QString::fromLocal8Bit("mV"));
		}
		else
		{
			ui->lineEdit_trigLevel_OR_holdOff->setText(QString::number((float)1000*psetting.trigHoldOffSamples/psetting.sampleFreq,'f',2));
			ui->pushButton_conversion->setText(QString::fromLocal8Bit("ns"));
		}
	}
	else
	{
		trig_conversion = false;
		if(psetting.trigger_mode == 3)
		{
			ui->lineEdit_trigLevel_OR_holdOff->setText(QString::number(psetting.trigLevel));
			ui->pushButton_conversion->setText(QString::fromLocal8Bit("LSB"));
		}
		else
		{
			ui->lineEdit_trigLevel_OR_holdOff->setText(QString::number(psetting.trigHoldOffSamples));
			ui->pushButton_conversion->setText(QString::fromLocal8Bit("点"));
		}
	}
}

//单次最大脉冲数
void paraDialog::on_pushButton_clicked()
{
	int max_plsAcc;
	if(psetting.trigger_mode == 2)					//外部触发
		max_plsAcc = (170*1024*1024-68)/(psetting.sampleNum*2 + psetting.trigHoldOffSamples*2);
	else											//电平触发
		max_plsAcc = (170*1024*1024-68)/(2*psetting.sampleNum);
	ui->lineEdit_plsAccNum->setText(QString::number(max_plsAcc));
}

//自动创建日期文件夹
void paraDialog::on_checkBox_autocreate_datafile_clicked()
{
	psetting.autocreate_datafile = ui->checkBox_autocreate_datafile->isChecked();
	QString str = psetting.DatafilePath;

	QDir mypath(str);
	QString dirname = mypath.dirName();
	QDateTime time = QDateTime::currentDateTime();

	if(psetting.autocreate_datafile)
	{
		int num = dirname.toInt();
		int len = dirname.length();
		QString today_str = time.toString("yyyyMMdd");
		int today_int = today_str.toInt();
		if(len == 8 && (num != today_int) && qAbs(num - today_int)<10000)
		{
			str = mypath.absolutePath();
			int str_len = str.length();
			str.resize(str_len - 8);
			str += today_str;
			qDebug()<<str<<endl;
		}

		else if( dirname != time.toString("yyyyMMdd"))
		{
			str = mypath.absolutePath();
			str += QString("/");
			str += time.toString("yyyyMMdd");			//设置显示格式
			qDebug()<<"Dir not Match";
		}
		qDebug()<<str<<endl;
	}
	else												//取消选择时，如果当前日期路径不存在，则取消，如存在，则不变。
	{
		if( dirname == time.toString("yyyyMMdd"))
		{
			if (!mypath.exists())
			{
				str = mypath.absolutePath();
				int str_len = str.length();
				str.resize(str_len - 9);				//减去/20xxxxxx
			}
			qDebug()<<"Dir Match"<<str<<endl;
		}
	}
	Set_DatafilePath(str);
}

void paraDialog::on_pushButton_pathModify_clicked()									//修改路径键
{
	QFileDialog *fd=new QFileDialog(this,QString::fromLocal8Bit("选择文件夹"),psetting.DatafilePath);
	fd->setFileMode(QFileDialog::Directory);
	fd->setOption(QFileDialog::ShowDirsOnly, true);
	if(fd->exec() == QFileDialog::Accepted)
	{
		QStringList file = fd->selectedFiles();
		QString str = static_cast<QString>(file.at(0));
		if (str.length() == 3)
			str.resize(2);
		Set_DatafilePath(str);
	}
	on_checkBox_autocreate_datafile_clicked();
}

void paraDialog::Set_DatafilePath(QString str)										//路径显示设置
{
	QDir mypath(str);
	if(!mypath.exists())															//路径不存在，红色
		ui->lineEdit_DatafilePath->setStyleSheet("color: red;""font-size:10pt;""font-family:'Microsoft YaHei UI';");
	else																			//存在，黑色
		ui->lineEdit_DatafilePath->setStyleSheet("color: black;""font-size:10pt;""font-family:'Microsoft YaHei UI';");
	psetting.DatafilePath = str;
	ui->lineEdit_DatafilePath->setText(str);
}

void paraDialog::on_pushButton_dataFileName_sch_clicked()							//自动查找最小序号
{
	QString filter_str;
	if(psetting.isSingleCh)															//设置文件名过滤器，如"Prefix-[0123456789][0123456789][0123456789]"的形式
		filter_str = psetting.dataFileName_Prefix + "_ch[1]_";
	else
		filter_str = psetting.dataFileName_Prefix + "_ch[AB]_";
	int suffix_l = psetting.dataFileName_Suffix.length();
	for(int i=0;i<suffix_l;i++)
		filter_str += "[0123456789]";

	filter_str += ".wld";
	QStringList FN_list;
	QStringList filter(filter_str);

	QDir *dir = new QDir(psetting.DatafilePath);									// 获取路径下的文件列表
	dir->setNameFilters(filter);

	QList<QFileInfo> *fileInfo = new QList<QFileInfo>(dir->entryInfoList(filter));	// 设置文件名过滤器

	int file_numbers = fileInfo->count();
	int max_num = 0;
	int tmp_num = 0;
	for(int i=0;i<file_numbers;i++)													//搜索当前最大序号
	{
		FN_list<<fileInfo->at(i).baseName().right(suffix_l);
		tmp_num = fileInfo->at(i).baseName().right(suffix_l).toInt();
		if (tmp_num > max_num)
			max_num = tmp_num;
	}
	max_num ++;
	if(suffix_l<=8)
	{
		psetting.dataFileName_Suffix.sprintf("%08d", max_num);
		psetting.dataFileName_Suffix = psetting.dataFileName_Suffix.right(suffix_l);
	}
	ui->lineEdit_dataFileName_Suffix->setText(psetting.dataFileName_Suffix);
}

void paraDialog::on_pushButton_save_clicked()										//保存键
{
	profile_path = QFileDialog::getSaveFileName(this,QString::fromLocal8Bit("保存"),".","*.ini");
	if(!profile_path.isEmpty())
	{
		if(QFileInfo(profile_path).suffix().isEmpty())
			profile_path.append(".ini");											//如果无后缀，自动补上.ini
		dlg_setfile.writeTo_file(psetting,profile_path);
	}
}

void paraDialog::on_pushButton_load_clicked()										//加载键
{
	profile_path = QFileDialog::getOpenFileName(this,QString::fromLocal8Bit("打开"),".","*.ini");
	if(!profile_path.isEmpty())
	{
		dlg_setfile.readFrom_file(profile_path);
		psetting = dlg_setfile.get_setting();
		defaulsetting = dlg_setfile.get_setting();
		update_show();
	}
}

void paraDialog::on_pushButton_reset_clicked()										//重置键
{
	psetting = defaulsetting;
	update_show();
}

void paraDialog::on_pushButton_sure_clicked()										//确定键
{
//	if(dlg_setfile.isSettingsChanged(psetting))										//文件未保存时
//	{
//		QMessageBox::StandardButton reply = QMessageBox::warning(this,QString::fromLocal8Bit("提示"),
//																 QString::fromLocal8Bit("修改的参数未保存，是否要保存修改"),
//																 QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel);
//		if(reply == QMessageBox::Save)												//点击是，弹出保存窗口
//			on_pushButton_save_clicked();
//		else
//			if(reply == QMessageBox::Discard)										//点击否时，不保存并accept()
//				accept();
//	}
//	else																			//文件若已保存，则accept()
//		accept();
	QString Disk_Name = psetting.DatafilePath.left(3);								//路径对应硬盘分区名
	quint64 freeSpace = getDiskFreeSpace(Disk_Name);								//获取路径对应硬盘分区的空间大小MB
	float totalfile_Space = ui->lineEdit_totalsize->text().toFloat();				//获取数据文件总量
	if(freeSpace > totalfile_Space+100)
		accept();
	else
		QMessageBox::warning(this,QString::fromLocal8Bit("提示"),
							 Disk_Name.left(1)+QString::fromLocal8Bit("盘剩余空间")
							 + QString::number(freeSpace) + QString::fromLocal8Bit("MB"));
}

void paraDialog::on_pushButton_cancel_clicked()										//取消键
{
	reject();
}

quint64 paraDialog::getDiskFreeSpace(QString driver)
{
	LPCWSTR lpcwstrDriver = (LPCWSTR)driver.utf16();
	ULARGE_INTEGER liFreeBytesAvailable,liTotalBytes,liTotalFreeBytes;
	if(!GetDiskFreeSpaceEx(lpcwstrDriver,&liFreeBytesAvailable,&liTotalBytes,&liTotalFreeBytes))
	{
		qDebug() << "Error: Call to GetDiskFreeSpaceEx failed";
		return 0;
	}
	return (quint64)liTotalFreeBytes.QuadPart/1024/1024;			//单位为MB
}

ACQSETTING paraDialog::get_settings()
{
	return psetting;
}
