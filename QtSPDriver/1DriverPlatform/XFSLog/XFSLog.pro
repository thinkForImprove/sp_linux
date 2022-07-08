#-------------------------------------------------
#
# Project created by QtCreator 2019-07-05T16:07:00
#
#-------------------------------------------------
#包含公共配置文件
include(../GlobalPathBin.pri)
#-------------------------------------------------

QT       -= gui

TARGET = XFSLog
TEMPLATE = lib

DEFINES += XFSLOG_LIBRARY


SOURCES += \
    XFSLogManager.cpp \
    XFSBufferLogger.cpp \
    XFSDataDesc.cpp \
    XFSFileLogManager.cpp \
    XFSLogThread.cpp \
    LogBackupManager.cpp \
    LogThread.cpp\
    StringBuffer.cpp\

HEADERS += \
    XFSLogManager.h \
    XFSBufferLogger.h \
    XFSDataDesc.h \
    XFSFileLogManager.h \
    XFSLogThread.h \
    LogBackupManager.h \
    LogThread.h\
    StringBuffer.h\

unix {
    target.path = /usr/lib
    INSTALLS += target
}
