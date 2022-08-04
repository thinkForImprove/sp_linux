#-------------------------------------------------
#
# Project created by QtCreator 2019-06-19T09:21:33
#
#-------------------------------------------------
#包含公共配置文件
include(../../GlobalPath.pri)
#-------------------------------------------------
#使用优化版本
DEFINES += SPBuild_OPTIM
#-------------------------------------------------

QT       -= gui

TARGET = DevBCR
TEMPLATE = lib

DEFINES += IDEVBCR_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#
SOURCES += \
    DevBCR.cpp

HEADERS += \
    IDevBCR.h\
    IAllDevPort.h \
    DevBCR.h

#NT0861(牛图)
SOURCES += \
    DevBCR_NT0861/DevBCR_NT0861.cpp \
    DevBCR_NT0861/DevImpl_NT0861.cpp

HEADERS += \
    DevBCR_NT0861/DevBCR_NT0861.h \
    DevBCR_NT0861/DevImpl_NT0861.h \
    DevBCR_NT0861/DevDef_NT0861.h

#配置依赖库目录
CONFIG(debug, debug|release) {
    LIBS += -lfile_accessd -ldata_convertord -ldevice_objectd -ludev
}else:CONFIG(release, debug|release) {
    LIBS += -lfile_access -ldata_convertor -ldevice_object -ludev
}

unix {
    target.path = /usr/lib
    INSTALLS += target
}
