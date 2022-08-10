/********************************************************************************
* 文件名称: device_port.cpp
* 文件描述: 定义静态库的函数(设备接口处理相关)
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年6月6日
* 文件版本: 1.0.0.1
*********************************************************************************/

#include "framework.h"
#include "device_object.h"


/*********************************************************************************
// 功能:	根据Vid+Pid取得摄像设备索引序号
// 参数:	入参: lpcVid:设备VID(16进制字符串,4位)
//            lpcVid:设备VID(16进制字符串,4位)
// 返回:	>=0:设备索引, -1/-1:实例化失败, -99:设备未找到， 其他<0:错误
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

            if (szGetVid != nullptr && szGetPid != nullptr)
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
// 功能:	查找设备名是否存在
// 参数:	入参: lpcDevName 设备名
// 返回:	参考返回值宏定义
*********************************************************************************/
INT CDevicePort::SearchDeviceNameIsHave(LPCSTR lpcDevName)
{
    return SearchDeviceIsHave(0, lpcDevName, nullptr, nullptr);
}

/*********************************************************************************
// 功能:	查找设备名是否存在
// 参数:	入参: lpcVid:设备VID(16进制字符串,4位)
//            lpcVid:设备VID(16进制字符串,4位)
// 返回:	参考返回值宏定义
*********************************************************************************/
INT CDevicePort::SearchDeviceVidPidIsHave(LPCSTR lpcVid, LPCSTR lpcPid)
{
    return SearchDeviceIsHave(1, nullptr, lpcVid, lpcPid);
}

/*********************************************************************************
// 功能:	查找/dev/videoX是否存在
// 参数:	入参: wVideoX:
// 返回:	参考返回值宏定义
*********************************************************************************/
INT CDevicePort::SearchVideoXIsHave(WORD wVideoX)
{
    CHAR szVideoID[24] = { 0x00 };
    sprintf(szVideoID, "%d", wVideoX);

    return SearchDeviceIsHave(2, szVideoID, nullptr, nullptr);
}

/*********************************************************************************
// 功能:	查找摄像设备索引序号是否存在
// 参数:	入参: wVideoX:
// 返回:	参考返回值宏定义
*********************************************************************************/
INT CDevicePort::SearchVideoIdxIsHave(WORD wIdx)
{
    CHAR szVideoID[24] = { 0x00 };
    sprintf(szVideoID, "%d", wIdx);

    return SearchDeviceIsHave(3, szVideoID, nullptr, nullptr);
}

