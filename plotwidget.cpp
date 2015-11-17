#include "plotwidget.h"

#include <qwt_counter.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_renderer.h>
#include <qwt_text.h>
#include <qwt_math.h>
#include <QMessageBox>

#include "qwt_plot_grid.h"
#include "qwt_plot_curve.h"
#include "mainwindow.h"

class Zoomer : public QwtPlotZoomer
{
public:
	Zoomer(int xAxis,int yAxis,QWidget *canvas):
		QwtPlotZoomer(xAxis,yAxis,canvas)
	{
		setTrackerMode(QwtPicker::AlwaysOff);
		setRubberBand(QwtPicker::NoRubberBand);

		setMousePattern(QwtEventPattern::MouseSelect2,
						Qt::RightButton,Qt::ControlModifier);
		setMousePattern(QwtEventPattern::MouseSelect3,
						Qt::RightButton);
	}
};

PlotWindow::PlotWindow(MainWindow *parent):QDialog(parent)
{
	qwtPlot = new QwtPlot(this);
																	//设置坐标轴
	qwtPlot->setAxisScale(qwtPlot->xBottom,0,1000);					//x轴值的初始范围0到1000
	qwtPlot->setAxisScale(qwtPlot->yLeft,0,5);						//y轴值的初始范围0到5
																	//加入网格
	grid = new QwtPlotGrid;
	grid->enableX(true);
	grid->enableY(true);
	grid->enableXMin(false);
	grid->enableYMin(false);
	grid->setMajorPen(QPen(Qt::black,0,Qt::DotLine));				//网格黑色
	grid->attach(qwtPlot);
																	//加入缩放功能
	d_zoomer = new Zoomer(QwtPlot::xBottom,QwtPlot::yLeft,
						qwtPlot->canvas());
	d_zoomer->setRubberBand(QwtPicker::RectRubberBand);
	d_zoomer->setRubberBandPen(QColor(Qt::blue));					//鼠标选中框为蓝色
	d_zoomer->setTrackerMode(QwtPicker::ActiveOnly);
	d_zoomer->setTrackerPen(QColor(Qt::blue));						//鼠标选中坐标为蓝色
																	//加入拖移功能
	d_panner = new QwtPlotPanner(qwtPlot->canvas());
	d_panner->setMouseButton(Qt::MidButton);						//鼠标中间滑轮用于拖移

//	//设置通道名
//	qwtPlot->setTitle(QStringLiteral("ch")+ QString::number(i));

	parentWindow = parent;
	enableZoomMode(true);											//放大、平移功能

	xValues = NULL;
	yValues = NULL;

	H_trim = 20;													//正常显示时，底边宽度留白（全屏时为60）
																	//加入曲线
	qwtPlotCurve = new QwtPlotCurve;
	qwtPlotCurve->setStyle(QwtPlotCurve::Steps);
	qwtPlotCurve->setRenderHint(QwtPlotCurve::RenderAntialiased);
	qwtPlotCurve->attach(qwtPlot);
}

void PlotWindow::show()
{
	const int W_trim = 20;											//右侧宽度留白20
	qwtPlot->setFixedSize(width()-W_trim,height()-H_trim);			//固定尺寸
	QWidget::show();
//	qwtPlot->setMinimumSize(width()-W_trim,height()-H_trim);		//绘图部分最小尺寸
}

void PlotWindow::enableZoomMode(bool on)							//设置放大、平移功能
{
	d_panner->setEnabled(on);

	d_zoomer->setEnabled(on);
	d_zoomer->zoom(0);
}

void PlotWindow::setMaxX(int xnum)
{
	qwtPlot->setAxisScale(qwtPlot->xBottom,0,xnum);					//设置x轴范围，采样点数乘以每组脉冲数
	qwtPlot->setAxisAutoScale(qwtPlot->yLeft,true);					//y轴自动缩放

	if(xValues != NULL || yValues != NULL)							//x,y清零
	{
		delete [] xValues;
		delete [] yValues;
		xValues = NULL;
		yValues = NULL;
	}
	xValues = new double[xnum];										//x,y所需的点数
	yValues = new double[xnum];

	for(int i = 0; i<xnum; i++)										//x,y进行初始赋值，x，y=i，设定为斜线
		xValues[i] = i;
	for(int i = 0; i<xnum; i++)
		yValues[i] = 0;

	qwtPlotCurve->setSamples(xValues,yValues,xnum);
	d_zoomer->setZoomBase(true);									//原始坐标轴基础范围设置
	qwtPlot->replot();
}

void PlotWindow::datashow(const qint16 *datas,uint snum,uint pnum)	//绘图数据显示
{
	for(uint i = 0;i < snum;i++)
		yValues[i] = *(datas + (snum*(pnum-1)+i));
	if(d_zoomer->zoomRectIndex() == 0)
	{
		qwtPlotCurve->setSamples(xValues,yValues,snum);
		qwtPlot->setAxisAutoScale(qwtPlot->xBottom,true);
		qwtPlot->setAxisAutoScale(qwtPlot->yLeft,true);
		d_zoomer->setZoomBase(true);
		qwtPlot->replot();
	}
}

void PlotWindow::set_titleName(QString ch_name)						//设置各通道名
{
		qwtPlot->setTitle(ch_name);
}

void PlotWindow::keyPressEvent(QKeyEvent *)                         //处理在键盘按键事件
{
}

void PlotWindow::timerEvent(QTimerEvent *)
{
}

void PlotWindow::resizeEvent(QResizeEvent *)						//绘图窗口大小改变时
{
	if(height()>=1060)
		H_trim = 60;
	else
		H_trim = 20;
	show();
}
