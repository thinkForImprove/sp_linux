#-------------------------------------------------
#
# Project created by QtCreator 2019-07-18T13:13:24
#
#-------------------------------------------------
#包含公共配置文件
include(../../GlobalPath.pri)
#-------------------------------------------------

QT       -= gui

TARGET = DevPIN
TEMPLATE = lib

DEFINES += IDEVPIN_LIBRARY

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
    XZF35/DevPIN_XZF35.cpp \
    ZT598/DES.cpp \
    ZT598/DevPIN_ZT598.cpp \
    DevPIN.cpp \
    ZT598/Encrypt.cpp

HEADERS += \
    IDevPIN.h \
    IAllDevPort.h \
    DevPIN.h \
    XZF35/DevPIN_XZF35.h \
    ZT598/DES.h \
    ZT598/DevPIN_ZT598.h \
    DevPIN.h \
    XZF35/XZF35Def.h \
    ZT598/Encrypt.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
