#include "threadstore.h"
#include <QString>
#include <QFile>
#include <QDebug>
#include <QDataStream>
#include <QDateTime>

threadStore::threadStore()
{
}

void threadStore::run()
{
	if(tsetting.isSingleCh)
		singleData();
	else
		doubleData();
	emit this->store_finish();
}

void threadStore::singleData()
{
	QFile outfile1(tsetting.DatafilePath+"/"+tsetting.dataFileName_Prefix+"_ch1_"+tsetting.dataFileName_Suffix+".wld");

	if(outfile1.open(QFile::WriteOnly | QIODevice::Truncate))//QIODevice::Truncate表示将原文件内容清空
	{
		QDataStream in1(&outfile1);
		in1 << collect_time;
		if(tsetting.step_azAngle == 0)						//采集模式 径向or扫描
			in1 << tsetting.step_azAngle;
		else
		{
			quint16 x = 1;
			in1 << x;
		}
		in1 << tsetting.angleNum;							//采集组数，方向数
		in1 << collect_angle;
		in1 << tsetting.elevationAngle;						//俯仰角
		in1 << tsetting.plsAccNum;							//脉冲数
		in1 << tsetting.sampleFreq;							//采样频率
		in1 << tsetting.sampleNum;							//采样点数32
		in1 << tsetting.laserRPF;							//激光重频
		in1 << tsetting.laserPulseWidth;					//激光脉宽
		in1 << tsetting.laserWaveLength;					//激光波长
		in1 << tsetting.AOM_Freq;							//AOM移频量
//		int ret;
//		ret = in1.writeRawData((char*)data_a,tsetting.sampleNum*tsetting.plsAccNum*2);//返回值为写入的数据的字节数
		in1.writeRawData((char*)data_a,tsetting.sampleNum*tsetting.plsAccNum*2);
															//采样数据的写入
		outfile1.close();
		qDebug() << tsetting.dataFileName_Suffix << " has finished!";
	}
	delete[] data_a;										//删除new分配的内存
}

void threadStore::doubleData()
{
	if(tsetting.channel_A)
	{
		QFile outfileA(tsetting.DatafilePath+"/"+tsetting.dataFileName_Prefix+"_chA_"+tsetting.dataFileName_Suffix+".wld");
		if(outfileA.open(QFile::WriteOnly | QIODevice::Truncate))//QIODevice::Truncate表示将原文件内容清空
		{
			QDataStream inA(&outfileA);
			inA << collect_time;
			if(tsetting.step_azAngle == 0)						//采集模式 径向or扫描
				inA << tsetting.step_azAngle;
			else
			{
				quint16 x = 1;
				inA << x;
			}
			inA << tsetting.angleNum;							//采集组数，方向数
			inA << collect_angle;
			inA << tsetting.elevationAngle;						//俯仰角
			inA << tsetting.plsAccNum;							//脉冲数
			inA << tsetting.sampleFreq;							//采样频率
			inA << tsetting.sampleNum;							//采样点数32
			inA << tsetting.laserRPF;							//激光重频
			inA << tsetting.laserPulseWidth;					//激光脉宽
			inA << tsetting.laserWaveLength;					//激光波长
			inA << tsetting.AOM_Freq;							//AOM移频量
			inA.writeRawData((char*)data_a,tsetting.sampleNum*tsetting.plsAccNum*2);
			outfileA.close();
			qDebug() << tsetting.dataFileName_Suffix << " A has finished!";
		}
		delete[] data_a;
	}
	if(tsetting.channel_B)
	{
		QFile outfileB(tsetting.DatafilePath+"/"+tsetting.dataFileName_Prefix+"_chB_"+tsetting.dataFileName_Suffix+".wld");
		if(outfileB.open(QFile::WriteOnly | QIODevice::Truncate))//QIODevice::Truncate表示将原文件内容清空
		{
			QDataStream inB(&outfileB);
			inB << collect_time;
			if(tsetting.step_azAngle == 0)						//采集模式 径向or扫描
				inB << tsetting.step_azAngle;
			else
			{
				quint16 x = 1;
				inB << x;
			}
			inB << tsetting.angleNum;							//采集组数，方向数
			inB << collect_angle;
			inB << tsetting.elevationAngle;						//俯仰角
			inB << tsetting.plsAccNum;							//脉冲数
			inB << tsetting.sampleFreq;							//采样频率
			inB << tsetting.sampleNum;							//采样点数32
			inB << tsetting.laserRPF;							//激光重频
			inB << tsetting.laserPulseWidth;					//激光脉宽
			inB << tsetting.laserWaveLength;					//激光波长
			inB << tsetting.AOM_Freq;							//AOM移频量
			inB.writeRawData((char*)data_b,tsetting.sampleNum*tsetting.plsAccNum*2);
			outfileB.close();
			qDebug() << tsetting.dataFileName_Suffix << " B has finished!";
		}
		delete[] data_b;
	}
}

void threadStore::fileDataPara(const ACQSETTING &setting)
{
	tsetting = setting;
}

void threadStore::s_memcpy(qint16 *d_cha)
{
	data_a = new qint16[tsetting.sampleNum*tsetting.plsAccNum];
	memcpy(data_a,d_cha,tsetting.sampleNum*tsetting.plsAccNum*2);
}

void threadStore::d_memcpy(qint16 *d_cha, qint16 *d_chb)
{
	data_a = new qint16[tsetting.sampleNum*tsetting.plsAccNum];
	data_b = new qint16[tsetting.sampleNum*tsetting.plsAccNum];
	memcpy(data_a,d_cha,tsetting.sampleNum*tsetting.plsAccNum*2);
	memcpy(data_b,d_chb,tsetting.sampleNum*tsetting.plsAccNum*2);
}

void threadStore::otherpara(QString a, uint b)
{
	collect_time = a;
	collect_angle = b;
}
