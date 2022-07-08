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
    PTRFORM/PTRDec.h \
    ID_Fpr.h \
    ID_FprCap.h
LIBS += -lusb-1.0
CONFIG(debug, debug|release) {
    LIBS += -lfile_accessd -ldata_convertord
}else:CONFIG(release, debug|release) {
    LIBS += -lfile_access -ldata_convertor
}
