#-------------------------------------------------
#
# Project created by QtCreator 2020-12-25T00:18:01
#
#-------------------------------------------------
#包含公共配置文件
include(../../GlobalPath.pri)
#-------------------------------------------------
QT       -= gui

TARGET = DevIDX
TEMPLATE = lib

DEFINES += IDEVIDX_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#配置包含目录
#INCLUDEPATH += $$PWD/../../AllDevPort/include


SOURCES += \
        DevIDX.cpp \
        DevIDX_BSID81/DevImpl_BSID81.cpp \
        DevIDX_BSID81/DevIDX_BSID81.cpp


HEADERS += \
        IDevIDX.h \
        DevIDX.h \
        ../XFS_IDX/def.h \
        DevIDX_BSID81/DevImpl_BSID81.h \
        DevIDX_BSID81/DevIDX_BSID81.h \
        DevIDX_BSID81/libID81_SO.h

#配置依赖库目录
CONFIG(debug, debug|release) {
    LIBS += -ldata_convertord
}else:CONFIG(release, debug|release) {
    LIBS += -ldata_convertor
}

unix {
    target.path = /usr/lib
    INSTALLS += target
}
