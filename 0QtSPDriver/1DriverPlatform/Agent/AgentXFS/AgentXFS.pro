#-------------------------------------------------
#
# Project created by QtCreator 2019-06-18T17:35:19
#
#-------------------------------------------------
#包含公共配置文件
include(../../GlobalPath.pri)
#-------------------------------------------------

QT       -= gui

TARGET = AgentXFS
TEMPLATE = lib

DEFINES += AGENTXFS_LIBRARY

#使用__stdcall类型导出的函数名不变
win32 {
QMAKE_LFLAGS += -Wl,--kill-at
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS += -lXFSLog

SOURCES += \
        AgentXFS.cpp

HEADERS += \
        AgentXFS.h\
        XFSLogHelper.h\
        ../../../2DriverAndSP/CommonFile/include/IAgentBase.h\
        ../../../2DriverAndSP/CommonFile/include/IQtPostMessage.h\
        ../../../2DriverAndSP/CommonFile/include/ILogWrite.h


#unix {
#LIBS += -lWhiteListCheck
#HEADERS += WhiteListCheck.h\
#}
unix {
    target.path = /usr/lib
    INSTALLS += target
}
