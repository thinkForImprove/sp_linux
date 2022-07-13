#-------------------------------------------------
#
# Project created by QtCreator 2019-06-19T09:21:33
#
#-------------------------------------------------
#包含公共配置文件
include(../../GlobalPath.pri)
#-------------------------------------------------

QT       -= gui

TARGET = DevCPR
TEMPLATE = lib

DEFINES += IDEVCPR_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD/../XFS_CPR

SOURCES += \
    DevCPR.cpp \
    BT8500M/DevCPR_BT8500M.cpp \
    BT8500M/DevImpl_BT8500M.cpp

HEADERS += \
    IDevPTR.h \
    DevCPR.h \
    ../XFS_CPR/def.h \
    BT8500M/DevCPR_BT8500M.h \
    BT8500M/DevImpl_BT8500M.h

#配置依赖库目录
CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/CommonFile/lib/Debug -lcjson_objectd -ldata_convertord
}else:CONFIG(release, debug|release) {
    LIBS += -L$$PWD/CommonFile/lib/Release -lcjson_object -ldata_convertor
}

unix {
    target.path = /usr/lib
    INSTALLS += target
}