/***************************************************************************
* 文件名称: XFS_CAM_DEC.h
* 文件描述: 摄像头模块子定义 头文件
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2021年4月4日
* 文件版本: 1.0.0.1
***************************************************************************/
#pragma once

#ifndef XFS_CAM_DEC_H
#define XFS_CAM_DEC_H


#include "QtTypeInclude.h"
#include "IDevCAM.h"
#include "def.h"
#include "ErrorDetail.h"

/***************************************************************************
// 未分类 宏定义
***************************************************************************/
// 状态非ONLINE||BUSY时,返回HWERR
#define DEV_STAT_RET_HWERR(DSTAT) \
    if (DSTAT != WFS_CAM_DEVONLINE && DSTAT != WFS_CAM_DEVBUSY) \
    { \
        Log(ThisModule, __LINE__, "Device Status != ONLINE | BUSY, Return: %d|HWERR.", WFS_ERR_HARDWARE_ERROR); \
        switch(DSTAT) \
        { \
            case WFS_CAM_DEVOFFLINE: SetErrorDetail((LPSTR)EC_XFS_DevOffLine); break; \
            case WFS_CAM_DEVNODEVICE: SetErrorDetail((LPSTR)EC_XFS_DevNotFound); break; \
            case WFS_CAM_DEVHWERROR: SetErrorDetail((LPSTR)EC_XFS_DevHWErr); break; \
            case WFS_CAM_DEVPOWEROFF: SetErrorDetail((LPSTR)EC_XFS_DevPowerOff); break; \
            case WFS_CAM_DEVUSERERROR: SetErrorDetail((LPSTR)EC_XFS_DevUserErr); break; \
            case WFS_CAM_DEVFRAUDATTEMPT: SetErrorDetail((LPSTR)EC_XFS_DevFraud); break; \
        } \
        return WFS_ERR_HARDWARE_ERROR; \
    }

// 检查是否有效设备类型
#define CHK_ISDEVTYPE(DT) \
    ((DT == XFS_YC0C98 || DT == XFS_TCF261 || DT == XFS_ZLF1000A3 || DT == XFS_JDY5001A0809) ? TRUE : FALSE)


// 摄像窗口运行方式
#define WIN_RUN_SDK             0x01        // SDK自带窗口处理
#define WIN_RUN_SHARED          0x02        // SP创建窗口(共享内存方式)
#define WIN_RUN_SHARED_PROG     0x04        // SP创建窗口(共享内存方式外接程序)

// 支持的设备总数目
#define DEV_CNT                 32          // 支持32个设备

// 摄像模式
#define CAM_MODE_ROOM           1           // 环境摄像
#define CAM_MODE_PERSON         2           // 人脸摄像
#define CAM_MODE_EXITSLOT       4           // 出口槽摄像
#define CAM_MODE_EXTRA          8           // 扩展摄像
#define CAM_MODE_HIGHT          16          // 高拍摄像
#define CAM_MODE_PANORA         32          // 全景摄像

// 适用 设备模式启用及对应设备数组/DevXXX调用实例数组 变量下标
#define LCMODE_CNT              6           // 支持的模式总数
#define LCMODE_ROOM             0           // 该下标固定为:环境摄像模式
#define LCMODE_PERSON           1           // 该下标固定为:人脸摄像模式
#define LCMODE_EXITSLOT         2           // 该下标固定为:出口槽摄像模式
#define LCMODE_EXTRA            3           // 该下标固定为:扩展摄像模式
#define LCMODE_HIGHT            4           // 该下标固定为:高拍摄像模式
#define LCMODE_PANORA           5           // 该下标固定为:全景摄像模式

// 设备相关变量列表索引
#define LIDX_YC0C98             XFS_YC0C98
#define LIDX_TCF261             XFS_TCF261
#define LIDX_ZLF1000A3          XFS_ZLF1000A3
#define LIDX_JDY5001A0809       XFS_JDY5001A0809

// LCMODE_XXX到CAM_MODE_XXX的转换
#define LCMODE_TO_CMODE(LM) \
    (LM == LCMODE_ROOM ? CAM_MODE_ROOM : \
     (LM == LCMODE_PERSON ? CAM_MODE_PERSON : \
      (LM == LCMODE_EXITSLOT ? CAM_MODE_EXITSLOT : \
       (LM == LCMODE_EXTRA ? CAM_MODE_EXTRA : \
        (LM == LCMODE_HIGHT ? CAM_MODE_HIGHT : \
          CAM_MODE_PANORA)))))

