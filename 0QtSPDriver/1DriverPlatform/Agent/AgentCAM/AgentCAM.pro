#-------------------------------------------------
#
# Project created by QtCreator 2020-11-18T19:15:14
#
#-------------------------------------------------
#包含公共配置文件
include(../../GlobalPath.pri)
#-------------------------------------------------

QT       -= gui

TARGET = AgentCAM
TEMPLATE = lib

DEFINES += AGENTCAM_LIBRARY

# 平安银行头文件差异使用
DEFINES += PINGAN_VERSION

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
        AgentCAM.cpp

HEADERS += \
        AgentCAM.h \
        ../../../2DriverAndSP/CommonFile/include/IAgentBase.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
