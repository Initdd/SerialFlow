QT += testlib serialport
QT -= gui

CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += tst_serialportmanager.cpp \
           ../src/serialportmanager.cpp

HEADERS += ../src/serialportmanager.h

INCLUDEPATH += ../src

TARGET = tst_serialportmanager
