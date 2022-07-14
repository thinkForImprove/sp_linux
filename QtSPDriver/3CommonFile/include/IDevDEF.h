/***************************************************************************
* 文件名称: IDevDEF.h
* 文件描述: 声明IDevXXX.h中通用定义
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年7月13日
* 文件版本: 1.0.0.1
***************************************************************************/

#ifndef IDEVDEF_H
#define IDEVDEF_H

#include "QtTypeDef.h"

//***************************************************************************
// SetData/GetData 数据类型 (0~50为共通使用, 50以上为各模块自行定义)
//***************************************************************************
#define SET_DEV_INIT            0       // 设置初始化数据
#define SET_LIB_PATH            1       // 设置动态库路径
#define SET_DEV_OPENMODE        2       // 设置设备打开模式
#define SET_GLIGHT_CONTROL      3       // 指示灯控制
#define SET_BEEP_CONTROL        4       // 设备鸣响控制
#define SET_DEV_RECON           5       // 设置断线重连标记
#define GET_DEV_ERRCODE         6       // 取DevXXX错误码

//***************************************************************************
// GetVersion 数据类型
//***************************************************************************
#define GET_VER_FW              0       // 固件版本
#define GET_VER_SOFT            1       // 软件版本
#define GET_VER_LIB             2       // SDK动态库版本
#define GET_VER_SERIAL          3       // 序列号
#define GET_VER_PN              4       // 设备编号
#define GET_VER_SN              5       // 设备出厂编号


//***************************************************************************
// 设备打开方式结构体变量 声明
// 适用于 SET_DEV_OPENMODE 参数传递(兼容多种方式)
//***************************************************************************
typedef
struct st_Device_OpenMode
{
    WORD    wOpenMode;              // 设备打开方式(0串口/1USB HID/>2其他自选方式)
    CHAR    szDevPath[8][64+1];     // 设备路径或其他路径[最多可设置8组]
    WORD    wBaudRate[8];           // 设备串口波特率或其他整型参数[最多可设置8组]
    CHAR    szHidVid[8][32+1];      // 设备VID(字符串)[最多可设置8组]
    CHAR    szHidPid[8][32+1];      // 设备PID(字符串)[最多可设置8组]
    INT     nHidVid[8];             // 设备VID(整型)[最多可设置8组]
    INT     nHidPid[8];             // 设备PID(整型)[最多可设置8组]
    WORD    wProtocol[8];           // 通讯协议[最多可设置8组]
    INT     nOtherParam[32];        // 其他参数(整型数组)
    CHAR    szOtherParam[256];      // 其他参数(字符串)

    st_Device_OpenMode()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_Device_OpenMode));
    }
} STDEVICEOPENMODE, *LPSTDEVICEOPENMODE;

//***************************************************************************
// 设备状态相关定义
//***************************************************************************
//　Status.Device返回状态
typedef
enum DEVICE_STATUS
{
    DEVICE_STAT_ONLINE              = 0,    // 设备正常
    DEVICE_STAT_OFFLINE             = 1,    // 设备脱机
    DEVICE_STAT_POWEROFF            = 2,    // 设备断电
    DEVICE_STAT_NODEVICE            = 3,    // 设备不存在
    DEVICE_STAT_HWERROR             = 4,    // 设备故障
    DEVICE_STAT_USERERROR           = 5,    // 用户操作故障
    DEVICE_STAT_BUSY                = 6,    // 设备读写中
    DEVICE_STAT_FRAUDAT             = 7,    // 设备出现欺诈企图
} EN_DEVICE_STATUS;


#endif // IDEVDEF_H

// -------------------------------- END -----------------------------------
