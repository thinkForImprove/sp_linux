#-------------------------------------------------
#包含公共配置文件
include(../GlobalSysType.pri)
#-------------------------------------------------

#减少编译器输出的警告信息
#CONFIG += warn_off

#配置支持C++11
CONFIG += c++11

#去掉链接文件，只保留库文件
CONFIG += plugin

#配置输出目录
win32 {
    DESTDIR += C:/CFES/BIN
    QMAKE_LFLAGS += -Wl,-rpath,C:/CFES/BIN
}
unix {
#    DESTDIR += /usr/local/CFES/BIN
    DESTDIR += $$PWD/../../../BIN
    QMAKE_LFLAGS += -Wl,-rpath,/usr/local/CFES/BIN
}

#配置包含目录
INCLUDEPATH += $$PWD/../CommonFile/include
INCLUDEPATH += $$PWD/include

#配置依赖库目录
CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/../CommonFile/lib/Debug -lShareHeaderFiled
}else:CONFIG(release, debug|release) {
    LIBS += -L$$PWD/../CommonFile/lib/Release -lShareHeaderFile
    #release版本去掉断言
    DEFINES += NDEBUG
}
