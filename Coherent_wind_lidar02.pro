#-------------------------------------------------
#
# Project created by QtCreator 2015-07-15T14:51:26
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Coherent_wind_lidar02
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    paradialog.cpp \
    informationleft.cpp \
    plotwidget.cpp \
    threadstore.cpp \
    settingfile.cpp

HEADERS  += mainwindow.h \
    paradialog.h \
    Acqsettings.h \
    informationleft.h \
    plotwidget.h \
    threadstore.h \
    settingfile.h

FORMS    += mainwindow.ui \
    paradialog.ui \
    informationleft.ui

win32: LIBS += -LD:/QtProjectFiles/Coherent_wind_lidar02/ -lADQAPI

RESOURCES += \
    images.qrc

CONFIG    += qwt

RC_ICONS = images\Convert.ico
