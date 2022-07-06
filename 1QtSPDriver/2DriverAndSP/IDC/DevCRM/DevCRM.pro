#-------------------------------------------------
#
# Project created by QtCreator 2019-06-19T09:21:33
#
#-------------------------------------------------
#包含公共配置文件
include(../../GlobalPath.pri)
#-------------------------------------------------

QT       -= gui

TARGET = DevCRM
TEMPLATE = lib

DEFINES += IDEVIDC_LIBRARY

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
    DevCRM.cpp \
    CRT-730B/DevCRM_CRT730B.cpp \
    CRT-730B/DevImpl_CRT730B.cpp

HEADERS += \
    ../../CommonFile/include/IDevCRM.h\
    ../../CommonFile/include/IAllDevPort.h \
    DevCRM.h \
    CRT-730B/DevCRM_CRT730B.h \
    CRT-730B/DevImpl_CRT730B.h


unix {
    target.path = /usr/lib
    INSTALLS += target
}
