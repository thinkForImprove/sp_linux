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
}

#--------------------------------------------------------------------------
# Manager区分
# SET_MANAGER_CFES: 自有Manager
# SET_MANAGER_PISA_CC: 长城PIAS
# SET_MANAGER_PISA: PIAS
# SET_MANAGER_GD: 广电Manager
#DEFINES +=

#--------------------------------------------------------------------------
# 银行区分，通过全局宏定义加入DEFINES指定[值必须为以下定义且必须唯一]，不指定则缺省为通用
# 银行定义格式为: SET_BANK_XXXX, 定义如下:
#   SET_BANK_ALL: 通用
#   SET_BANK_CSCB: 长沙银行; SET_BANK_CMBC: 民生银行;
#   SET_BANK_SXXH: 陕西信合; SET_BANK_JIANGNAN: 江南银行
#   SET_BANK_ICBC: 工商银行;
# 指定方式: DEFINES += SET_BANK_XXXX(不指定缺省为通用)
#DEFINES += SET_BANK_ALL
#DEFINES += SET_BANK_CMBC
#DEFINES += SET_BANK_CSCB
#DEFINES += SET_BANK_SXXH
#DEFINES += SET_BANK_JIANGNAN
#DEFINES += SET_BANK_ICBC
#DEFINES += SET_BANK_CCB
#DEFINES += SET_BANK_JINDIAN
#--------------------------------------------------------------------------
# 指定 SERVICE_CLASS 序号, 通过全局宏定义加入DEFINES指定(值唯一), 不指定为缺省缺省
# 分类定义处理在 3CommonFile/XFS/INCLUDE/XFS_COMMON.H


#--------------------------------------------------------------------------
# 指定 SERVICE_CLASS_VERSION, 通过全局宏定义加入DEFINES指定[值必须为以下定义且必须唯一]，
# 不指定则缺省为1.00
# 分类定义处理在 3CommonFile/XFS/INCLUDE/XFS_COMMON.H
# 定义格式为: SERVICE_CLASS_VERSION_XXXX, 定义如下:
#   SET_SERVICE_CLASS_VER_300: Version 3.00
#   SET_SERVICE_CLASS_VER_303: Version 3.03
#   SET_SERVICE_CLASS_VER_310: Version 3.10
#   SET_SERVICE_CLASS_VER_320: Version 3.20
#   SET_SERVICE_CLASS_VER_100: Version 1.00
# 指定方式: DEFINES += SERVICE_CLASS_VERSION_XXXX
DEFINES += SET_SERVICE_CLASS_VER_100

#--------------------------------------------------------------------------
# SP事件通知机制
# 分为以下下三种：1.callback回掉函数 2.dbus通信 2.Manger提供接口WFMPostMessage
# DEFINES += EVENT_NOTIFY_CALLBACK
# DEFINES += EVENT_NOTIFY_DBUS
# DEFINES += EVENT_NOTIFY_POSTMESSAGE
#DEFINES += EVENT_NOTIFY_POSTMESSAGE
#DEFINES += EVENT_NOTIFY_POSTMESSAGE_XFS       #30-00-00-00(FT#0074)
#DEFINES += EVENT_NOTIFY_DBUS       #30-00-00-00(FT#0074)

#--------------------------------------------------------------------------
# Manager配置文件路径
# 分为以下下两种：1./etc/xfs 2./etc/LFS
# DEFINES += MGR_CFG_PATH_XFS
# DEFINES += MGR_CFG_PATH_LFS
#DEFINES += MGR_CFG_PATH_LFS