// CAM_MODE_XXX到LCMODE_XXX的转换
#define CMODE_TO_LCMODE(LM) \
    (LM == CAM_MODE_ROOM ? LCMODE_ROOM : \
     (LM == CAM_MODE_PERSON ? LCMODE_PERSON : \
      (LM == CAM_MODE_EXITSLOT ? LCMODE_EXITSLOT : \
       (LM == CAM_MODE_EXTRA ? LCMODE_EXTRA : \
        (LM == CAM_MODE_HIGHT ? LCMODE_HIGHT : \
          LCMODE_PANORA)))))

// LCMODE_XXX到WFS_CAM_XXX的转换
#define LCMODE_TO_WMODE(LM) \
    (LM == LCMODE_ROOM ? WFS_CAM_ROOM : \
     (LM == LCMODE_PERSON ? WFS_CAM_PERSON : \
      (LM == LCMODE_EXITSLOT ? WFS_CAM_EXITSLOT : \
       (LM == LCMODE_EXTRA ? WFS_CAM_EXTRA : \
        (LM == LCMODE_HIGHT ? WFS_CAM_HIGHTCAMERA : \
          WFS_CAM_PANORAMIC)))))

// WFS_CAM_XXX到LCMODE_XXX的转换
#define WMODE_TO_LCMODE(LM) \
    (LM == WFS_CAM_ROOM ? LCMODE_ROOM : \
     (LM == WFS_CAM_PERSON ? LCMODE_PERSON : \
      (LM == WFS_CAM_EXITSLOT ? LCMODE_EXITSLOT : \
       (LM == WFS_CAM_EXTRA ? LCMODE_EXTRA : \
        (LM == WFS_CAM_HIGHTCAMERA ? LCMODE_HIGHT : \
          LCMODE_PANORA)))))

// WFS_CAM_XXX到CAM_MODE_XXX的转换
#define WMODE_TO_CMODE(LM) \
    (LM == WFS_CAM_ROOM ? CAM_MODE_ROOM : \
     (LM == WFS_CAM_PERSON ? CAM_MODE_PERSON : \
      (LM == WFS_CAM_EXITSLOT ? CAM_MODE_EXITSLOT : \
       (LM == WFS_CAM_EXTRA ? CAM_MODE_EXTRA : \
        (LM == WFS_CAM_HIGHTCAMERA ? CAM_MODE_HIGHT : \
          CAM_MODE_PANORA)))))

// WFS_CAM_XXX到DEV_CAM_MODE_XXX的转换
#define WMODE_TO_DMODE(LM) \
    (LM == WFS_CAM_ROOM ? DEV_CAM_MODE_ROOM : \
     (LM == WFS_CAM_PERSON ? DEV_CAM_MODE_PERSON : \
      (LM == WFS_CAM_EXITSLOT ? DEV_CAM_MODE_EXITSLOT : \
       (LM == WFS_CAM_EXTRA ? DEV_CAM_MODE_EXTRA : \
        (LM == WFS_CAM_HIGHTCAMERA ? DEV_CAM_MODE_HIGHT : \
          DEV_CAM_MODE_PANORA)))))

// 摄像模式中文解析
static CHAR szCamModeStr[LCMODE_CNT][256] =
    {   "ROOM模式", "PERSON模式", "EXITSLOT模式", "EXTRA模式",
        "HIGHTCAMERA模式", "PANORAMIC模式" };

