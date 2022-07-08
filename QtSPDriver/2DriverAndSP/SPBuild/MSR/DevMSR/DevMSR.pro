#-------------------------------------------------
#
# Project created by QtCreator 2020-12-25T00:18:01
#
#-------------------------------------------------
#包含公共配置文件
include(../../GlobalPath.pri)
#-------------------------------------------------
QT       -= gui

TARGET = DevMSR
TEMPLATE = lib

DEFINES += IDEVMSR_LIBRARY

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
        DevMSR.cpp

HEADERS += \
        IDevMSR.h \
        ../XFS_MSR/def.h \
        DevMSR.h \

#WBC-S10
SOURCES += \
        DevMSR_WBCS10/DevImpl_WBCS10.cpp \
        DevMSR_WBCS10/DevMSR_WBCS10.cpp

HEADERS += \
        DevMSR_WBCS10/wb-cs10.h \
        DevMSR_WBCS10/DevImpl_WBCS10.h \
        DevMSR_WBCS10/DevMSR_WBCS10.h

#WBT-2000
SOURCES += \
        DevMSR_WBT2000/DevImpl_WBT2000.cpp \
        DevMSR_WBT2000/DevMSR_WBT2000.cpp

HEADERS += \
        DevMSR_WBT2000/DevImpl_WBT2000.h \
        DevMSR_WBT2000/DevMSR_WBT2000.h


#配置依赖库目录
CONFIG(debug, debug|release) {
    LIBS += -lcjson_objectd -lfile_accessd
}else:CONFIG(release, debug|release) {
    LIBS += -lcjson_object -lfile_access
}

unix {
    target.path = /usr/lib
    INSTALLS += target
}
