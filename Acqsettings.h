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
    quint16 elevationAngle;
    quint16 start_azAngle;
    quint16 step_azAngle;
    quint16 angleNum;
    float circleNum;
    bool anglekey;
    bool circlekey;

    //采样参数
    bool singleCh;
    bool doubleCh;
    quint16 triggleLevel;
	quint16 triggerHoldOffSamples;
    quint16 sampleFreq;
    quint32 sampleNum;
    float detRange;
    quint16 plsAccNum;

    //文件存储
    QString DatafilePath;
    bool autocreate_datafile;
    QString dataFileName_Prefix;
    QString dataFileName_Suffix;
}ACQSETTING;

#endif // ACQSETTINGS_H
