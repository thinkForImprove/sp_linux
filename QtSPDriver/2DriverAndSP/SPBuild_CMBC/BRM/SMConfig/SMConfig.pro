include(../../GlobalPath.pri)

QT       += core

TARGET = SMConfig
TEMPLATE = lib
#版本号
#VERSION += 2.1.2

DEFINES += SMCONFIG_LIBRARY

SOURCES += \
    tinystr.cpp \
    tinyxml.cpp \
    tinyxmlerror.cpp \
    tinyxmlparser.cpp

HEADERS +=\
        tinystr.h \
        tinyxml.h\
../../CommonFile/Include/QtTypeDef.h\

