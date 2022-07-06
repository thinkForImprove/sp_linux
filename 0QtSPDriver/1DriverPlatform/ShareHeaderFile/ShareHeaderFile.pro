#-------------------------------------------------
#
# Project created by QtCreator 2019-06-04T17:32:31
#
#-------------------------------------------------
#包含公共配置文件
include(../GlobalSysType.pri)
#-------------------------------------------------
QT       -= gui
QT       += network

#TARGET = ShareHeaderFile
TEMPLATE = lib
CONFIG += staticlib

#复制文件
win32 {
!build_pass:system("xcopy .\INIFileReader.h ..\..\2DriverAndSP\CommonFile\include\ /Y")
!build_pass:system("xcopy .\LogWriteThread.h ..\..\2DriverAndSP\CommonFile\include\ /Y")
!build_pass:system("xcopy .\MultiString.h ..\..\2DriverAndSP\CommonFile\include\ /Y")
!build_pass:system("xcopy .\QtShareMemoryRW.h ..\..\2DriverAndSP\CommonFile\include\ /Y")
!build_pass:system("xcopy .\QtAppRunning.h ..\..\2DriverAndSP\CommonFile\include\ /Y")
!build_pass:system("xcopy .\SimpleMutex.h ..\..\2DriverAndSP\CommonFile\include\ /Y")
!build_pass:system("xcopy .\StlSimpleThread.h ..\..\2DriverAndSP\CommonFile\include\ /Y")
}
unix {
!build_pass:system("cp -f  ./INIFileReader.h ../../2DriverAndSP/CommonFile/include/")
!build_pass:system("cp -f  ./LogWriteThread.h ../../2DriverAndSP/CommonFile/include/")
!build_pass:system("cp -f  ./MultiString.h ../../2DriverAndSP/CommonFile/include/")
!build_pass:system("cp -f  ./QtShareMemoryRW.h ../../2DriverAndSP/CommonFile/include/")
!build_pass:system("cp -f  ./QtAppRunning.h ../../2DriverAndSP/CommonFile/include/")
!build_pass:system("cp -f  ./SimpleMutex.h ../../2DriverAndSP/CommonFile/include/")
!build_pass:system("cp -f  ./StlSimpleThread.h ../../2DriverAndSP/CommonFile/include/")
}

#配置输出文件名和目录
CONFIG(debug, debug|release){
    TARGET = ShareHeaderFiled
    DESTDIR += $$PWD/../../2DriverAndSP/CommonFile/lib/Debug
}else:CONFIG(release, debug|release){
    TARGET = ShareHeaderFile
    DESTDIR += $$PWD/../../2DriverAndSP/CommonFile/lib/Release
}

#配置包含目录
INCLUDEPATH += $$PWD/../../2DriverAndSP/CommonFile/include
INCLUDEPATH += $$PWD/../../2DriverAndSP/CommonFile/XFS/include

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
        INIFileReader.cpp\
        MultiString.cpp\
        QtShareMemoryRW.cpp \
        StlSimpleThread.cpp \
        SimpleMutex.cpp\
        LogWriteThread.cpp\
        QtAppRunning.cpp


HEADERS += \
        INIFileReader.h\
        MultiString.h\
        QtShareMemoryRW.h \
        StlSimpleThread.h \
        SimpleMutex.h\
        LogWriteThread.h\
        QtAppRunning.h


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
