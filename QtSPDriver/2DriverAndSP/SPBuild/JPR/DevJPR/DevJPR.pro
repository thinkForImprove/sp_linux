#-------------------------------------------------
#
# Project created by QtCreator 2020-12-25T00:18:01
#
#-------------------------------------------------
#包含公共配置文件
include(../../GlobalPath.pri)
#-------------------------------------------------

QT       += widgets dbus core network gui

TARGET = DevJPR
TEMPLATE = lib

DEFINES += IDEVJTR_LIBRARY

#配置包含目录
#INCLUDEPATH += $$PWD/../../AllDevPort/include

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
    DevJPR.cpp \
    BT_T080AII/DevJPR_BTT080AII.cpp \
    BT_T080AII/DevImpl_BTT080AII.cpp

HEADERS += \
    IDevPTR.h\
    ../XFS_JPR/def.h \
    DevJPR.h \
    BT_T080AII/DevJPR_BTT080AII.h \
    BT_T080AII/DevImpl_BTT080AII.h \
    BT_T080AII/POSSDK.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
