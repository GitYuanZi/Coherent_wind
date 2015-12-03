#ifndef THREADSTORE_H
#define THREADSTORE_H

#include <QThread>
#include <QString>
#include "Acqsettings.h"

class threadStore : public QThread
{
    Q_OBJECT

public:
	threadStore();										//构造函数，用于在声明类的对象时进行初始化

    void fileDataPara(const ACQSETTING &setting);

	void singleData();									//文件头和数据写入
    void doubleData();

	void s_memcpy(qint16 *a);							//内存拷贝
    void d_memcpy(qint16 *a,qint16 *b);

	void otherpara(QString a,QString b,uint c);			//采集文件的后缀序号，对应的采集时间和方位角信息

protected:
    void run();

signals:
	void store_finish();								//数据存储完成

private:
    ACQSETTING tsetting;

	qint16 *data_a;										//用于数据内存拷贝
    qint16 *data_b;

	QString collect_filesuffix;							//后缀序号
	QString collect_time;								//采集时间
	quint16 collect_angle;								//方位角
};

#endif // THREADSTORE_H
