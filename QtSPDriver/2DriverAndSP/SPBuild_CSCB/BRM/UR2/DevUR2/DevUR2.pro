include(../../../GlobalPath.pri)
QT       -= gui
QT       += core
TARGET = DevUR2
TEMPLATE = lib

DEFINES += DEVUR2_LIBRARY

#INCLUDEPATH += ../../../AllDevPort/include

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += DevUR2.cpp\
        DevExport.cpp\
        ErrCodeMap.cpp\
        VHUSBDrive.cpp\
        ../../../../../3CommonFile/CPP/DevPortLogFile.cpp

HEADERS += \
        DevUR2.h\
        ErrCodeMap.h\
        UsbDLLDef.h\
        VHUSBDrive.h\
        QtTypeDef.h\
        IURDevice.h\
        DevPortLogFile.h\
        ILogWrite.h\


CONFIG += c++11
CONFIG += warn_off
