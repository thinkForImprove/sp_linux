include(../../GlobalPath.pri)

TARGET = XFS_SIG

QT += core gui widgets
#隐藏控制台窗口: 去掉console配置
#CONFIG += console
CONFIG += c++11
CONFIG -= app_bundle

LIBS += -ldl \
        -lrt

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += PINGAN_VERSION

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    XFS_SIG.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS  += \
    ../../CommonFile/include/IDevSIG.h \
    XFS_SIG.h \
    mainwindow.h \
    CommonInfo.h

FORMS += \
    mainwindow.ui
