#-------------------------------------------------
#
# Project created by QtCreator 2021-04-25T14:55:28
#
#-------------------------------------------------
#包含公共配置文件
include(../../GlobalPath.pri)
#-------------------------------------------------

TARGET = XFS_PPR

QT += widgets
QT -= gui

#隐藏控制台窗口: 去掉console配置
#CONFIG += console
CONFIG += c++11
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS += -ldl \
        -lrt


SOURCES += main.cpp \
    XFS_PPR.cpp \
    ./PPRFORM/PTRForm.cpp\
    ./PPRFORM/PTRData.cpp\
    ./PPRFORM/TextPrinter.cpp

HEADERS  += \
    XFS_PPR.h \
    def.h \
    mng_transdev.h \
    ./PPRFORM/PTRForm.h\
    ./PPRFORM/PTRData.h\
    ./PPRFORM/TextPrinter.h\
    ./PPRFORM/PTRDec.h\
    ../../CommonFile/include/IDevPPR.h

#配置依赖库目录
CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/CommonFile/lib/Debug -lfile_access -ldata_convertor -lcjson_object
}else:CONFIG(release, debug|release) {
    LIBS += -L$$PWD/CommonFile/lib/Release -lfile_access -ldata_convertor -lcjson_object
    #release版本去掉断言
    DEFINES += NDEBUG
}

LIBS += -lusb-1.0

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
