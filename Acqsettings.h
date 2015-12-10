#ifndef ACQSETTINGS_H
#define ACQSETTINGS_H

#include <QString>
typedef struct
{
    //激光参数
    quint16 laserRPF;
    quint16 laserPulseWidth;
    quint16 laserWaveLength;
    quint16 AOM_Freq;

    //扫描参数
	quint16 SP;				//驱动器速度
	quint16 elevationAngle;
    quint16 start_azAngle;
    quint16 step_azAngle;
	quint32 angleNum;
    float circleNum;
    bool anglekey;
    bool circlekey;

    //采样参数
    bool singleCh;
    bool doubleCh;
	qint16 triggleLevel;
	qint16 triggerHoldOffSamples;
    quint16 sampleFreq;
    quint32 sampleNum;
    float detRange;
    quint16 plsAccNum;

	//文件存储
    QString DatafilePath;
    bool autocreate_datafile;
	bool channel_A;
	bool channel_B;
    QString dataFileName_Prefix;
	QString dataFileName_Suffix;
}ACQSETTING;

#endif // ACQSETTINGS_H
