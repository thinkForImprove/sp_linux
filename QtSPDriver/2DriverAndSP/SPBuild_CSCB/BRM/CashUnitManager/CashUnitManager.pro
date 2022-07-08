include(../../GlobalPath.pri)

QT       += core
TARGET = CashUnitManager
TEMPLATE = lib

DEFINES += CASHUNITMANAGER_LIBRARY

#INCLUDEPATH += ../../CommonFile/Common\


LIBS += -lSMConfig \


SOURCES += CashUnitConfig.cpp \
    CashUnitManager.cpp \
    CashUnit.cpp \
    CUInterface.cpp \
    ISOCurrencySet.cpp\
    MultiString.cpp\

HEADERS +=  CashUnitManager.h\
    AutoRollback.h \
    CashUnit.h \
    CashUnitConfig.h \
    Configurable.h \
    CUInterface.h \
    CUMngrFunc.h \
    ICashUnitConfig.h \
    ISOCurrencySet.h\
    CashUnitDefine.h\
    ICashUnitManager.h\
    ISMConfig.h\
    ILogWrite.h\



