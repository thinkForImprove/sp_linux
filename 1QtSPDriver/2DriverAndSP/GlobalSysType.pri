#区分平台的宏定义
win32 {
    DEFINES += QT_WIN32
}
unix {
    DEFINES += QT_LINUX
}
#区分不同版本的LFS
#其中QT_WIN_LINUX_XFS为杭州银行版本，QT_LINUX_LFS为邮储银行版本
win32 {
    DEFINES += QT_WIN_LINUX_XFS
}
unix {
    #DEFINES += QT_WIN_LINUX_XFS
    DEFINES += QT_LINUX_LFS
    #平安银行广电Manager版本控制
#    DEFINES += QT_LINUX_MANAGER_PISA

    DEFINES += QT_LINUX_MANAGER_ZJ
}
