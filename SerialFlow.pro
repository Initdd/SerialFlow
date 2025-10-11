#-------------------------------------------------
# Project setup
#-------------------------------------------------
QT += core widgets serialport

CONFIG += c++17
CONFIG += qt warn_on release

TARGET = SerialFlow
TEMPLATE = app

VERSION = 1.0

#-------------------------------------------------
# Source files
#-------------------------------------------------
SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/serialportmanager.cpp \
    src/settingsdialog.cpp

#-------------------------------------------------
# Header files
#-------------------------------------------------
HEADERS += \
    src/mainwindow.h \
    src/serialportmanager.h \
    src/settingsdialog.h

#-------------------------------------------------
# UI files
#-------------------------------------------------
FORMS += \
    forms/mainwindow.ui \
    forms/settingsdialog.ui

#-------------------------------------------------
# Resource files
#-------------------------------------------------
FORMS += \
    forms/mainwindow.ui \
    forms/settingsdialog.ui

#-------------------------------------------------
# Resource files
#-------------------------------------------------
RESOURCES += \
    resources/resources.qrc

#-------------------------------------------------
# Include directories
#-------------------------------------------------
INCLUDEPATH += src

#-------------------------------------------------
# Installation (optional)
#-------------------------------------------------
# You can define an install path if you want
# target.path = $$[QT_INSTALL_BINS]
# INSTALLS += target
