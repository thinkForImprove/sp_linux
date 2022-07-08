#-------------------------------------------------
#
# Project created by QtCreator 2021-2-26T09:45:01
#
#-------------------------------------------------
#包含公共配置文件
include(../../GlobalPath.pri)
#-------------------------------------------------
QT       -= gui

TARGET = DevFIG
TEMPLATE = lib

DEFINES += IDEVFIG_LIBRARY

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#配置包含目录
#INCLUDEPATH += $$PWD/../../AllDevPort/include
INCLUDEPATH += $$PWD/DevFIG_SM205BCT/
INCLUDEPATH += $$PWD/DevFIG_HX/
INCLUDEPATH += $$PWD/DevFIG_TCM042/
INCLUDEPATH += $$PWD/DevFIG_WL
#LIBS += -L/usr/local/CFES/lib/ -lID_FprCap -lID_Fpr -lpthread -ludev -lusb-1.0

SOURCES += \
        ../../../../3CommonFile/CPP/DevPortLogFile.cpp \
        DevFIG.cpp \
        DevFIG_HX/DevFIG_HX.cpp \
        DevFIG_TCM042/DevFIG_TCM042.cpp \
        DevFIG_TCM042/DevImpl_TCM042.cpp \
        DevFIG_HX/DevImpl_HX.cpp \
        DevFIG_SM205BCT/DevFIG_SM205BCT.cpp \
        DevFIG_SM205BCT/DevImpl_SM205BCT.cpp \
        DevFIG_WL/DevFIG_WL.cpp \
        DevFIG_WL/DevImpl_WL.cpp \



HEADERS += \
        DevPortLogFile.h \
        IDevFIG.h \
        DevFIG.h \
        DevFIG_HX/DevFIG_HX.h \
        DevFIG_TCM042/DevFIG_TCM042.h \
        DevFIG_TCM042/DevImpl_TCM042.h \
        DevFIG_HX/DevImpl_HX.h \
        DevFIG_SM205BCT/DevFIG_SM205BCT.h \
        DevFIG_SM205BCT/DevImpl_SM205BCT.h \
        DevFIG_WL/DevFIG_WL.h \
        DevFIG_WL/DevImpl_WL.h \

LIBS += -lusb-1.0
CONFIG(debug, debug|release) {
    LIBS += -lfile_accessd -ldata_convertord
}else:CONFIG(release, debug|release) {
    LIBS += -lfile_access -ldata_convertor
}

unix {
    target.path = /usr/lib
    INSTALLS += target
}

