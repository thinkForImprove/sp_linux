#-------------------------------------------------
#
# Project created by QtCreator 2019-06-19T09:21:33
#
#-------------------------------------------------
#包含公共配置文件
include(../../GlobalPath.pri)
#-------------------------------------------------
#使用优化版本
DEFINES += SPBuild_OPTIM
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
    DevFIDC.cpp \

HEADERS += \
    IDevIDC.h\
    IAllDevPort.h \
    DevFIDC.h

#CJ201
#SOURCES += \
#    CJ201/DevFIDC_CJ201.cpp \

#HEADERS += \
#    CJ201/DevFIDC_CJ201.h

#MT50
SOURCES += \
    MT50/DevFIDC_MT50.cpp \
    MT50/DevImpl_MT50.cpp

HEADERS += \
    MT50/DevFIDC_MT50.h \
    MT50/DevImpl_MT50.h \
    MT50/mtx.h

#TMZ
SOURCES += \
    TMZ/DevFIDC_TMZ.cpp \
    TMZ/DevImpl_TMZ.cpp

HEADERS += \
    TMZ/DevFIDC_TMZ.h \
    TMZ/DevImpl_TMZ.h

#创自CRT-603-CZ7-6001
SOURCES += \
    CRT603CZ7/DevFIDC_CRT603CZ7.cpp \
    CRT603CZ7/DevImpl_CRT603CZ7.cpp

HEADERS += \
    CRT603CZ7/DevFIDC_CRT603CZ7.h \
    CRT603CZ7/DevImpl_CRT603CZ7.h


#配置依赖库目录
CONFIG(debug, debug|release) {
    LIBS += -lfile_accessd -ldata_convertord
}else:CONFIG(release, debug|release) {
    LIBS += -lfile_access -ldata_convertor
}

unix {
    target.path = /usr/lib
    INSTALLS += target
}
