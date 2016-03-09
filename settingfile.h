#ifndef SETTINGFILE_H
#define SETTINGFILE_H

#include "Acqsettings.h"

class settingFile
{
public:
    settingFile();

	//配置文件
    ACQSETTING get_setting(void);
	void init_fsetting(const ACQSETTING &setting);
	void writeTo_file(const ACQSETTING &setting,const QString &a);	//写入文件中
	void readFrom_file(const QString &b);							//从文件中读取
	void checkValid();												//检查加载的配置文件是否有效
	void test_create_file(const QString &c);						//检查配置文件是否存在，若无则生成
	bool isSettingsChanged(const ACQSETTING &setting);				//设置窗口参数修改后，未保存时提示是否保存

	//记录文件
	void updatelogFile(const QString &addInstruct);					//记录采集过程

private:
	ACQSETTING fsetting;
	QString LF_path;												//记录采集文件的路径
	QString instruct_str;											//文字说明
};

#endif // SETTINGFILE_H
