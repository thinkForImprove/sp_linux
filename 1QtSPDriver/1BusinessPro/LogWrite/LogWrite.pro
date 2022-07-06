#-------------------------------------------------
#
# Project created by QtCreator 2019-05-23T13:16:23
#
#-------------------------------------------------
#包含公共配置文件
include(../GlobalPath.pri)
#-------------------------------------------------

QT       -= gui

TARGET = LogWrite
TEMPLATE = lib
DEFINES += LOGWRITE_LIBRARY


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
        LogWrite.cpp \
        ../CommonFile/include/QtShareMemoryRW.cpp \
        ../CommonFile/include/QtAppRunning.cpp\
        ../CommonFile/include/SimpleMutex.cpp \
    ../CommonFile/include/INIFileReader.cpp

HEADERS += \
        LogWrite.h \
        ../CommonFile/include/ILogWrite.h \
        ../CommonFile/include/QtShareMemoryRW.h\
        ../CommonFile/include/QtAppRunning.h\
        ../CommonFile/include/SimpleMutex.h \
    ../CommonFile/include/INIFileReader.h


unix {
    target.path = /usr/lib
    INSTALLS += target
}
