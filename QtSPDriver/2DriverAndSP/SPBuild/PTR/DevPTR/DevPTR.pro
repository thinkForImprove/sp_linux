#-------------------------------------------------
#
# Project created by QtCreator 2021-04-25T14:55:28
#
#-------------------------------------------------
#包含公共配置文件
include(../../GlobalPath.pri)
#-------------------------------------------------

QT       += widgets dbus core network gui

TARGET = DevPTR
TEMPLATE = lib

DEFINES += IDEVPTR_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    DevRPR.cpp \
    SNBC-BKC310/DevRPR_SNBC.cpp \
    SNBC-BKC310/DevImpl_BKC310.cpp

HEADERS += \
    IDevPTR.h \
    DevPortLogFile.h \
    ../XFS_PTR/def.h \
    DevRPR.h \    
    SNBC-BKC310/DevRPR_SNBC.h \
    SNBC-BKC310/POSSDK.h \
    SNBC-BKC310/BKC310Def.h \
    SNBC-BKC310/DevImpl_BKC310.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}



#### 第三方库:Freetype,打印字模转换相关 ####
#LIBS += -L../../CommonFile/lib/freetype -lfreetype
#INCLUDEPATH += ../../CommonFile/include/freetype2
#INCLUDEPATH += ../../CommonFile/include/freetype2/freetype
#INCLUDEPATH += $$PWD/../../../../usr/local/include
#DEPENDPATH += $$PWD/../../../../usr/local/include

#unix:!macx: PRE_TARGETDEPS += $$PWD/../../../../usr/local/lib/libfreetype.a

#SUBDIRS += \
#    DevPTR_RPR.pro \
#    DevPTR_RPR.pro
