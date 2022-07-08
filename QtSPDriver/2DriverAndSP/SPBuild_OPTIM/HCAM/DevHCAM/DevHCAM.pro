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

#QT       -= gui

#QT       += widgets dbus core network gui

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DevHCAM
TEMPLATE = lib

DEFINES += IDEVCAM_LIBRARY

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
INCLUDEPATH +=

#共通
SOURCES += \
        DevHCAM.cpp

HEADERS += \
        IDevCAM.h \
        ../XFS_HCAM/def.h \
        DevHCAM.h

#JDY-5001A-0809
SOURCES += \
        DevHCAM_JDY5001A0809/DevImpl_JDY5001A0809.cpp \
        DevHCAM_JDY5001A0809/DevHCAM_JDY5001A0809.cpp


HEADERS += \
        DevHCAM_JDY5001A0809/DevImpl_JDY5001A0809.h \
        DevHCAM_JDY5001A0809/DevHCAM_JDY5001A0809.h

#配置依赖库目录
CONFIG(debug, debug|release) {
    LIBS += -lfile_accessd -ldata_convertord
} else:CONFIG(release, debug|release) {
    LIBS += -lfile_access -ldata_convertor
}

#Opencv库依赖设定
LIBS += -lopencv_video \
        -lopencv_videoio \
        -ludev

unix {
    target.path = /usr/lib
    INSTALLS += target
}
