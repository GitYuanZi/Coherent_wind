#ifndef PLOTDIALOG_H
#define PLOTDIALOG_H

#include <QDialog>
#include "Acqsettings.h"

namespace Ui {
class plotDialog;
}

class plotDialog : public QDialog
{
	Q_OBJECT

public:
	explicit plotDialog(QWidget *parent = 0);
	~plotDialog();
	void dialog_show(const PLOTPARA &setting,bool ch);
	PLOTPARA ret_settings(void);

private slots:
	void set_hide_grid();
	void set_showA();
	void set_showB();
	void set_countNum();
	void set_echoDistance();

	void on_pushButton_sure_clicked();		//确定键
	void on_pushButton_cancel_clicked();	//取消键

private:
	Ui::plotDialog *ui;
	PLOTPARA p_paraValue;
	bool p_ch;								//双通道
};

#endif // PLOTDIALOG_H
