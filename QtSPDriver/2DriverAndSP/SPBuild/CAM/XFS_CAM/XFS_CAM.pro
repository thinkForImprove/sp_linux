#-------------------------------------------------
#
# Project created by QtCreator 2021-01-20T14:50:04
#
#-------------------------------------------------
#包含公共配置文件
include(../../GlobalPath.pri)
#-------------------------------------------------

QT       += core gui widgets dbus network

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = XFS_CAM
#TEMPLATE = app

#隐藏控制台窗口: 去掉console配置
#CONFIG += console
CONFIG += c++11
CONFIG -= app_bundle

LIBS += -ldl \
        -lrt

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += PINGAN_VERSION(在GlobalPath.pri配置,该位置不再使用)

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#配置包含目录
INCLUDEPATH += \
    /usr/local/include/cloudwalk \
    /usr/local/include/cloudwalk/opencv \
    /usr/local/include/cloudwalk/opencv/opencv \
    /usr/local/include/cloudwalk/opencv/opencv2


SOURCES += \
        main.cpp \
        mainwindow.cpp \
        XFS_CAM.cpp \
        XFS_CAM_DEC.cpp \
        cwframewidget.cpp

HEADERS += \
        mainwindow.h \
        XFS_CAM.h \
        cwframewidget.h \
        ComInfo.h \
        IDevCAM.h

FORMS += \
        mainwindow.ui




# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

#配置依赖库目录
CONFIG(debug, debug|release) {
    LIBS += -lfile_accessd -ldata_convertord
}else:CONFIG(release, debug|release) {
    LIBS += -lfile_access -ldata_convertor
}

#Opencv库依赖设定
LIBS += -lopencv_core \
        -lopencv_imgproc \
        -lopencv_imgcodecs


