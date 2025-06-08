#-------------------------------------------------
#
# Project created by QtCreator 2025-06-07T18:01:04
#
#-------------------------------------------------

QT       += core gui


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Intelligent_classroom
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    controller.cpp \
    sensor.cpp \
    hardware.cpp \
    timefile.cpp \
    mqttcloud.cpp

HEADERS  += mainwindow.h \
    controller.h \
    global.h \
    sensor.h \
    hardware.h \
    timefile.h \
    mqttcloud.h

FORMS    += mainwindow.ui
