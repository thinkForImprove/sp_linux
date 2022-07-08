#-------------------------------------------------
#
# Project created by QtCreator 2019-06-19T09:21:33
#
#-------------------------------------------------
#包含公共配置文件
include(../../GlobalPath.pri)
#-------------------------------------------------

QT       -= gui

TARGET = DevPPR
TEMPLATE = lib

DEFINES += IDEVPTR_LIBRARY


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
    DevPPR.cpp \
    MB2/DevPPR_MB2.cpp \
    MB2/DevImpl_MB2.cpp \
    PRM/DevPPR_PRM.cpp \
    PRM/DevImpl_PRM.cpp

HEADERS += \
    IDevPTR.h\
    ../XFS_PPR/def.h \
    DevPPR.h \
    MB2/DevPPR_MB2.h \
    MB2/DevImpl_MB2.h \
    PRM/DevPPR_PRM.h \
    PRM/DevImpl_PRM.h

LIBS += -lusb-1.0

#配置依赖库目录
CONFIG(debug, debug|release) {
    LIBS += -lcjson_objectd -ldata_convertord
} else:CONFIG(release, debug|release) {
    LIBS += -lcjson_object -ldata_convertor
}

unix {
    target.path = /usr/lib
    INSTALLS += target
}
