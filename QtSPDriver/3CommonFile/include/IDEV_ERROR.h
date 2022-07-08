#pragma once
/////////////////////////////////////////////////////////////////////////////////////
#define DEV_ERROR_TYPE_HARDWARE 0  //驱动硬件错误的标识
#define DEV_ERROR_TYPE_SOFTWARE 1  //驱动软件错误类型的标识
/////////////////////////////////////////////////////////////////////////////////////
#define DEV_SUCCESS 0  //成功
/////////////////////////////////////////////////////////////////////////////////////
//帮助构造软件错误的错误码
#define SOFTERRORCODE(code) ((DEV_ERROR_TYPE_SOFTWARE << 20) | ((code)&0xFFFF))
//帮助构造硬件错误的错误码
#define HARDWAREERRORCODE(code) ((DEV_ERROR_TYPE_HARDWARE << 20) | ((code)&0xFFFF))
//公共错误
#define DEF_PUB_SOFT_ERR(CODE) (SOFTERRORCODE(0x1000 | ((CODE)&0x0FFF)))
//设备类公共错误
#define DEF_DEV_SOFT_ERROR(CODE) (SOFTERRORCODE(0x2000 | ((CODE)&0x0FFF)))
//自定义错误
#define DEF_SOFT(CODE) (SOFTERRORCODE(((CODE)&0x0FFF)))
//组装错误码的宏
// devClass 值同XFS中的类型一致
// devCode 设备的唯一标识号
// type   错误类型 参见上面的错误类型 DEV_ERROR_TYPE_XXX
#define MACRO_ERROR_CODE(devClass, devCode, type, code) ((devClass << 24) | ((type << 20) & 0x00F00000) | ((devCode << 16) & 0x000F0000) | (code & 0x0FFFF))

//帮助定义软件错误的宏
#define MACRO_SOFTWARE_ERROR_CODE(devClass, devCode, code) MACRO_ERROR_CODE(devClass, devCode, DEV_ERROR_TYPE_SOFTWARE, code)

//帮助定义配置错误的宏
#define MACRO_CONFIG_ERROR_CODE(devClass, devCode, code) MACRO_ERROR_CODE(devClass, devCode, DEV_ERROR_TYPE_CONFIG, code)

//帮助定义硬件错误的宏
//这里需要注意的是，如果需要从驱动层面返回软件错误或者其他类型错误，请使用上面的SOFTERRORCODE/CONFIGERRORCODE
#define MACRO_HARDWARE_ERROR_CODE(devClass, devCode, code) \
    MACRO_ERROR_CODE(devClass, devCode, ((code & 0x00F00000) ? ((code & 0x00F00000) >> 20) : DEV_ERROR_TYPE_HARDWARE), code)

/////////////////////////////////////////////////////////////////////////////////////
//提取错误类型的宏
#define ERRORTYPE(code) (code & 0x00F00000)

//判断是否是硬件错误的宏
#define ISHARDWAREERROR(code) (HARDWAREERRORCOE(0) == (ERRORTYPE(code)))

//判断是否是软件错误的宏
#define ISSOFTWARERROR(code) (SOFTERRORCODE(0) == ERRORTYPE(code))

