include(../../GlobalPath.pri)


LIBS += -L/usr/local/CFES/lib/opencv

LIBS += -lopencv_core\
        -lopencv_imgcodecs\
        -lopencv_imgproc


QT       -= gui

TARGET = SNImageParser
TEMPLATE = lib
#版本号
VERSION += 1.0.0

DEFINES += SNIMAGEPARSER_LIBRARY

SOURCES += SNImageParser.cpp
HEADERS +=../Include/ISNImageParser.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}





