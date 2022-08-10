#-------------------------------------------------
#包含公共配置文件
include(./GlobalSysTypeAll.pri)
#-------------------------------------------------
#增加网络配置
#QT += network

#减少编译器输出的警告信息
#CONFIG += warn_off

# 忽略警告: -Wno-unused-parameter(设置了但未使用的参数)
QMAKE_CXXFLAGS +=  -Wno-unused-parameter

#配置支持C++11
CONFIG += c++11

#去掉链接文件，只保留库文件
CONFIG += plugin

##版本信息
win32{
    RC_FILE+=version.rc
    LIBS+=-lVersion
}

#配置包含目录
INCLUDEPATH += $$PWD/3CommonFile/include/
INCLUDEPATH += $$PWD/3CommonFile/include/utilities_inc
INCLUDEPATH += $$PWD/3CommonFile/XFS/INCLUDE
INCLUDEPATH += $$PWD/3CommonFile/CPP

#配置依赖库目录
CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/3CommonFile/lib/Debug
    LIBS += -L$$PWD/3CommonFile/lib/other
    LIBS += -L$$PWD/3CommonFile/lib/other/OpenCV_3.2
} else:CONFIG(release, debug|release) {
    LIBS += -L$$PWD/3CommonFile/lib/Release
    LIBS += -L$$PWD/3CommonFile/lib/other
    LIBS += -L$$PWD/3CommonFile/lib/other/OpenCV_3.2
    #release版本去掉断言
    DEFINES += NDEBUG
}





