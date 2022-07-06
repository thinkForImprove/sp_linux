#-------------------------------------------------
#
# Project created by QtCreator 2019-06-28T09:40:51
#
#-------------------------------------------------
#包含公共配置文件
include(../GlobalPath.pri)
#-------------------------------------------------

QT       -= gui

TARGET = QtPostMessage
TEMPLATE = lib

DEFINES += QTPOSTMESSAGE_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        QtPostMessage.cpp

HEADERS += \
        QtPostMessage.h \
        ../../2DriverAndSP/CommonFile/include/IQtPostMessage.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

#Add dbus-1.0 headfile directory
INCLUDEPATH += /usr/include/dbus-1.0/
INCLUDEPATH += /usr/lib/aarch64-linux-gnu/dbus-1.0/include/