/*********************************************************************************
// 功能:	查找设备名是否存在
// 参数:	入参: lpcVid:设备VID(16进制字符串,4位)
//           lpcVid:设备VID(16进制字符串,4位)
// 返回:	参考返回值宏定义
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
// 功能:	查找设备是否存在
// 参数:	入参: wMode: 查找方式(0设备名, 1VidPid, 2videoX, 3摄像设备索引)
//            lpcVid:设备VID(16进制字符串,4位)
//            lpcVid:设备VID(16进制字符串,4位)
// 返回:	参考返回值宏定义
*********************************************************************************/
INT CDevicePort::SearchDeviceIsHave(WORD wMode, LPCSTR lpcDevName, LPCSTR lpcVid, LPCSTR lpcPid)
{
    INT nRet = DP_RET_NOTHAVE;
    struct udev *stUDev = nullptr;
    struct udev_enumerate *stUDevEnumErate = nullptr;
    struct udev_list_entry *stUDevices = nullptr, *stUDevList = nullptr;
    struct udev_device *stDevice = nullptr;
    //struct udev_device *stDevice2 = nullptr;

    CHAR szInVid[1024] = { 0x00 };
    CHAR szInPid[1024] = { 0x00 };
    CHAR szGetVid[1024] = { 0x00 };
    CHAR szGetPid[1024] = { 0x00 };

    // 设备检索关键字列表
    static struct
    {
        WORD wMode;
        CHAR szKey[64];
    } stDevSearchList[] =
        {
            { 0, "tty" },  { 0, "hidraw" },
            { 1, "hidraw" },  { 1, "video4linux" },
            { 2, "video4linux"},
            { 3, "video4linux"},
        };

    // 指定搜索关键字
    if (wMode == 0 || wMode == 2 || wMode == 3)     // 设备名方式/摄像ID查找
    {
        if (lpcDevName == nullptr)
        {
            return DP_RET_NOTHAVE;
        }
    } else
    if (wMode == 1)     // VidPid方式查找
    {
        if (lpcVid == nullptr || lpcPid == nullptr)
        {
            return DP_RET_NOTHAVE;
        }
        // VidPid转换为大写
        str_to_toupper(lpcVid, strlen(lpcVid), szInVid, sizeof(szInVid));
        str_to_toupper(lpcPid, strlen(lpcPid), szInPid, sizeof(szInPid));
    } else
    {
        return DP_RET_INPUT_INV;
    }

    // 实例化
    stUDev = udev_new();
    if (stUDev == nullptr)
    {
        return DP_RET_INST_ERR;
    }

    // 循环检索
    for(INT i = 0; i < sizeof(stDevSearchList) / sizeof(stDevSearchList[0]); i ++)
    {
        if (stDevSearchList[i].wMode == wMode)
        {
            stUDevEnumErate = udev_enumerate_new(stUDev);
            if (stUDevEnumErate == nullptr)
            {
                return DP_RET_INST_ERR;
            }

            udev_enumerate_add_match_subsystem(stUDevEnumErate, stDevSearchList[i].szKey);
            udev_enumerate_scan_devices(stUDevEnumErate);
            stUDevices = udev_enumerate_get_list_entry(stUDevEnumErate);

            udev_list_entry_foreach(stUDevList, stUDevices)
            {
                const char *path = udev_list_entry_get_name(stUDevList);
                stDevice = udev_device_new_from_syspath(stUDev, path);
                if (stDevice == nullptr)
                {
                    break;
                }

                if (wMode == 0)     // 设备名
                {
                    const char *node = udev_device_get_devnode(stDevice);
                    if (node != nullptr)
                    {
                        if (MCMP_IS0(lpcDevName, node))
                        {
                            nRet = DP_RET_SUCC;
                            break;
                        }
                    }
                } else
                if (wMode == 1)     // VidPid
                {
                    struct udev_device *stDevice2 = nullptr;
                    stDevice2 = udev_device_get_parent_with_subsystem_devtype(stDevice,
                                                                             "usb", "usb_device");
                    if (stDevice2 == nullptr)
                    {
                        break;
                    }

                    const char *szVid = udev_device_get_sysattr_value(stDevice2, "idVendor");
                    const char *szPid = udev_device_get_sysattr_value(stDevice2, "idProduct");
                    if (szVid != nullptr || szPid != nullptr)
                    {
                        str_to_toupper(szVid, strlen(szVid), szGetVid, sizeof(szGetVid));
                        str_to_toupper(szPid, strlen(szPid), szGetPid, sizeof(szGetPid));
                        if (MCMP_IS0(szInVid, szGetVid) && MCMP_IS0(szInPid, szGetPid))
                        {
                            nRet = DP_RET_SUCC;
                            break;
                        }
                    }
                } else
                if (wMode == 2)     // /dev/videoX
                {
                    const char *szVideoX = udev_device_get_sysnum(stDevice);
                    if (szVideoX != nullptr)
                    {
                        if (MCMP_IS0(lpcDevName, szVideoX))
                        {
                            nRet = DP_RET_SUCC;
                            break;
                        }
                    }
                }
            }

            if (stDevice != nullptr)
            {
                udev_device_unref(stDevice);
            }

            if (stUDevEnumErate != nullptr)
            {
                udev_enumerate_unref(stUDevEnumErate);
            }

            if (nRet == DP_RET_SUCC)
            {
                break;
            }
        }
    }

    if (stUDev != nullptr)
    {
        udev_unref(stUDev);
    }

    return nRet;
}

/*********************************************************************************
// 功能: 字串转换为大写
// 参数:	lpcSource: 转换源字串   dwSourceSize: 转换源字串大小
//      lpDest: 返回转换后字串空间  dwDestSize: 返回转换后字串大小
// 返回:	转换成功, 返回转换的字串长度, 字串无效或不成功返回0
*********************************************************************************/
DWORD CDevicePort::str_to_toupper(LPCSTR lpcSource, DWORD dwSourceSize, LPSTR lpDest, DWORD dwDestSize)
{
    DWORD dwSize = 0;
    CHAR szStrTmp[2] = { 0x00 };

    if (lpcSource == nullptr || dwSourceSize < 1)
    {
        return 0;
    }

    if (lpDest == nullptr || dwDestSize < 1)
    {
        return 0;
    }

    std::string stdToupper = "";
    for(INT i = 0; i < dwSourceSize; i ++)
    {
        szStrTmp[0] = lpcSource[i];
        if (szStrTmp[0] != 0x00)
        {
            if (szStrTmp[0] >= 'a' && szStrTmp[0] <= 'z')
            {
                szStrTmp[0] -= 32;
            }
            stdToupper.append(szStrTmp);
        } else
        {
            break;
        }
    }

    dwSize = stdToupper.length();

    memcpy(lpDest, stdToupper.c_str(), dwSize >= dwDestSize ? dwDestSize : dwSize);

    return dwSize;
}

// -------------------------------------- END --------------------------------------
