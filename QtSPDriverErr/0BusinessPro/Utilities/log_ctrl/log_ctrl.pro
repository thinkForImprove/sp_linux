#-------------------------------------------------
#
# Project created by QtCreator 2019-07-05T16:07:00
#
#-------------------------------------------------
#包含公共配置文件
include(../GlobalPath.pri)
#-------------------------------------------------

QT -= gui

TEMPLATE = lib
DEFINES += LOG_CTRL_LIBRARY

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    elog.c \
    elog_async.c \
    elog_buf.c \
    elog_port.c \
    elog_utils.c \
    log_ctrl.cpp

HEADERS += \
    elog.h \
    elog_cfg.h \
    framework.h \
    log_ctrl_global.h \
    log_ctrl.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
    INSTALLS += target
}

# Setting the depending libs
LIBS += -lsynchronism -lfile_access
