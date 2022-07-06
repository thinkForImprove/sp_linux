include(../../GlobalPath.pri)

QT -= gui

#隐藏控制台窗口: 去掉console配置
#CONFIG += console
CONFIG += c++11
CONFIG -= app_bundle

SOURCES += main.cpp\
    AutoFireDeviceStatus.cpp \
    AutoSaveDataAndFireEvent.cpp \
    AutoSaveDataToFile.cpp \
    AutoUpdateCassData.cpp \
    AutoUpdateCassStatusAndFireCassStatus.cpp \
    BRMCONFIGPARAM.cpp \
    ExtraInforManager.cpp \
    SPConfigFile.cpp \
    SPUtil.cpp \
    StatusConvertor.cpp \
    XFSErrorConvertor.cpp\
    XFS_CRS.cpp\
    CDMResult.cpp\
    CIMResult.cpp \
    brmcashacceptor.cpp

HEADERS  += \
    AutoDeleteArray.h \
    AutoFireDeviceStatus.h \
    AutoSaveDataAndFireEvent.h \
    AutoSaveDataToFile.h \
    AutoUpdateCassData.h \
    AutoUpdateCassStatusAndFireCassStatus.h \
    BRMCASHININFOR.h \
    BRMCASHUNITINFOR.h \
    BRMCONFIGPARAM.h \
    BRMPRESENTINFOR.h \
    ExtraInforManager.h \
    XFS_CRS.h \
    SPConfigFile.h \
    SPUtil.h \
    StatusConvertor.h \
    XFSErrorConvertor.h\
    CDMResult.h\
    CIMResult.h\
    ../../CommonFile/include/IBRMAdapter.h\
    ../../CommonFile/include/ILogWrite.h \
    brmcashacceptor.h \
    brmcashacceptor.h

LIBS += -lSMConfig \
-lMixManager \
-lCashUnitManager
