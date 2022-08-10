#-------------------------------------------------
#
# Project created by QtCreator 2020-12-25T00:18:01
#
#-------------------------------------------------
#包含公共配置文件
include(../../GlobalPath.pri)
#-------------------------------------------------
#使用优化版本
DEFINES += SPBuild_OPTIM
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


SOURCES += \
        DevIDX.cpp

HEADERS += \
        IDevIDC.h \
        DevIDX.h \
        ../XFS_IDX/def.h

SOURCES += ComFile/ComFile.cpp
HEADERS += ComFile/ComFile.h


#BSID81
SOURCES += \
        DevIDX_BSID81/DevImpl_BSID81.cpp \
        DevIDX_BSID81/DevIDX_BSID81.cpp

HEADERS += \
        DevIDX_BSID81/DevImpl_BSID81.h \
        DevIDX_BSID81/DevIDX_BSID81.h \
        DevIDX_BSID81/libID81_SO.h

#DMT-M1-M
SOURCES += \
        DevIDX_DMTM1M/DevImpl_DMTM1M.cpp \
        DevIDX_DMTM1M/DevIDX_DMTM1M.cpp

HEADERS += \
        DevIDX_DMTM1M/DevImpl_DMTM1M.h \
        DevIDX_DMTM1M/DevIDX_DMTM1M.h \
        DevIDX_DMTM1M/libDMTM1M.h \
        DevIDX_DMTM1M/libwlt.h

#配置依赖库目录
CONFIG(debug, debug|release) {
    LIBS += -lfile_accessd -ldata_convertord -lwlt -lusb-1.0
}else:CONFIG(release, debug|release) {
    LIBS += -lfile_access -ldata_convertor -lwlt -lusb-1.0
}


unix {
    target.path = /usr/lib
    INSTALLS += target
}
