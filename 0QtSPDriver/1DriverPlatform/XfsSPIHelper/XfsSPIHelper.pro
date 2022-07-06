#-------------------------------------------------
#
# Project created by QtCreator 2019-06-06T15:18:39
#
#-------------------------------------------------
#包含公共配置文件
include(../GlobalSysType.pri)
#-------------------------------------------------

QT       -= gui
QT       += network

TARGET = XfsSPIHelper
TEMPLATE = lib
CONFIG += staticlib

#复制文件
win32 {
!build_pass:system("xcopy .\XfsSPIHelper.h ..\..\2DriverAndSP\CommonFile\include\ /Y")
!build_pass:system("xcopy .\ExtraInforHelper.h ..\..\2DriverAndSP\CommonFile\include\ /Y")
!build_pass:system("xcopy .\XfsRegValue.h ..\..\2DriverAndSP\CommonFile\include\ /Y")
}
unix {
!build_pass:system("cp -f ./XfsSPIHelper.h ../../2DriverAndSP/CommonFile/include/")
!build_pass:system("cp -f ./XfsRegValue.h ../../2DriverAndSP/CommonFile/include/")
!build_pass:system("cp -f ./ExtraInforHelper.h ../../2DriverAndSP/CommonFile/include/")
}


#配置输出文件名和目录
CONFIG(debug, debug|release){
    TARGET = XfsSPIHelperd
    DESTDIR += $$PWD/../../2DriverAndSP/CommonFile/lib/Debug
}else:CONFIG(release, debug|release){
    TARGET = XfsSPIHelper
    DESTDIR += $$PWD/../../2DriverAndSP/CommonFile/lib/Release
}

#配置包含目录
INCLUDEPATH += $$PWD/../../2DriverAndSP/CommonFile/include
INCLUDEPATH += $$PWD/../../2DriverAndSP/CommonFile/XFS/INCLUDE

#配置依赖库目录
CONFIG(debug, debug|release){
    LIBS += -L$$PWD/../../2DriverAndSP/CommonFile/lib/Debug -lShareHeaderFiled
}else:CONFIG(release, debug|release){
   LIBS += -L$$PWD/../../2DriverAndSP/CommonFile/lib/Release -lShareHeaderFile
}


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
        XfsSPIHelper.cpp \
        XfsRegValue.cpp \
        ExtraInforHelper.cpp

HEADERS += \
        XfsSPIHelper.h \
        XfsRegValue.h \
        ExtraInforHelper.h \
        ../../2DriverAndSP/CommonFile/include/IRegOperator.h \
        ../../2DriverAndSP/CommonFile/include/RegOperator.h \
        ../../2DriverAndSP/CommonFile/include/QtDLLLoader.h \
        ../../2DriverAndSP/CommonFile/include/QtAppRunning.h \
        ../../2DriverAndSP/CommonFile/include/QtShareMemoryRW.h



unix {
    target.path = /usr/lib
    INSTALLS += target
}

CONFIG(debug, debug|release){
    QMAKE_PRE_LINK  += mkdir -p ../../../1QtSPDriver/2DriverAndSP/CommonFile/lib/Debug/
    QMAKE_POST_LINK += cp ../../2DriverAndSP/CommonFile/lib/Debug/* \
                       ../../../1QtSPDriver/2DriverAndSP/CommonFile/lib/Debug/
} else {
    QMAKE_PRE_LINK  += mkdir -p ../../../1QtSPDriver/2DriverAndSP/CommonFile/lib/Release/
    QMAKE_POST_LINK += cp ../../2DriverAndSP/CommonFile/lib/Release/* \
                       ../../../1QtSPDriver/2DriverAndSP/CommonFile/lib/Release/
}
