#-------------------------------------------------
#
# Project created by QtCreator 2019-06-29T16:50:19
#
#-------------------------------------------------
#包含公共配置文件
include(../../GlobalPath.pri)
#-------------------------------------------------

QT       -= gui

TARGET = SPBaseCRS
TEMPLATE = lib

DEFINES += SPBASECRS_LIBRARY


# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += SPBaseCRS.cpp \
            FMTResData.cpp

HEADERS += SPBaseCRS.h\
           FMTResData.h\
           ../../../2DriverAndSP/CommonFile/include/ISPBaseCRS.h



unix {
    target.path = /usr/lib
    INSTALLS += target
}

#配置依赖库目录
CONFIG(debug, debug|release) {
LIBS += -L$$PWD/../../../2DriverAndSP/CommonFile/lib/Debug -lXfsSPIHelperd
LIBS += -L$$PWD/../../../2DriverAndSP/CommonFile/lib/Debug -lShareHeaderFiled
}else:CONFIG(release, debug|release) {
LIBS += -L$$PWD/../../../2DriverAndSP/CommonFile/lib/Release -lXfsSPIHelper
LIBS += -L$$PWD/../../../2DriverAndSP/CommonFile/lib/Release -lShareHeaderFile
}