//提取原始错误码的宏
#define ERRORCODE(code)     (code & 0x00F0FFFF)
#define REALERRORCODE(code) (code & 0x0FFFF)
//软件层面的错误
//通讯类格式
// 0x1XXX
#define SOFT_ERROR_DEVPORT_NOTOPEN   DEF_PUB_SOFT_ERR(1)   //端口打开失败
#define SOFT_ERROR_DEVPORT_FAIL      DEF_PUB_SOFT_ERR(2)   //通讯错误
#define SOFT_ERROR_DEVPORT_PARAM     DEF_PUB_SOFT_ERR(3)   //通讯参数错误
#define SOFT_ERROR_DEVPORT_CANCELED  DEF_PUB_SOFT_ERR(4)   //操作取消
#define SOFT_ERROR_DEVPORT_LIBRARY   DEF_PUB_SOFT_ERR(5)   //加载通讯库失败
#define SOFT_ERROR_DEVPORT_TIMEOUT   DEF_PUB_SOFT_ERR(6)   //通讯超时
#define SOFT_ERROR_DEVPORT_NODEFINED DEF_PUB_SOFT_ERR(7)   //未知错误
#define SOFT_ERROR_UNSUPP            DEF_PUB_SOFT_ERR(16)  //不支持的命令
#define SOFT_ERROR_NOT_OPEN          DEF_PUB_SOFT_ERR(17)  //设备没有打开
#define SOFT_ERROR_NOT_LOAD          DEF_PUB_SOFT_ERR(18)  //驱动没有加载
#define SOFT_ERROR_ALLOC_MEMEORY     DEF_PUB_SOFT_ERR(19)  //分配内存错误
#define CALL_DRIVER_ERROR            DEF_PUB_SOFT_ERR(20)  //调用驱动接口错误
#define SOFT_ERROR_PARAMS            DEF_PUB_SOFT_ERR(21)  //参数错误
#define SOFT_USER_CANCEL             DEF_PUB_SOFT_ERR(22)  //用户取消
#define SOFT_USER_TIMEOUT            DEF_PUB_SOFT_ERR(23)  //用户超时
#define SOFT_DEVICE_ALREADY_OPEN     DEF_PUB_SOFT_ERR(24)  //设备已经打开，不能Release
#define SOFT_ERROR_UNSUPP_DATA       DEF_PUB_SOFT_ERR(25)  //不支持的数据
#define SOFT_ERROR_FW_NOSUPP         DEF_PUB_SOFT_ERR(26)  //不支持的固件
#define SOFT_ERROR_FW_VERSION        DEF_PUB_SOFT_ERR(27)  //固件版本错误
#define SOFT_ERROR_FW_ERROR          DEF_PUB_SOFT_ERR(28)  //固件升级方式错误
#define SOFT_ERROR_FW_NONEED         DEF_PUB_SOFT_ERR(29)  //不需要升级固件
#define SOFT_ERROR_FW_ILLGE          DEF_PUB_SOFT_ERR(30)  //固件文件非法
#define SOFT_ERROR_UNKNOWNCODE       DEF_PUB_SOFT_ERR(31)  //不知道的错误码
#define SOFT_ERROR_INOP              DEF_PUB_SOFT_ERR(32)  //正在使用中
#define SOFT_ERROR_FW_NOFILE         DEF_PUB_SOFT_ERR(33)  //固件不存在
#define SOFT_ERROR_OTHER             DEF_PUB_SOFT_ERR(34)  //其他错误
#define SOFT_ERROR_INVALID_DATA      DEF_PUB_SOFT_ERR(35)  //无效的数据
#define SOFT_ERROR_NEED_UPDATE_FW    DEF_PUB_SOFT_ERR(37)  //需要更新固件
#define SOFT_ERROR_NEED_RESET        DEF_PUB_SOFT_ERR(38)  //需要复位
#define SOFT_ERROR_LOAD_FILE         DEF_PUB_SOFT_ERR(38)  //加载文件失败

//硬件层面的错误
#define DEF_PUB_HW_ERR(CODE) (HARDWAREERRORCODE(0x1000 | (0x0FFFF & CODE)))
//硬件不可恢复类错误
#define DEF_HW_BIG_ERR(CODE) (HARDWAREERRORCODE(0x8000 | CODE))
#define DEF_DEV_HW_ERR(CODE) (HARDWAREERRORCODE(0x2000 | (0x0FFFF & CODE)))
#define DEF_HW(CODE)         (HARDWAREERRORCODE((0x0FFFF & CODE)))

