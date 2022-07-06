include(../../GlobalPath.pri)

QT       -= gui
TARGET = MixManager
TEMPLATE = lib
#版本号
#VERSION += 1.0.0

#LIBS += -llog_lib

DEFINES += MIXMANAGER_LIBRARY

SOURCES += \
    MixMng.cpp\

HEADERS +=\
#mixmanager_global.h \
        MixMng.h\
../../CommonFile/Include/IMixManager.h\
../../CommonFile/Include/ILogWrite.h\

