#ifndef PORTDIALOG_H
#define PORTDIALOG_H

#include <QDialog>
#include "serialportthread.h"

namespace Ui {
class portDialog;
}

class portDialog : public QDialog
{
	Q_OBJECT

public:
	explicit portDialog(QWidget *parent = 0);
	~portDialog();

	void inital_data(const QString &c,quint16 a,quint16 b);//默认的设置参数

	QString get_returnSet();					//将类型为QString的串口值返回到主线程中

	void search_port();

private slots:
//	void opposite_transaction();				//相对转动
//	void absolute_transaction();				//绝对转动

	void on_pushButton_default_clicked();		//默认键

	void on_pushButton_sure_clicked();

	void on_pushButton_cancel_clicked();

	void on_pushButton_auto_searchPort_clicked();

	void on_pushButton_opposite_clicked();

	void on_pushButton_absolute_clicked();

	void on_pushButton_setPXis0_clicked();

private:
	Ui::portDialog *ui;
	QString portTested;
	SerialPortThread thread_dia;

	QString dialog_SP;			//速度
	QString dialog_AC;			//加速度
	QString dialog_DC;			//减速度
	QString dialog_PR;			//移动距离
	QString dialog_PA;			//绝对距离
	QString dialog_PX;			//当前位置
	QString request;
	quint16 step;
	quint16 Num;
};

#endif // PORTDIALOG_H
