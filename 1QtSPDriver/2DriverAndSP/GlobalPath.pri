#-------------------------------------------------
#包含公共配置文件
include(./GlobalSysType.pri)
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
    LIBS += -LC:/CFES/BIN
    #QMAKE_LFLAGS += /MANIFESTUAC:"level='requireAdministrator'uiAccess='false'"
}
unix {
#    DESTDIR += /usr/local/CFES/BIN
    DESTDIR += $$PWD/../../BIN
    QMAKE_LFLAGS += -Wl,-rpath,/usr/local/CFES/BIN
#    LIBS += -L/usr/local/CFES/BIN
    LIBS += -L$$PWD/../../BIN
}

#配置包含目录
INCLUDEPATH += $$PWD/CommonFile/include
INCLUDEPATH += $$PWD/CommonFile/XFS/INCLUDE
INCLUDEPATH += $$PWD/CommonFile/include/utilities_inc

#配置依赖库目录
CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/CommonFile/lib/Debug -lXfsSPIHelperd
    LIBS += -L$$PWD/CommonFile/lib/Debug -lShareHeaderFiled
}else:CONFIG(release, debug|release) {
    LIBS += -L$$PWD/CommonFile/lib/Release -lXfsSPIHelper
    LIBS += -L$$PWD/CommonFile/lib/Release -lShareHeaderFile
    #release版本去掉断言
    DEFINES += NDEBUG
}
