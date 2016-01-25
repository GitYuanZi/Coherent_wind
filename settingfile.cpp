#include "settingfile.h"

#include <QSettings>
#include <QDebug>
#include <QtCore>
#include <QDateTime>
#include "Acqsettings.h"

settingFile::settingFile()
{
}

void settingFile::init_fsetting(const ACQSETTING &setting)
{
	fsetting = setting;
}

void settingFile::writeTo_file(const ACQSETTING &setting,const QString &a )
{
	fsetting = setting;
	QString path_a = a;

	QSettings settings(path_a,QSettings::IniFormat);
	settings.beginGroup("Laser_parameters");
	settings.setValue("laserRPF",fsetting.laserRPF);					//激光重频
	settings.setValue("laserPulseWidth",fsetting.laserPulseWidth);		//激光脉宽
	settings.setValue("laserWaveLength",fsetting.laserWaveLength);		//激光波长
	settings.setValue("AOM_Freq",fsetting.AOM_Freq);					//AOM移频量
	settings.endGroup();

	settings.beginGroup("Scan_parameters");
	settings.setValue("elevationAngle",fsetting.elevationAngle);		//俯仰角
	settings.setValue("start_azAngle",fsetting.start_azAngle);			//起始角
	settings.setValue("step_azAngle",fsetting.step_azAngle);			//步进角
	settings.setValue("angleNum",fsetting.angleNum);					//方向数
	settings.setValue("circleNum",fsetting.circleNum);					//圆周数
	settings.setValue("anglekey",fsetting.anglekey);					//方向数
	settings.setValue("circlekey",fsetting.circlekey);					//圆周数
	settings.setValue("SP",fsetting.SP);								//电机速度
	settings.endGroup();

	settings.beginGroup("Sample_parameters");
	settings.setValue("singleCh",fsetting.singleCh);					//单通道
	settings.setValue("doubleCh",fsetting.doubleCh);					//双通道
	settings.setValue("trigger_mode",fsetting.trigger_mode);			//触发方式
	settings.setValue("trigLevel",fsetting.trigLevel);					//触发电平
	settings.setValue("trigHoldOffSamples",fsetting.trigHoldOffSamples);//触发延迟
	settings.setValue("time_direct_interval",fsetting.time_direct_interval);//方向间间隔
	settings.setValue("time_circle_interval",fsetting.time_circle_interval);//圆周间间隔
	settings.setValue("sampleFreq",fsetting.sampleFreq);				//采样频率
	settings.setValue("detRange",fsetting.detRange);					//探测距离
	settings.setValue("sampleNum",fsetting.sampleNum);
	settings.setValue("plsAccNum",fsetting.plsAccNum);					//脉冲数
	settings.endGroup();

	settings.beginGroup("File_store");
	settings.setValue("DatafilePath",fsetting.DatafilePath);					//文件保存路径
	settings.setValue("autocreate_datafile",fsetting.autocreate_datafile);		//自动创建日期文件夹
	settings.setValue("channel_A",fsetting.channel_A);
	settings.setValue("channel_B",fsetting.channel_B);
	settings.setValue("dataFileNamePrefix",fsetting.dataFileName_Prefix);		//前缀文件名
	settings.setValue("dataFileNameSuffix",fsetting.dataFileName_Suffix);		//后缀文件名
	settings.endGroup();
}