// 设备模式结构体
// stModeList: 模式列表, 下标参考宏定义LCMODE_XXX, 对应WFS标准模式
typedef struct ST_CAM_MODE_INFO
{
    struct
    {
        BOOL bEnable;                       // 摄像模式是否启用
        WORD wDeviceType;                   // 设备类型(INI定义的设备类型, 0为无效/不启用)
        CQtDLLLoader<IDevCAM>   m_pCAMDev;  // 设备连接句柄(DevCAM)
        CHAR szModeName[128];               // 摄像模式中文解析(参考static CHAR szCamModeStr)
    } stModeList[LCMODE_CNT];

    ST_CAM_MODE_INFO()
    {
        Clear();
    }

    void Clear()
    {
        memset(this, 0x00, sizeof(ST_CAM_MODE_INFO));
        for (INT i = 0; i < LCMODE_CNT; i ++)
        {
            stModeList[i].bEnable = FALSE;
            stModeList[i].wDeviceType = 0;
            stModeList[i].m_pCAMDev = nullptr;
        }
    }

    // 设置设备类型及相关变量
    INT SetDeviceType(WORD wMode, WORD wDevice)
    {
        if (wMode >= LCMODE_CNT)
        {
            return -1;
        }
        stModeList[wMode].wDeviceType =
                (CHK_ISDEVTYPE(wDevice) == TRUE ? wDevice : 0);
        if (stModeList[wMode].wDeviceType > 0)
        {
            stModeList[wMode].bEnable = TRUE;
        }
        MCPY_NOLEN(stModeList[wMode].szModeName, szCamModeStr[wMode]);

        return 0;
    }

    // 返回指定模式的设备类型
    INT GetDeviceType(WORD wMode)
    {
        if (wMode >= LCMODE_CNT)
        {
            return 0;
        }
        return stModeList[wMode].wDeviceType;
    }

    // 检索是否存在指定设备类型
    BOOL SearchIsDeviceType(WORD wDevice)
    {
        for (INT i = 0; i < LCMODE_CNT; i ++)
        {
            if (stModeList[i].wDeviceType == wDevice)
            {
                return TRUE;
            }
        }
        return FALSE;
    }

    // 设备模式是否处于启用状态
    BOOL IsModeEnable(WORD wMode)
    {
        if (wMode >= LCMODE_CNT)
        {
            return FALSE;
        }

        if (stModeList[wMode].bEnable == TRUE &&
            stModeList[wMode].wDeviceType > 0)
        {
            return TRUE;
        }

        return FALSE;
    }

    // 设备连接句柄
    CQtDLLLoader<IDevCAM>* GetDevDllHandle(WORD wMode)
    {
        if (wMode >= LCMODE_CNT)
        {
            return nullptr;
        }

        for (INT i = 0; i < LCMODE_CNT; i ++)
        {
            if (i == wMode)
            {
                return &stModeList[i].m_pCAMDev;
            }
        }

        return nullptr;
    }

    // 释放DexCAM连接句柄
    INT ReleaseDevCAMDll(WORD wMode)
    {
        if (wMode >= LCMODE_CNT)
        {
            return -1;
        }

        if (stModeList[wMode].m_pCAMDev != nullptr)
        {
            stModeList[wMode].m_pCAMDev.Release();
            stModeList[wMode].m_pCAMDev = nullptr;
        }

        return 0;
    }

    // 释放DevCAM连接句柄
    INT ReleaseDevCAMDllAll()
    {
        for (INT i = 0; i < LCMODE_CNT; i ++)
        {
            if (stModeList[i].m_pCAMDev != nullptr)
            {
                stModeList[i].m_pCAMDev.Release();
                stModeList[i].m_pCAMDev = nullptr;
            }
        }

        return 0;
    }

} STCAMMODEINFO, *LPSTCAMMODEINFO;

// 以下宏定义用于简化ST_CAM_MODE_INFO结构体调用接口
#define STNAME              m_stCamModeInfo                 // 指定CXFS_CAM类中定义的结构体变量名(必须项)
#define MI_SetDevType(m, d) STNAME.SetDeviceType(m, d)      // 设置设备类型及相关变量
#define MI_GetDevType(m)    STNAME.GetDeviceType(m)         // 返回指定模式的设备类型
#define MI_IsDevType(d)     STNAME.SearchIsDeviceType(d)    // 检索是否存在指定设备类型
#define MI_Enable(m)        STNAME.IsModeEnable(m)          // 设备模式是否处于启用状态
#define MI_DevDll(m)        STNAME.stModeList[m].m_pCAMDev  // 取对应模式的设备连接句柄
#define MI_RelDevDll(m)     STNAME.ReleaseDevCAMDll(m)      // 释放DexCAM连接句柄
#define MI_RelDevDllA(m)    STNAME.ReleaseDevCAMDllAll(m)   // 释放DexCAM连接句柄
#define MI_ModeName(m)      STNAME.stModeList[m].szModeName // 取对应模式的中文解析

#endif // XFS_CAM_DEC_H

// -------------------------------------- END --------------------------------------
