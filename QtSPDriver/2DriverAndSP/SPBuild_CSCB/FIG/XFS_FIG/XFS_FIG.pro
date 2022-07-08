include(../../GlobalPath.pri)

TARGET = XFS_FIG

QT += widgets
QT -= gui
#隐藏控制台窗口: 去掉console配置
#CONFIG += console
CONFIG += c++11
CONFIG -= app_bundle

SOURCES += \
        main.cpp \
    XFS_FIG.cpp

HEADERS += \
    XFS_FIG.h \
    IDevFIG.h \
    ISPBaseFIG.h \
    FIGData.h \
    PTRDec.h \
    ID_Fpr.h \
    ID_FprCap.h
