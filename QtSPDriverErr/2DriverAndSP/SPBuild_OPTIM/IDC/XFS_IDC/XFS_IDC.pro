#-------------------------------------------------
#
# Project created by QtCreator 2019-06-25T13:20:38
#
#-------------------------------------------------
#包含公共配置文件
include(../../GlobalPath.pri)
include(../../GlobalPathFormIDC.pri)
#-------------------------------------------------
#使用优化版本
DEFINES += SPBuild_OPTIM
#-------------------------------------------------

QT -= gui

#隐藏控制台窗口: 去掉console配置
#CONFIG += console
CONFIG += c++11
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \

HEADERS += \
        def.h

# 读卡器(IDC)相关
SOURCES += \
        XFS_IDC.cpp \
        XFS_IDC_DEC.cpp \
        XFS_IDC_FIRE.cpp \
        XFS_SIU.cpp

HEADERS += \
        XFS_IDC.h \
        IDCXfsHelper.h \
        IDevIDC.h \
        ISPBaseIDC.h

# 退卡模块(CRM)相关
SOURCES += \
        XFS_CRM.cpp \
        XFS_CRM_DEC.cpp

HEADERS += \
        IDevCRM.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


