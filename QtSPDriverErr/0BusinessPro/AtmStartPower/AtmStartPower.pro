#-------------------------------------------------
#
# Project created by QtCreator 2019-10-29T16:49:58
#
#-------------------------------------------------
#包含公共配置文件
include(../GlobalPathBin.pri)
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AtmStartPower
TEMPLATE = app


#程序图标
RC_ICONS = ./res/icon.ico

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        AtmStartPower.cpp \
#        USBDrive.cpp \
        ../../3CommonFile/CPP/QtAppRunning.cpp \
        ../../3CommonFile/CPP/SimpleMutex.cpp \
        ../../3CommonFile/CPP/INIFileReader.cpp

HEADERS += \
        AtmStartPower.h \
#        IOMCUsbdef.h \
#        UsbDLLDef.h \
#        USBDrive.h \
        ILogWrite.h \
        ../../3CommonFile/CPP/QtAppRunning.h \
        ../../3CommonFile/CPP/SimpleMutex.h \
        ../../3CommonFile/CPP/INIFileReader.h

FORMS += \
        AtmStartPower.ui

RESOURCES += \
    AtmStartPower.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

CONFIG(debug, debug|release){
    LIBS += -lShareHeaderFiled -lIomcCommd
}else:CONFIG(release, debug|release){
    LIBS += -lShareHeaderFile -lIomcComm
}
