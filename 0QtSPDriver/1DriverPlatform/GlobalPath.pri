#-------------------------------------------------
#包含公共配置文件
include(./GlobalSysType.pri)
#-------------------------------------------------
#增加网络配置
#QT += network

#减少编译器输出的警告信息
#CONFIG += warn_off
QMAKE_CXXFLAGS +=  -Wno-unused-parameter

#配置支持C++11
CONFIG += c++11

#去掉链接文件，只保留库文件
CONFIG += plugin

#配置输出目录
win32 {
    DESTDIR += C:/CFES/BIN
    QMAKE_LFLAGS += -Wl,-rpath,C:/CFES/BIN
    LIBS += -LC:/CFES/BIN
}
unix {
#    DESTDIR += /usr/local/CFES/BIN
    DESTDIR += $$PWD/../../BIN
    QMAKE_LFLAGS += -Wl,-rpath,/usr/local/CFES/BIN
#    LIBS += -L/usr/local/CFES/BIN
    LIBS += -L$$PWD/../../BIN
}

#配置包含目录
INCLUDEPATH += $$PWD/../2DriverAndSP/CommonFile/include
INCLUDEPATH += $$PWD/../2DriverAndSP/CommonFile/XFS/INCLUDE

#配置依赖库目录
CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/../2DriverAndSP/CommonFile/lib/Debug -lXfsSPIHelperd
    LIBS += -L$$PWD/../2DriverAndSP/CommonFile/lib/Debug -lShareHeaderFiled
}else:CONFIG(release, debug|release) {
    LIBS += -L$$PWD/../2DriverAndSP/CommonFile/lib/Release -lXfsSPIHelper
    LIBS += -L$$PWD/../2DriverAndSP/CommonFile/lib/Release -lShareHeaderFile
    #release版本去掉断言
    DEFINES += NDEBUG
}

##版本信息
win32{
    RC_FILE+=version.rc
    LIBS+=-lVersion
}

