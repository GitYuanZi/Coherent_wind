#ifndef INFORMATIONLEFT_H
#define INFORMATIONLEFT_H

#include <QDialog>

namespace Ui {
class informationleft;
}

class informationleft : public QDialog
{
    Q_OBJECT

public:
    explicit informationleft(QWidget *parent = 0);
    ~informationleft();

public slots:
	void set_currentAngle(float a);			//当前角度
	void set_groupcnt(quint16 a);			//采集组数分数？
	void set_groupNum(quint32 a);			//采集组数百分数
	void set_filename1(QString str);		//数据文件名1
	void set_filename2(QString str);		//数据文件名2

private:
    Ui::informationleft *ui;

	quint16 groupNum;						//总组数
	int groupcnt;							//采集组数
};

#endif // INFORMATIONLEFT_H
