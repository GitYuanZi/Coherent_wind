#ifndef SETTINGFILE_H
#define SETTINGFILE_H

#include "Acqsettings.h"

class settingFile
{
public:
    settingFile();

    ACQSETTING get_setting(void);
	void init_fsetting(const ACQSETTING &setting);
	void writeTo_file(const ACQSETTING &setting,QString a);		//写入文件中
	void readFrom_file(QString a);								//从文件中读取
	void checkValid();											//检查加载的配置文件是否有效
	void test_create_file(QString a);							//检查配置文件是否存在，若无则生成
	bool isSettingsChanged(const ACQSETTING &setting);			//设置窗口参数修改后，未保存时提示是否保存

private:
    ACQSETTING fsetting;
};

#endif // SETTINGFILE_H
