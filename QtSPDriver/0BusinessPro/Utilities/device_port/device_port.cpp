/********************************************************************************
* 文件名称: device_port.cpp
* 文件描述: 定义静态库的函数(设备处理相关)
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年6月6日
* 文件版本: 1.0.0.1
*********************************************************************************/

#include "framework.h"
#include "device_port.h"


/*********************************************************************************
 @功能:	根据Vid+Pid取得摄像设备索引序号
 @参数:	入参: lpcVid:设备VID(16进制字符串,4位)
             lpcVid:设备VID(16进制字符串,4位)
 @返回:	>=0:设备索引, -1/-1:实例化失败, -99:设备未找到， 其他<0:错误
*********************************************************************************/
INT CDevicePort::SearchVideoIdxFromVidPid(LPCSTR lpcVid, LPCSTR lpcPid)
{
    struct udev *stUDev = nullptr;
    struct udev_enumerate *stUDevEnumErate = nullptr;
    struct udev_list_entry *stUDevList = nullptr;
    INT nVideoIdx = 0;

    // 实例化
    stUDev = udev_new();
    if (stUDev == nullptr)
    {
        return DP_RET_INST_ERR;
    }

    //
    stUDevEnumErate = udev_enumerate_new(stUDev);
    if (stUDevEnumErate == nullptr)
    {
        return DP_RET_INST_ERR;
    }

    //
    udev_enumerate_add_match_subsystem(stUDevEnumErate, "video4linux");
    udev_enumerate_scan_devices(stUDevEnumErate);
    udev_list_entry_foreach(stUDevList, udev_enumerate_get_list_entry(stUDevEnumErate))
    {
        struct udev_device *stDevice = nullptr;
        stDevice = udev_device_new_from_syspath(udev_enumerate_get_udev(stUDevEnumErate),
                                                udev_list_entry_get_name(stUDevList));
        if (stDevice != nullptr)
        {
            const CHAR *szGetVid = nullptr;
            const CHAR *szGetPid = nullptr;
            szGetVid = udev_device_get_property_value(stDevice, "ID_VENDOR_ID");
            szGetPid = udev_device_get_property_value(stDevice, "ID_MODEL_ID");

            if (szGetVid != nullptr && szGetVid != nullptr)
            {
                if (MCMP_IS0(szGetVid, lpcVid) && MCMP_IS0(szGetPid, lpcPid))
                {
                    nVideoIdx = atoi(udev_device_get_sysnum(stDevice));
                    udev_device_unref(stDevice);
                    udev_enumerate_unref(stUDevEnumErate);
                    udev_unref(stUDev);
                    return nVideoIdx;
                }
            }
            udev_device_unref(stDevice);
        } else
        {
            udev_enumerate_unref(stUDevEnumErate);
            udev_unref(stUDev);
            return DP_RET_NOTHAVE;
        }
    }

    return DP_RET_NOTHAVE;
}

/*********************************************************************************
 @功能:	查找设备名是否存在
 @参数:	入参: lpcDevName 设备名
 @返回:	参考返回值宏定义
*********************************************************************************/
INT CDevicePort::SearchDeviceNameIsHave(LPCSTR lpcDevName)
{
    return SearchDeviceIsHave(0, lpcDevName, "", "");
}

/*********************************************************************************
 @功能:	查找设备名是否存在
 @参数:	入参: lpcVid:设备VID(16进制字符串,4位)
             lpcVid:设备VID(16进制字符串,4位)
 @返回:	参考返回值宏定义
*********************************************************************************/
INT CDevicePort::SearchDeviceVidPidIsHave(LPCSTR lpcVid, LPCSTR lpcPid)
{
    return SearchDeviceIsHave(1, "", lpcVid, lpcPid);
}

/*********************************************************************************
 @功能:	查找设备名是否存在
 @参数:	入参: lpcVid:设备VID(16进制字符串,4位)
             lpcVid:设备VID(16进制字符串,4位)
 @返回:	参考返回值宏定义
*********************************************************************************/
INT CDevicePort::GetComDevName(LPCSTR lpMode, LPSTR lpDevName, INT nDevNameLen)
{
    // 格式lpMode: 串口=COM:/dev/ttyUSB0:9600,N,8,1
    QString strMode = QString::fromLocal8Bit(lpMode);
    QStringList strList = strMode.split(QRegExp("[:,]"), QString::SkipEmptyParts);
    if (strList.size() != 6)
    {
        return DP_RET_INPUT_INV;
    }
    if (strList[0] != "COM")
    {
        return DP_RET_INPUT_INV;
    }

    MCPY_LEN(lpDevName, strList[1].toStdString().c_str(), strList[1].toStdString().length());

    return DP_RET_SUCC;
}

/*********************************************************************************
 @功能:	查找设备是否存在
 @参数:	入参: wMode: 查找方式(0设备名, 1VidPid)
             lpcVid:设备VID(16进制字符串,4位)
             lpcVid:设备VID(16进制字符串,4位)
 @返回:	参考返回值宏定义
*********************************************************************************/
INT CDevicePort::SearchDeviceIsHave(WORD wMode, LPCSTR lpcDevName, LPCSTR lpcVid, LPCSTR lpcPid)
{
    INT nRet = DP_RET_NOTHAVE;
    CHAR szDevKey[32] = { 0x00 };// 搜索关键字
    struct udev *stUDev = nullptr;
    struct udev_enumerate *stUDevEnumErate = nullptr;
    struct udev_list_entry *stUDevices = nullptr, *stUDevList = nullptr;
    struct udev_device *stDevice = nullptr;

    //char szbuff[10240] = { 0x00 };      // 测试用

    // 指定搜索关键字
    if (wMode == 0)     // 设备名方式查找
    {
        if (memcmp(lpcDevName, "/dev/tty", strlen("/dev/tty")) == 0)
        {
            sprintf(szDevKey, "tty");
        } else
        if (memcmp(lpcDevName, "/dev/hidraw", strlen("/dev/hidraw")) == 0)
        {
            sprintf(szDevKey, "hidraw");
        } else
        {
            return DP_RET_INPUT_INV;
        }
    }

    // 实例化
    stUDev = udev_new();
    if (stUDev == nullptr)
    {
        return DP_RET_INST_ERR;
    }

    //
    stUDevEnumErate = udev_enumerate_new(stUDev);
    if (stUDevEnumErate == nullptr)
    {
        return DP_RET_INST_ERR;
    }

    //
    udev_enumerate_add_match_subsystem(stUDevEnumErate, szDevKey);
    udev_enumerate_scan_devices(stUDevEnumErate);
    stUDevices = udev_enumerate_get_list_entry(stUDevEnumErate);

    udev_list_entry_foreach(stUDevList, stUDevices)
    {
        const char *path;
        path = udev_list_entry_get_name(stUDevList);
        stDevice = udev_device_new_from_syspath(stUDev, path);
        const char *node = udev_device_get_devnode(stDevice);

        //sprintf(szbuff + strlen(szbuff), "%s, ", node);

        if (node != nullptr)
        {
            if (MCMP_IS0(lpcDevName, node))
            {
                nRet = DP_RET_SUCC;
                break;
            }
        }

        stDevice = udev_device_get_parent_with_subsystem_devtype(stDevice, "usb", "usb_device");
        if (!stDevice)
        {
            break;
        }
    }

    udev_device_unref(stDevice);
    udev_enumerate_unref(stUDevEnumErate);
    udev_unref(stUDev);

    return nRet;
}

