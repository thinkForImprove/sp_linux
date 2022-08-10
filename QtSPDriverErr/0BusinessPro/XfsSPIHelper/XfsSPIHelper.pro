#-------------------------------------------------
#
# Project created by QtCreator 2019-06-06T15:18:39
#
#-------------------------------------------------
#包含公共配置文件
include(../GlobalPathLib.pri)
#-------------------------------------------------

QT       -= gui
QT       += network

TARGET = XfsSPIHelper
TEMPLATE = lib
CONFIG += staticlib

#复制文件
win32 {
!build_pass:system("xcopy .\XfsSPIHelper.h ..\..\..\1QtSPDriver\2DriverAndSP\CommonFile\include\ /Y")
!build_pass:system("xcopy .\ExtraInforHelper.h ..\..\1QtSPDriver\..\2DriverAndSP\CommonFile\include\ /Y")
!build_pass:system("xcopy .\XfsRegValue.h ..\..\..\1QtSPDriver\2DriverAndSP\CommonFile\include\ /Y")
}
unix {
#!build_pass:system("cp -f ./XfsSPIHelper.h ../../../1QtSPDriver/2DriverAndSP/CommonFile/include/")
#!build_pass:system("cp -f ./XfsRegValue.h ../../../1QtSPDriver/2DriverAndSP/CommonFile/include/")
#!build_pass:system("cp -f ./ExtraInforHelper.h ../../../1QtSPDriver/2DriverAndSP/CommonFile/include/")
#建立头文件链接
QMAKE_POST_LINK += ln -sf $$PWD/XfsSPIHelper.h $$PWD/../../3CommonFile/include/XfsSPIHelper.h;
QMAKE_POST_LINK += ln -sf $$PWD/XfsRegValue.h $$PWD/../../3CommonFile/include/XfsRegValue.h;
QMAKE_POST_LINK += ln -sf $$PWD/ExtraInforHelper.h $$PWD/../../3CommonFile/include/ExtraInforHelper.h;
}


#配置输出文件名和目录
CONFIG(debug, debug|release){
    TARGET = XfsSPIHelperd
}else:CONFIG(release, debug|release){
    TARGET = XfsSPIHelper
}

#配置依赖库目录
CONFIG(debug, debug|release){
    LIBS += -lShareHeaderFiled
}else:CONFIG(release, debug|release){
    LIBS += -lShareHeaderFile
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
        IRegOperator.h \
        RegOperator.h \
        QtDLLLoader.h \
        QtAppRunning.h \
        QtShareMemoryRW.h



unix {
    target.path = /usr/lib
    INSTALLS += target
}
