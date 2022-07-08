#-------------------------------------------------
#
# Project created by QtCreator 2021-04-25T14:55:28
#
#-------------------------------------------------
#包含公共配置文件
include(../../GlobalPath.pri)
include(../../GlobalPathForm.pri)
#-------------------------------------------------

TARGET = XFS_JPR

QT += widgets
QT -= gui

#隐藏控制台窗口: 去掉console配置
#CONFIG += console
CONFIG += c++11
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS += -ldl \
        -lrt


SOURCES += main.cpp \
    XFS_JPR.cpp \
    XFS_JPR_DEC.cpp

HEADERS  += \
    XFS_JPR.h \
    mng_transdev.h \
    def.h \
    IDevPTR.h


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

