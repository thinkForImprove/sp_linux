
#增加网络配置
#QT += network

#减少编译器输出的警告信息
#CONFIG += warn_off

#配置支持C++11
CONFIG += c++11

#去掉链接文件，只保留库文件
CONFIG += plugin

#区分平台的宏定义
win32 {
    DEFINES += QT_WIN32
}
unix {
    DEFINES += QT_LINUX
}

#配置输出目录
win32 {
    DESTDIR += C:/CFES/BIN
    QMAKE_LFLAGS += -Wl,-rpath,C:/CFES/BIN
}
unix {
#    DESTDIR += /usr/local/CFES/BIN
    DESTDIR += $$PWD/../../BIN
    QMAKE_LFLAGS += -Wl,-rpath,/usr/local/CFES/BIN
}

#配置包含目录
INCLUDEPATH += $$PWD/CommonFile/include

#配置依赖库目录
Debug:LIBS += -L"CommonFile/lib/Debug"
Release:LIBS += -L"CommonFile/lib/Release"


