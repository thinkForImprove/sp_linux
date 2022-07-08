include(../../../GlobalPath.pri)
QT       -= gui
TARGET = AdapterUR2
TEMPLATE = lib

CONFIG += c++11
CONFIG += warn_off

DEFINES += IBRMADAPTER_LIBRARY
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        AdapterUR2Export.cpp\
        AdapterUR2.cpp\
        ErrCodeTranslate.cpp\
        GenerateSN.cpp

HEADERS += \
        AdapterUR2.h\
        AdapterUR2_global.h\
        ErrCodeTranslate.h\
        GenerateSN.h\
        ../../../CommonFile/include/IBRMAdapter.h\
        ../../../CommonFile/include/IURDevice.h\
        ../../../CommonFile/include/ILogWrite.h\
        ../../../CommonFile/include/CashUnitDefine.h\
        ../../../CommonFile/include/ISaveBRMCustomSNInfo.h\
        ../../../CommonFile/include/FSNDefine.h\
        ../../../CommonFile/include/QtTypeDef.h\
        ../../../CommonFile/include/ISNImageParser.h



LIBS += -lSNImageParser\
        -lGenerateCustomSNInfo\
        -lDevUR2\
