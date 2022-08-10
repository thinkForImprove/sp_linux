#-------------------------------------------------
#
# Project created by QtCreator 2019-06-28T09:40:51
#
#-------------------------------------------------
#包含公共配置文件
include(../GlobalPathAll.pri)
#-------------------------------------------------

#配置输出目录
win32 {
    DESTDIR += C:/CFES/BIN
    QMAKE_LFLAGS += -Wl,-rpath,C:/CFES/BIN
    LIBS += -LC:/CFES/BIN
}
unix {
    DESTDIR += $$PWD/../../BIN
    QMAKE_LFLAGS += -Wl,-rpath,/usr/local/CFES/BIN
    LIBS += -L$$PWD/../../BIN
}

#配置依赖库目录
CONFIG(debug, debug|release) {
    LIBS += -lXfsSPIHelperd -lShareHeaderFiled
}else:CONFIG(release, debug|release) {
    LIBS += -lXfsSPIHelper -lShareHeaderFile
}

