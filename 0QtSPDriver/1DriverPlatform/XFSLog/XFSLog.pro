include(../GlobalPath.pri)

QT       -= gui

TARGET = XFSLog
TEMPLATE = lib

DEFINES += XFSLOG_LIBRARY

# 平安银行头文件差异使用
DEFINES += PINGAN_VERSION


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
