#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include <QDialog>
#include <fstream>
#include "qwt_plot.h"
#include "Acqsettings.h"

class MainWindow;
class QwtPlotGrid;											//网格
class QwtPlotCurve;											//曲线
class QwtPlotZoomer;										//放大
class QwtPlotPicker;										//选择
class QwtPlotPanner;										//平移

class PlotWindow : public QDialog
{
    Q_OBJECT
public:
    PlotWindow(MainWindow *paraent);
	void show(void);										//网格固定、最小尺寸

	void enableZoomMode(bool);								//缩放模式启用
	void setMaxX(int xnum);									//x轴参数选择，y轴自动缩放，原始坐标轴值范围设置
    void datashow(const qint16 *dats,uint snum,uint pnum);  //单通道更新数据显示
	void set_titleName(QString ch_name);					//通道名设置

protected:
    void timerEvent(QTimerEvent *);
    void resizeEvent(QResizeEvent *);
	void keyPressEvent(QKeyEvent *);

private:
    QwtPlot *qwtPlot;
    QwtPlotGrid *grid;
    MainWindow *parentWindow;
    double *xValues;
    double *yValues;
    QwtPlotCurve *qwtPlotCurve;
    QwtPlotZoomer *d_zoomer;
    QwtPlotPanner *d_panner;
	int ch_name;											//用于设置通道名
	int H_trim;												//曲线显示区域，高度的所见值，形成底部空白，最大化时用于避开windows任务栏
};

#endif // PLOTWIDGET_H
