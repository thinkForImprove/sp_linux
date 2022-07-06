include(../../GlobalPath.pri)

TARGET = XFS_PTR

QT += widgets
QT -= gui
#隐藏控制台窗口: 去掉console配置
#CONFIG += console
CONFIG += c++11
CONFIG -= app_bundle

LIBS += -ldl \
        -lrt


SOURCES += main.cpp \
    XFS_PTR.cpp \
    PTRForm.cpp\
    PTRData.cpp\
    TextPrinter.cpp

HEADERS  += \
    XFS_PTR.h \
    mng_transdev.h \
    PTRForm.h\
    PTRData.h\
    TextPrinter.h\
    PTRDec.h\
    ../../CommonFile/include/IDevPTR.h
