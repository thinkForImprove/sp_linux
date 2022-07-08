#-------------------------------------------------
#
# Project created by QtCreator 2022-04-14T18:31:56
#
#-------------------------------------------------
#包含公共配置文件
include(../GlobalPathLib.pri)

QT       -= gui

TARGET = IomcComm
TEMPLATE = lib
CONFIG += staticlib

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
    USBDrive.cpp

HEADERS += \
    IOMCUsbdef.h \
    UsbDLLDef.h \
    USBDrive.h \
    IOMCDef.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}

#复制文件
win32 {
#!build_pass:system("xcopy .\INIFileReader.h ..\..\..\1QtSPDriver\2DriverAndSP\CommonFile\include\ /Y")
#!build_pass:system("xcopy .\LogWriteThread.h ..\..\..\1QtSPDriver\2DriverAndSP\CommonFile\include\ /Y")
#!build_pass:system("xcopy .\MultiString.h ..\..\..\1QtSPDriver\2DriverAndSP\CommonFile\include\ /Y")
#!build_pass:system("xcopy .\QtShareMemoryRW.h ..\..\..\1QtSPDriver\2DriverAndSP\CommonFile\include\ /Y")
#!build_pass:system("xcopy .\QtAppRunning.h ..\..\..\1QtSPDriver\2DriverAndSP\CommonFile\include\ /Y")
#!build_pass:system("xcopy .\SimpleMutex.h ..\..\..\1QtSPDriver\2DriverAndSP\CommonFile\include\ /Y")
#!build_pass:system("xcopy .\StlSimpleThread.h ..\..\..\1QtSPDriver\2DriverAndSP\CommonFile\include\ /Y")
}
unix {
#建立头文件链接
QMAKE_POST_LINK += ln -sf $$PWD/IOMCDef.h $$PWD/../../3CommonFile/include/IOMCDef.h;
QMAKE_POST_LINK += ln -sf $$PWD/IOMCUsbdef.h  $$PWD/../../3CommonFile/include/IOMCUsbdef.h;
QMAKE_POST_LINK += ln -sf $$PWD/UsbDLLDef.h   $$PWD/../../3CommonFile/include/UsbDLLDef.h;
QMAKE_POST_LINK += ln -sf $$PWD/USBDrive.h    $$PWD/../../3CommonFile/include/USBDrive.h;
}

#配置输出文件名和目录
CONFIG(debug, debug|release){
    TARGET = IomcCommd
}else:CONFIG(release, debug|release){
    TARGET = IomcComm
}
