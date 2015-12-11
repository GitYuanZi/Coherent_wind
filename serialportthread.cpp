﻿#include "serialportthread.h"
#include <QtSerialPort/QSerialPort>
#include <QDebug>

SerialPortThread::SerialPortThread(QObject *parent)
	: QThread(parent), waitTimeout(0), quit(false)
{
}

SerialPortThread::~SerialPortThread()
{
	mutex.lock();
	quit = true;
	cond.wakeOne();
	mutex.unlock();
	wait();
}

void SerialPortThread::transaction(const QString &portName, const QString &request)
{
	QMutexLocker locker(&mutex);
	this->portName = portName;
	this->waitTimeout = 200;
	this->request = request;
	if(!isRunning())
		start();
	else
		cond.wakeOne();
}

void SerialPortThread::run()
{
	bool currentPortNameChanged = false;

	mutex.lock();
	QString currentPortName;
	if(currentPortName != portName)
	{
		currentPortName = portName;
		currentPortNameChanged = true;
		qDebug() << "currentPortName" << currentPortName;
	}
	int currentWaitTimeout = waitTimeout;
	QString currentRequest = request;
	qDebug() << "currentRequest = " << currentRequest;
	mutex.unlock();
	QSerialPort serial;
	while(!quit)
	{
		if(currentPortNameChanged)
		{
			serial.close();
			serial.setPortName(currentPortName);
			qDebug() << "serial.currentPortName" << currentPortName;
			if(!serial.open(QIODevice::ReadWrite))
			{
				qDebug() << "Can't open "<< currentPortName;
				emit this->S_PortNotOpen();						//串口未成功打开，信号返回到主程序
				return;
			}
			serial.setBaudRate(QSerialPort::Baud19200);			//波特率
			serial.setDataBits(QSerialPort::Data8);				//数据位
			serial.setParity(QSerialPort::NoParity);			//校验位
			serial.setStopBits(QSerialPort::OneStop);			//停止位
			serial.setFlowControl(QSerialPort::NoFlowControl);	//流控制
		}
		QByteArray requestData = currentRequest.toLocal8Bit();
		qDebug() << "requestData = " << requestData;
		serial.write(requestData);
		if(serial.waitForBytesWritten(waitTimeout))
		{
			if(serial.waitForReadyRead(currentWaitTimeout))
			{
				QByteArray responseData = serial.readAll();
				while(serial.waitForReadyRead(50))
					responseData += serial.readAll();
				QString response(responseData);
				emit this->response(response);
				qDebug() << "response = " << response;
			}
			else
			{
//				emit this->timeout();
				return;
			}
		}
		else
		{
			emit this->timeout();
			return;
		}
		mutex.lock();
		cond.wait(&mutex);
		if(currentPortName != portName)
		{
			currentPortName = portName;
			currentPortNameChanged = true;
		}
		else
			currentPortNameChanged = false;
		qDebug() << "last";
		currentWaitTimeout = waitTimeout;
		currentRequest = request;
		mutex.unlock();
	}
}
