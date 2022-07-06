#-------------------------------------------------
#
# Project created by QtCreator 2019-07-05T16:07:00
#
#-------------------------------------------------
#包含公共配置文件
include(../../GlobalPath.pri)
#-------------------------------------------------

QT -= gui

TEMPLATE = lib
CONFIG += staticlib
DEFINES += DATA_CONVERTOR_LIBRARY

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#配置包含目录
INCLUDEPATH += $$PWD/../include

SOURCES += \
    data_convertor.cpp

HEADERS += \
    data_convertor_global.h \
    data_convertor.h \
    framework.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
    INSTALLS += target
}

unix {
!build_pass:system("cp -f data_convertor_global.h ../../CommonFile/include/")
!build_pass:system("cp -f data_convertor.h ../../CommonFile/include/")
}

#自动拷贝静态库到目标目录
CONFIG(debug, debug|release){
    QMAKE_PRE_LINK  += mkdir -p $$PWD/../../CommonFile/lib/Debug/
    QMAKE_POST_LINK += cp $$PWD/../../../../BIN/lib$(QMAKE_TARGET).a \
                          $$PWD/../../CommonFile/lib/Debug/
} else {
    QMAKE_PRE_LINK  += mkdir -p $$PWD/../../CommonFile/lib/release/
    QMAKE_POST_LINK += cp ../../../../BIN/lib$(QMAKE_TARGET).a \
                          ../../CommonFile/lib/release/lib$(QMAKE_TARGET).a
}


