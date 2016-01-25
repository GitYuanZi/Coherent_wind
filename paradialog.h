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

	void init_setting(const ACQSETTING &setting, bool sop);
	void initial_para();
	ACQSETTING get_settings(void);

public slots:
    void on_checkBox_autocreate_datafile_clicked();

private slots:
	void set_laserRPF();
	void set_laserPulseWidth();
	void set_laserWaveLength();
	void set_AOM_Freq();

	void set_elevationAngle();
	void set_step_azAngle();
	void set_SP_Interval();
	void set_start_azAngle();
	void set_circleNum();
	void set_angleNum();
	void set_anglekey();
	void set_circlekey();
	void set_motorSP();

	void set_singleCh();
	void set_doubleCh();
	void set_trig_mode();
	void set_trigLevel_OR_holdOff();
	void set_time_direct_interval();
	void set_time_circle_interval();
    void set_sampleFreq();
	void set_detRange();
	void set_filesize();
	void set_plsAccNum();

	void set_dataFileName_Suffix();
	void set_channelA();
	void set_channelB();

	void on_pushButton_conversion_clicked();
	void on_pushButton_clicked();
    void on_pushButton_pathModify_clicked();
	void on_pushButton_dataFileName_sch_clicked();
	void on_pushButton_save_clicked();
	void on_pushButton_load_clicked();
	void on_pushButton_reset_clicked();
    void on_pushButton_sure_clicked();
	void on_pushButton_cancel_clicked();

private:
    Ui::paraDialog *ui;
    ACQSETTING psetting;
    ACQSETTING defaulsetting;
	settingFile dlg_setfile;
	QString profile_path;										//配置文件路径
	double direct_size;											//单方向上的数据量
	int sampleNum;												//单个脉冲的采样点数
	bool nocollecting;											//是否正在采集数据
	bool trig_conversion;										//用于触发电平或延迟单位转换

	void update_show();
	void update_s_filename();
	void update_d_filename();
	void show_detect_mode();
	void set_dect_time();										//计算预估探测时间
	void single_filesize();										//单通道单文件数据量
	void double_filesize();										//双通道单文件数据量
	void filesize_over();										//判断单文件量超过采集卡内存
	void Set_DatafilePath(QString str);
	quint64 getDiskFreeSpace(QString driver);					//获取路径下的硬盘剩余空间大小
};

#endif // PARADIALOG_H
