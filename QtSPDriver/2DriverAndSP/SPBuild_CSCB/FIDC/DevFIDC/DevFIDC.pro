#-------------------------------------------------
#
# Project created by QtCreator 2019-06-19T09:21:33
#
#-------------------------------------------------
#包含公共配置文件
include(../../GlobalPath.pri)
#-------------------------------------------------

QT       -= gui

TARGET = DevFIDC
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
    CJ201/DevFIDC_CJ201.cpp \
    DevFIDC.cpp \
    MT50/DevFIDC_MT50.cpp \
    MT50/MTDriver.cpp \
    TMZ/DevFIDC_TMZ.cpp \
    TMZ/TMZDriver.cpp


HEADERS += \
    IDevIDC.h\
    IAllDevPort.h \
    CJ201/DevFIDC_CJ201.h \
    DevFIDC.h \
    MT50/DevFIDC_MT50.h \
    MT50/MTDriver.h \
    MT50/mtx.h \
    TMZ/DevFIDC_TMZ.h \
    TMZ/TMZDriver.h

LIBS += -L/usr/local/CFES/lib
LIBS += -lCnSysReader

unix {
    target.path = /usr/lib
    INSTALLS += target
}