void settingFile::readFrom_file(const QString &b)
{
	QString path_b = b;
	QSettings settings(path_b,QSettings::IniFormat);
	settings.beginGroup("Laser_parameters");
	fsetting.laserRPF = settings.value("laserRPF").toInt();
	fsetting.laserPulseWidth = settings.value("laserPulseWidth").toInt();
	fsetting.laserWaveLength = settings.value("laserWaveLength").toInt();
	fsetting.AOM_Freq = settings.value("AOM_Freq").toInt();
	settings.endGroup();

	settings.beginGroup("Scan_parameters");
	fsetting.elevationAngle = settings.value("elevationAngle").toInt();		//俯仰角
	fsetting.start_azAngle = settings.value("start_azAngle").toInt();		//起始角
	fsetting.step_azAngle = settings.value("step_azAngle").toInt();			//步进角
	fsetting.angleNum = settings.value("angleNum").toInt();					//方向数
	fsetting.circleNum = settings.value("circleNum").toFloat();				//圆周数
	fsetting.anglekey = settings.value("anglekey").toBool();				//方向键
	fsetting.circlekey = settings.value("circlekey").toBool();				//圆周键
	fsetting.SP = settings.value("SP").toInt();								//电机速度
	settings.endGroup();

	settings.beginGroup("Sample_parameters");
	fsetting.singleCh = settings.value("singleCh").toBool();				//单通道
	fsetting.doubleCh = settings.value("doubleCh").toBool();				//双通道
	fsetting.trigger_mode = settings.value("trigger_mode").toInt();			//触发方式
	fsetting.trigLevel = settings.value("trigLevel").toInt();				//触发电平
	fsetting.trigHoldOffSamples = settings.value("trigHoldOffSamples").toInt();//触发延迟
	fsetting.time_direct_interval = settings.value("time_direct_interval").toFloat();//方向间间隔
	fsetting.time_circle_interval = settings.value("time_circle_interval").toFloat();//圆周间间隔
	fsetting.sampleFreq = settings.value("sampleFreq").toInt();				//采样频率
	fsetting.detRange = settings.value("detRange").toFloat();				//探测距离
	fsetting.sampleNum = settings.value("sampleNum").toInt();				//采样点数
	fsetting.plsAccNum = settings.value("plsAccNum").toInt();				//脉冲数
	settings.endGroup();

	settings.beginGroup("File_store");
	fsetting.DatafilePath = settings.value("DatafilePath").toString();					//文件保存路径
	fsetting.autocreate_datafile = settings.value("autocreate_datafile").toBool();		//自动创建最小文件夹
	fsetting.channel_A = settings.value("channel_A").toBool();
	fsetting.channel_B = settings.value("channel_B").toBool();
	fsetting.dataFileName_Prefix = settings.value("dataFileNamePrefix").toString();		//前缀文件名
	fsetting.dataFileName_Suffix = settings.value("dataFileNameSuffix").toString();		//后缀文件名
	settings.endGroup();
}

void settingFile::checkValid()
{
}

//检查settings.ini是否存在，若不存在则创建
void settingFile::test_create_file(const QString &c)
{
	QString path_c = c;
	QString prefix_str = QDateTime::currentDateTime().toString("yyyyMMdd");				//获取最新日期
	QFileInfo file(path_c);
	QSettings settings(path_c,QSettings::IniFormat);
	if(file.exists() == false)
	{
		settings.beginGroup("Laser_parameters");
		settings.setValue("laserRPF",10000);				//激光重频
		settings.setValue("laserPulseWidth",500);			//激光脉宽
		settings.setValue("laserWaveLength",1540);			//激光波长
		settings.setValue("AOM_Freq",120);					//AOM移频量
		settings.endGroup();

		settings.beginGroup("Scan_parameters");
		settings.setValue("elevationAngle",70);				//俯仰角
		settings.setValue("start_azAngle",0);				//起始角
		settings.setValue("step_azAngle",90);				//步进角
		settings.setValue("angleNum",80);					//方向数
		settings.setValue("circleNum",20);					//圆周数
		settings.setValue("anglekey",true);					//方向键
		settings.setValue("circlekey",false);				//圆周键
		settings.setValue("SP",90);							//电机速度
		settings.endGroup();

		settings.beginGroup("Sample_parameters");
		settings.setValue("singleCh",true);					//单通道
		settings.setValue("doubleCh",false);				//双通道
		settings.setValue("trigger_mode",3);				//触发方式
		settings.setValue("trigLevel",1);					//触发电平
		settings.setValue("trigHoldOffSamples",500);		//触发延迟
		settings.setValue("time_direct_interval",0);		//方向间间隔
		settings.setValue("time_circle_interval",0);		//圆周间间隔
		settings.setValue("sampleFreq",550);				//采样频率
		settings.setValue("detRange",6000);					//探测距离
		settings.setValue("sampleNum",22000);				//采样点数
		settings.setValue("plsAccNum",100);					//脉冲数
		settings.endGroup();

		settings.beginGroup("File_store");
		path_c.chop(13);									//截掉末尾配置文件名
		path_c.append("/").append(prefix_str);				//路径末尾加上日期文件夹
		settings.setValue("DatafilePath",path_c);			//文件保存路径
		settings.setValue("autocreate_datafile",true);		//自动创建日期文件夹
		settings.setValue("channel_A",true);				//通道A
		settings.setValue("channel_B",true);				//通道B
		settings.setValue("dataFileNamePrefix",prefix_str);	//前缀文件名
		settings.setValue("dataFileNameSuffix","001");		//后缀文件名
		settings.endGroup();
	}
	else
	{
		settings.beginGroup("File_store");
		settings.setValue("dataFileNamePrefix",prefix_str);	//前缀文件名
		settings.endGroup();
		qDebug() <<"Settings file exist";
	}
}

