include(../../../GlobalPath.pri)
QT       -= gui

TARGET = GenerateCustomSNInfo
TEMPLATE = lib

DEFINES += GENERATECUSTOMSNINFO_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS += -lSMConfig \   #30-00-00-00(FS#0019)

SOURCES += GenerateCustomSNInfo.cpp\
    ../../SMConfig/tinystr.cpp \
    ../../SMConfig/tinyxml.cpp \
    ../../SMConfig/tinyxmlerror.cpp \
    ../../SMConfig/tinyxmlparser.cpp

HEADERS += GenerateCustomSNInfo.h\
        ../../../CommonFile/include/ISaveBRMCustomSNInfo.h\
        ../../../CommonFile/include/FSNDefine.h\
        ../../../CommonFile/include/QtTypeDef.h\
        ../../SMConfig/tinystr.h \
        ../../SMConfig/tinyxml.h\

#LIBS += -lopencv_core -lopencv_imgcodecs -lopencv_imgproc
