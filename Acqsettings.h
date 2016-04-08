#ifndef ACQSETTINGS_H
#define ACQSETTINGS_H

#include <QString>
typedef struct
{
    //激光参数
	quint16 laserRPF;			//激光频率
	quint16 laserPulseWidth;	//脉冲宽度
	quint16 laserWaveLength;	//激光波长
	quint16 AOM_Freq;			//AOM移频量

    //扫描参数
	quint16 elevationAngle;		//俯仰角
	quint16 start_azAngle;		//起始角
	quint16 step_azAngle;		//步进角
	quint32 angleNum;			//方向数
	float circleNum;			//圆周数
	bool anglekey;				//方向键
	bool circlekey;				//圆周键
	quint16 SP;					//驱动器速度
	float direct_intervalTime;	//方向间间隔
	float time_circle_interval;	//圆周间间隔

    //采样参数
	bool isSingleCh;			//是否单通道
	int trigger_mode;			//触发方式
	qint16 trigLevel;			//触发电平
	bool isPreTrig;				//预触发
	qint16 Pre_OR_HoldOff_Samples;	//预触发或触发延迟数
	quint16 sampleFreq;			//采样频率
	float detRange;				//采样距离
	quint32 sampleNum;			//采样点数
	quint16 plsAccNum;			//单次脉冲数

	//文件存储
	QString DatafilePath;		//数据存储路径
	bool autocreate_datafile;	//自动创建日期文件夹
	bool channel_A;				//通道A存储
	bool channel_B;				//通道B存储
	QString dataFileName_Prefix;//数据文件前缀
	QString dataFileName_Suffix;//数据文件后缀
}ACQSETTING;

typedef struct
{
	bool hide_grid;				//隐藏网格
	bool showA;					//通道A显示
	bool showB;					//通道B显示
	bool countNum;				//计数序号
	bool echoDistance;			//回波距离
}PLOTPARA;

#endif // ACQSETTINGS_H