bool settingFile::isSettingsChanged(const ACQSETTING &setting)
{
	ACQSETTING dlgsetting = setting;
	if(fsetting.laserRPF != dlgsetting.laserRPF)					//激光重频
		return true;
	if(fsetting.laserPulseWidth != dlgsetting.laserPulseWidth)		//脉冲宽度
		return true;
	if(fsetting.laserWaveLength != dlgsetting.laserWaveLength)		//激光波长
		return true;
	if(fsetting.AOM_Freq != dlgsetting.AOM_Freq)					//AOM移频量
		return true;

	if(fsetting.elevationAngle != dlgsetting.elevationAngle)		//俯仰角
		return true;
	if(fsetting.start_azAngle != dlgsetting.start_azAngle)			//起始角
		return true;
	if(fsetting.step_azAngle != dlgsetting.step_azAngle)			//步进角
		return true;
	if(fsetting.angleNum != dlgsetting.angleNum)					//方向数
		return true;
	if(fsetting.circleNum != dlgsetting.circleNum)					//圆周数
		return true;
	if(fsetting.anglekey != dlgsetting.anglekey)
		return true;
	if(fsetting.circlekey != dlgsetting.circlekey)
		return true;
	if(fsetting.SP != dlgsetting.SP)								//电机速度
		return true;

	if(fsetting.singleCh != dlgsetting.singleCh)					//单通道
		return true;
	if(fsetting.doubleCh != dlgsetting.doubleCh)					//双通道
		return true;
	if(fsetting.trigger_mode != dlgsetting.trigger_mode)
		return true;
	if(fsetting.trigLevel != dlgsetting.trigLevel)					//触发电平
		return true;
	if(fsetting.trigHoldOffSamples != dlgsetting.trigHoldOffSamples)//触发延迟
		return true;
	if(fsetting.time_direct_interval != dlgsetting.time_direct_interval)
		return true;
	if(fsetting.time_circle_interval != dlgsetting.time_circle_interval)
		return true;
	if(fsetting.sampleFreq != dlgsetting.sampleFreq)				//采样频率
		return true;
	if(fsetting.detRange != dlgsetting.detRange)					//探测距离
		return true;
	if(fsetting.sampleNum != dlgsetting.sampleNum)
		return true;
	if(fsetting.plsAccNum != dlgsetting.plsAccNum)
		return true;

	if(fsetting.DatafilePath != dlgsetting.DatafilePath)			//文件保存路径
		return true;
	if(fsetting.autocreate_datafile != dlgsetting.autocreate_datafile)//自动创建日期文件夹
		return true;
	if(fsetting.channel_A != dlgsetting.channel_A)
		return true;
	if(fsetting.channel_B != dlgsetting.channel_B)
		return true;
	if(fsetting.dataFileName_Prefix != dlgsetting.dataFileName_Prefix)
		return true;
	if(fsetting.dataFileName_Suffix != dlgsetting.dataFileName_Suffix)
		return true;

	return false;
}

ACQSETTING settingFile::get_setting()
{
	return fsetting;
}

