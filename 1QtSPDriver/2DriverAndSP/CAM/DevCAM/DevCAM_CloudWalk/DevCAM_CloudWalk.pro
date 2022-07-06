#-------------------------------------------------
#
# Project created by QtCreator 2020-12-25T00:18:01
#
#-------------------------------------------------
#包含公共配置文件
include(../../../GlobalPath.pri)
#-------------------------------------------------
#QT       += core gui
#QT       += widgets
#QT       -= gui

QT       += widgets dbus core network gui

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DevCAM
TEMPLATE = lib

#DEFINES += DEVCAM_CLOUDWALK_LIBRARY
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
INCLUDEPATH += $$PWD/../../../AllDevPort/include
INCLUDEPATH += \
    /usr/local/include/cloudwalk \
    /usr/local/include/cloudwalk/opencv \
    /usr/local/include/cloudwalk/opencv/opencv \
    /usr/local/include/cloudwalk/opencv/opencv2


SOURCES += \
        ../../../AllDevPort/include/DevPortLogFile.cpp \
        DevCAM_CloudWalk.cpp \
        DevImpl_CloudWalk.cpp \
        cwframewidget.cpp \
    DevCAM_Show.cpp

HEADERS += \
        ../../../AllDevPort/include/DevPortLogFile.h \
        ../../../CommonFile/include/IDevCAM.h \
        DevCAM_CloudWalk.h \
        DevImpl_CloudWalk.h \
        cloudwalk.h \
        cwframewidget.h \
        DevCAM_Show.h

FORMS += \
    DevCAM_Show.ui

LIBS += -L/usr/local/lib/cloudwalk  \
    -lcwlivdetengine \
    -lcwlivenessdetector \
    -lcwCameraDevice\
    -lopencv_world \
    -lcwCrypt \
    -lCwLicenseChip \
    -lcwbase \
    -lcwlog \
    -lDetectLivenessModule \
    -lcwauthorize_x64 \
    -lCwChip \
    -ljpeg

unix {
    target.path = /usr/lib
    INSTALLS += target
}
