#-------------------------------------------------
#
# Project created by QtCreator 2019-07-05T16:07:00
#
#-------------------------------------------------
#包含公共配置文件
include(../../GlobalPathLib.pri)
#-------------------------------------------------

QT -= gui

TEMPLATE = lib
CONFIG += staticlib
DEFINES += CJSON_OBJECT_LIBRARY

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

#配置输出文件名和目录
CONFIG(debug, debug|release){
    TARGET = cjson_objectd
}else:CONFIG(release, debug|release){
    TARGET = cjson_object
}

#配置包含目录
#INCLUDEPATH += $$PWD/../../../3CommonFile/include/utilities_inc

SOURCES += \
    cJSON.c \
    cjson_object.cpp

HEADERS += \
    cJSON.h \
    cjson_object_global.h \
    cjson_object.h \
    framework.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
    INSTALLS += target
}

# Creating the linkers of output header file
#QMAKE_POST_LINK += ln -sf $$PWD/../../utilities/$(QMAKE_TARGET)/$(QMAKE_TARGET)_global.h $$PWD/../../CommonFile/include/
#QMAKE_POST_LINK += ln -sf $$PWD/../../utilities/$(QMAKE_TARGET)/$(QMAKE_TARGET).h $$PWD/../../CommonFile/include/

#构建时执行一次
#unix {
#!build_pass:system("cp -f cjson_object_global.h ../../../3CommonFile/CommonFile/include/")
#!build_pass:system("cp -f cjson_object.h ../../../3CommonFile/CommonFile/include/")
#}

#建立头文件链接
QMAKE_POST_LINK += ln -sf $$PWD/cjson_object_global.h $$PWD/../../../3CommonFile/include/cjson_object_global.h;
QMAKE_POST_LINK += ln -sf $$PWD/cjson_object.h $$PWD/../../../3CommonFile/include/cjson_object.h;



