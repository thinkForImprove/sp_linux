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

TARGET = DevCAM
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
        DevCAM.cpp \
        DevCAM_DEF/DevCAM_DEF.cpp \
        ../../../../3CommonFile/CPP/ClassCommon.cpp

HEADERS += \
        IDevCAM.h \
        ../XFS_CAM/def.h \
        DevCAM.h \
        DevCAM_DEF/DevCAM_DEF.h \
        DevCAM_DEF/DevImpl_DEF.h \
        ClassCommon.h


#YC-0C98(加载CPP/H)
SOURCES += \
        DevCAM_CloudWalk/DevImpl_CloudWalk.cpp \
        DevCAM_CloudWalk/DevCAM_CloudWalk.cpp

HEADERS += \
        DevCAM_CloudWalk/DevImpl_CloudWalk.h \
        DevCAM_CloudWalk/DevCAM_CloudWalk.h \
        DevCAM_CloudWalk/cloudwalk.h

#TCF261(加载CPP/H)
SOURCES += \
        DevCAM_TCF261/DevCAM_TCF261.cpp \
        DevCAM_TCF261/DevImpl_TCF261.cpp

HEADERS += \
        DevCAM_TCF261/DevCAM_TCF261.h \
        DevCAM_TCF261/DevImpl_TCF261.h \
        DevCAM_TCF261/TCF261.h

#ZLF1000A3(加载CPP/H)
SOURCES += \
        DevCAM_ZLF1000A3/DevCAM_ZLF1000A3.cpp \
        DevCAM_ZLF1000A3/DevImpl_ZLF1000A3.cpp

HEADERS += \
        DevCAM_ZLF1000A3/DevCAM_ZLF1000A3.h \
        DevCAM_ZLF1000A3/DevImpl_ZLF1000A3.h

#JDY-5001A-0809(加载CPP/H)
SOURCES += \
        DevCAM_JDY5001A0809/DevImpl_JDY5001A0809.cpp \
        DevCAM_JDY5001A0809/DevCAM_JDY5001A0809.cpp

HEADERS += \
        DevCAM_JDY5001A0809/DevImpl_JDY5001A0809.h \
        DevCAM_JDY5001A0809/DevCAM_JDY5001A0809.h

#配置依赖库目录
CONFIG(debug, debug|release) {
    LIBS += -lfile_accessd -ldata_convertord -ldevice_objectd
} else:CONFIG(release, debug|release) {
    LIBS += -lfile_access -ldata_convertor -ldevice_object
}

#Opencv库依赖设定
LIBS += -lopencv_video \
        -lopencv_videoio \
       # -ludev

#云从摄像依赖加载
LIBS += -lcwlivdetengine \
        -lcwauthorize_x64

#LIBS += -lv4l1compat -lv4l2

unix {
    target.path = /usr/lib
    INSTALLS += target
}
