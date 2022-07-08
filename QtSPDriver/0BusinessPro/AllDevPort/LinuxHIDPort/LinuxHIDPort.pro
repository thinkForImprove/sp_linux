#-------------------------------------------------
#
# Project created by QtCreator 2019-07-06T08:59:40
#
#-------------------------------------------------
#包含公共配置文件
include(../../GlobalPathBin.pri)
#-------------------------------------------------

QT    -= gui

TARGET = LinuxHIDPort
TEMPLATE = lib

DEFINES += IDEVPORT_LIBRARY

#包含USB配置文件
#LIBS += -L/usr/lib/x86_64-linux-gnu -lusb-1.0
LIBS += -lusb-1.0

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
        LinuxHIDPort.cpp \
        hid-libusb.c \
        ../../../3CommonFile/CPP/DevPortLogFile.cpp

HEADERS += \
        LinuxHIDPort.h \
        hidapi.h \
        IDevPort.h \
        DevPortLogFile.h\


unix {
    target.path = /usr/lib
    INSTALLS += target
}
