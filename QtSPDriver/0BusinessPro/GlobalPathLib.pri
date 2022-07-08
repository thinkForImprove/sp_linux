#-------------------------------------------------
#
# Project created by QtCreator 2019-07-05T16:07:00
#
#-------------------------------------------------
#包含公共配置文件
include(../GlobalPathAll.pri)
#-------------------------------------------------

#配置包含目录
#INCLUDEPATH += $$PWD/../3CommonFile/include/utilities_inc
#INCLUDEPATH += $$PWD/../3CommonFile/include

#配置输出目录
win32 {
    CONFIG(debug, debug|release) {
        DESTDIR += C:/CFES/lib/Debug
        QMAKE_LFLAGS += -Wl,-rpath,C:/CFES/lib/Debug
        LIBS += -LC:/CFES/lib/Debug
    } else {
        DESTDIR += C:/CFES/lib/Release
        QMAKE_LFLAGS += -Wl,-rpath,C:/CFES/lib/Release
        LIBS += -LC:/CFES/lib/Release
    }
}
unix {
    CONFIG(debug, debug|release) {
        DESTDIR += $$PWD/../3CommonFile/lib/Debug
        QMAKE_LFLAGS += -Wl,-rpath,/usr/local/CFES/BIN
        LIBS += -L$$PWD/../3CommonFile/lib/Debug
    } else {
        DESTDIR += $$PWD/../3CommonFile/lib/Release
        QMAKE_LFLAGS += -Wl,-rpath,/usr/local/CFES/BIN
        LIBS += -L$$PWD/../3CommonFile/lib/Release
    }
}


#创建目标目录
QMAKE_PRE_LINK  += mkdir -p $(DESTDIR)

