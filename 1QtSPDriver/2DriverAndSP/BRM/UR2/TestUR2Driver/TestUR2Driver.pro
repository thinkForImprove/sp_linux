#-------------------------------------------------
#
# Project created by QtCreator 2016-03-01T13:44:27
#
#-------------------------------------------------
#输出主路径
#DESTDIR_PARENT=/usr/bin/ndt/lfs
#GLOGALLIB_PATH=/usr/bin/ndt/lfs

win32 {
    DESTDIR_PARENT=C:/CFES/BIN
    GLOGALLIB_PATH=C:/CFES/BIN
}

unix {
    DESTDIR_PARENT=/usr/local/CFES/BIN
    GLOGALLIB_PATH=/usr/local/CFES/BIN
}



#公共头文件路径
INCLUDEPATH += ../../../CommonFile/Include\
               ../../../CommonFile/Common

unix {
    target.path = /usr/lib
    INSTALLS += target
    QMAKE_LFLAGS += -Wl,-rpath,./
}

CONFIG(debug, debug|release){
    DESTDIR = $${DESTDIR_PARENT}
    LIBS += -L$${DESTDIR_PARENT}
}else {
    DESTDIR = $${DESTDIR_PARENT}
    LIBS += -L$${DESTDIR_PARENT}
}

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TestUR2Driver
TEMPLATE = app
#版本号
VERSION += 1.0.0

SOURCES += main.cpp\
    TestUR2Driver.cpp \
    ReadDriverConfig.cpp

HEADERS  += \
    DeviceSOLoader.h \
    TestUR2Driver.h \
    ReadDriverConfig.h\
    ../../../Include/CommonFile/IURDevice.h\
    ../../../Include/CommonFile/TypeDef.h\

FORMS    += \
    ur2drivertest.ui

#unix|win32: LIBS += -ldl-2.19
#LIBS += -lXDTSPUtil\

RC_FILE = app.rc

DISTFILES += \
    app.rc

#DESTDIR += C:\NDT\XFS\BIN

#unix {
#    target.path = /usr/lib
#    INSTALLS += target
#}

RESOURCES += \
    resource.qrc

OTHER_FILES += \
    app.rc