#define ERROR_CMD_UNSUPP        DEF_PUB_HW_ERR(1)   //不支持的通讯协议，或者不支持的命令
#define ERROR_CMD_CATCH         DEF_PUB_HW_ERR(2)   //驱动内部错误
#define ERROR_FW_FAILED         DEF_PUB_HW_ERR(3)   //固件升级失败
#define ERROR_HW_ERROR          DEF_PUB_HW_ERR(4)   //硬件故障
#define ERROR_DEVPORT_READERR   DEF_PUB_HW_ERR(5)   //读取错误
#define ERROR_DEVPORT_WRITE     DEF_PUB_HW_ERR(6)   //发送错误
#define ERROR_DEVPORT_RTIMEOUT  DEF_PUB_HW_ERR(7)   //操作超时
#define ERROR_DEVPORT_WTIMEOUT  DEF_PUB_HW_ERR(8)   //操作超时
#define ERROR_HW_DISASTER       DEF_HW_BIG_ERR(9)   //严重故障
#define ERROR_DEVPORT_DEVLOCKED DEF_PUB_HW_ERR(21)  //设备被锁定

/////////////////////////////////////////////////////////////////////////////////////
static const char *Dev_Error_Descript(long code)
{
    switch (ERRORCODE(code))
    {
    case DEV_SUCCESS:
        return "成功";
    case SOFT_ERROR_DEVPORT_NOTOPEN:
        return "端口打开失败";
    case SOFT_ERROR_DEVPORT_FAIL:
        return "通讯错误";
    case SOFT_ERROR_DEVPORT_PARAM:
        return "通讯参数错误";
    case SOFT_ERROR_DEVPORT_CANCELED:
        return "通讯操作";
    case SOFT_ERROR_DEVPORT_LIBRARY:
        return "加载通讯库失败";
    case SOFT_ERROR_DEVPORT_NODEFINED:
        return "通讯未知错误";
    case SOFT_ERROR_UNSUPP:
        return "不支持的命令";
    case SOFT_ERROR_NOT_OPEN:
        return "设备没有打开";
    case SOFT_ERROR_NOT_LOAD:
        return "驱动没有加载";
    case SOFT_ERROR_ALLOC_MEMEORY:
        return "分配内存错误";
    case CALL_DRIVER_ERROR:
        return "调用驱动接口错误";
    case SOFT_ERROR_PARAMS:
        return "参数错误";
    case SOFT_USER_CANCEL:
        return "用户取消";
    case SOFT_USER_TIMEOUT:
        return "用户超时";
    case SOFT_DEVICE_ALREADY_OPEN:
        return "设备已经打开，不能Release";
    case SOFT_ERROR_UNSUPP_DATA:
        return "不支持的数据";
    case SOFT_ERROR_FW_NOSUPP:
        return "不支持的固件";
    case SOFT_ERROR_FW_VERSION:
        return "固件版本错误";
    case SOFT_ERROR_FW_ERROR:
        return "固件升级方式错误";
    case SOFT_ERROR_FW_NONEED:
        return "不需要升级固件";
    case SOFT_ERROR_FW_ILLGE:
        return "固件文件非法";
    case SOFT_ERROR_UNKNOWNCODE:
        return "不知道的错误码";
    case SOFT_ERROR_INOP:
        return "正在使用中";
    case SOFT_ERROR_FW_NOFILE:
        return "固件不存在";
    case SOFT_ERROR_OTHER:
        return "其他错误";
    case SOFT_ERROR_INVALID_DATA:
        return "无效的数据";
    case SOFT_ERROR_NEED_UPDATE_FW:
        return "需要更新固件";
    case SOFT_ERROR_NEED_RESET:
        return "需要复位";
    case ERROR_CMD_UNSUPP:
        return "不支持的通讯协议，或者不支持的命令";
    case ERROR_CMD_CATCH:
        return "驱动内部错误";
    case ERROR_FW_FAILED:
        return "固件升级失败";
    case ERROR_HW_ERROR:
        return "硬件故障";
    case ERROR_DEVPORT_READERR:
        return "通讯错误,读取错误";
    case ERROR_DEVPORT_WRITE:
        return "通讯错误,发送错误";
    case ERROR_DEVPORT_RTIMEOUT:
        return "通讯错误,操作超时";
    case ERROR_DEVPORT_WTIMEOUT:
        return "通讯错误,操作超时";
    case ERROR_HW_DISASTER:
        return "严重故障";
    case ERROR_DEVPORT_DEVLOCKED:
        return "设备被锁定";
    default:
        return "其它错误";
    }
}
