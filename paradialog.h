#ifndef PARADIALOG_H
#define PARADIALOG_H

#include <QDialog>
#include "Acqsettings.h"
#include "settingfile.h"

namespace Ui {
class paraDialog;
}

class paraDialog : public QDialog
{
    Q_OBJECT

public:
    explicit paraDialog(QWidget *parent = 0);
    ~paraDialog();

	void init_setting(const ACQSETTING &setting, int b, bool sop);
    ACQSETTING get_setting(void);
    void update_show();
															//参考信息显示
    void updates_filename();
    void updated_filename();
	void show_detect_mode();								//探测方式

	void initial_para();
	void Set_DatafilePath(QString str);

public slots:
    void on_checkBox_autocreate_datafile_clicked();

private slots:

	void set_laserRPF();
	void set_laserPulseWidth();
	void set_laserWaveLength();
	void set_AOM_Freq();

	void set_elevationAngle();
	void set_step_azAngle();
	void set_start_azAngle();
	void set_angleNum();
	void set_circleNum();
	void set_anglekey();
	void set_circlekey();

	void set_singleCh();
	void set_doubleCh();
	void set_triggerLevel();
	void set_triggerHoldOffSamples();
    void set_sampleFreq();
	void set_detRange();
	void set_plsAccNum();

	void set_filesize();
	void set_dataFileName_Suffix();

    void on_pushButton_pathModify_clicked();
    void on_pushButton_save_clicked();
    void on_pushButton_sure_clicked();
    void on_pushButton_cancel_clicked();
    void on_pushButton_load_clicked();
    void on_pushButton_reset_clicked();
    void on_pushButton_dataFileName_sch_clicked();

private:
    Ui::paraDialog *ui;
    ACQSETTING psetting;
    ACQSETTING defaulsetting;
	double direct_size;											//单方向上的数据量
	int sampleNum;												//单个脉冲的采样点数
	settingFile dlg_setfile;
	QString profile_path;										//配置文件路径

	int dlg_SP;													//SP值
	void set_dect_time();										//计算预估探测时间
	bool nocollecting;											//是否正在采集数据
};

#endif // PARADIALOG_H
