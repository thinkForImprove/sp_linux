/***************************************************************************
* 文件名称: XFS_CAM.h
* 文件描述: 摄像头模块命令处理接口 头文件
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2021年4月4日
* 文件版本: 1.0.0.1
***************************************************************************/
#pragma once

#ifndef XFS_CAM_H
#define XFS_CAM_H

#include <QWidget>
#include <QMainWindow>
#include <algorithm>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <QSettings>
#include <QMetaType>
#include <QWindow>

#include "IDevCAM.h"
#include "ISPBaseCAM.h"
#include "QtTypeInclude.h"
#include "def.h"
#include "file_access.h"
#include "opencv2/opencv.hpp"
#include "ErrorDetail.h"
#include "XfsHelper.h"

#include "mainwindow.h"

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


/***************************************************************************
// 主处理类
***************************************************************************/
class CXFS_CAM : public QObject, public ICmdFunc, public CLogManage, public ConvertVarCAM
{
    Q_OBJECT

public:
    CXFS_CAM();
    ~CXFS_CAM();

public:
    // 开始运行SP
    long StartRun();

protected:
    // 基本接口
    virtual HRESULT OnOpen(LPCSTR lpLogicalName);
    virtual HRESULT OnClose();
    virtual HRESULT OnStatus();
    virtual HRESULT OnCancelAsyncRequest();
    virtual HRESULT OnUpdateDevPDL();

    // CAM接口
    virtual HRESULT GetStatus(LPWFSCAMSTATUS &lpStatus);
    virtual HRESULT GetCapabilities(LPWFSCAMCAPS &lpCaps);
    virtual HRESULT TakePicture(const WFSCAMTAKEPICT &stTakePict, DWORD dwTimeout);
    virtual HRESULT TakePictureEx(const WFSCAMTAKEPICTEX &stTakePict, DWORD dwTimeout);
    virtual HRESULT Display(const WFSCAMDISP &stDisplay, DWORD dwTimeout);
    virtual HRESULT Reset();


private:    // XFS_CAM_DEC.cpp 子处理相关接口
    HRESULT InnerOpen(BOOL bReConn = FALSE);                        // Open设备及初始化相关子处理
    BOOL LoadDevCAMDll(LPCSTR ThisModule);                          // 加载DevXXX动态库
    BOOL UnLoadDevCAMDll();                                         // 卸载DevXXX动态库
    INT InitConfig();                                               // 加载INI设置
    INT PrintIniData();                                             // INI配置输出
    void InitStatus();                                              // 状态结构体实例初始化
    void InitCaps();                                                // 能力值结构体实例初始化
    void UpdateExtra();                                             // 更新扩展数据
    WORD UpdateDeviceStatus();                                      // 设备状态实时更新
    HRESULT InnerDisplay(const WFSCAMDISP &stDisplay, DWORD dwTimeout);// 摄像窗口处理
    HRESULT InnerTakePicture(const WFSCAMTAKEPICT &stTakePict, DWORD dwTimeout);// 拍照处理
    HRESULT InnerTakePictureEx(const WFSCAMTAKEPICTEX &stTakePict, DWORD dwTimeout);// 拍照处理扩展
    BOOL bCreateSharedMemory(LPSTR lpMemName, ULONG ulSize);        // 创建摄像共享内存
    void SharedMemRelease();                                        // 销毁摄像共享内存
    INT InnerTakePicAfterMothod();                                  // TakePicture命令完成后处理
    INT SetErrorDetail(LPSTR lpCode = nullptr, INT nIdx = -1);      // 设置ErrorDetail错误码
    HRESULT ChkOpenIsCamDevMode();                                  // Open前检查INI设定摄像模式和设备模式是否正常
    HRESULT ChkOpenShowWinMode();                                   // Open前检查摄像窗口显示模式
    BOOL OpenFrontChkDevIsOpen(WORD wCamMode, BOOL bIsLog);         // 检查是否有同一设备已Open,有则共享连接
    INT OpenFrontSendDevParam(BOOL bReConn, WORD wCamMode,
                              WORD wDeviceType);                    // 设备Open前参数传递
    INT InitConfigDef(LPCSTR lpsKeyName, WORD wDeviceType);         // INI共通变量配置获取

private:    // XFS_CAM_FIRE.cpp 事件消息子处理相关接口
    void FireHWEvent(DWORD dwHWAct, char *pErr);
    void FireStatusChanged(WORD wStatus);
    void FireEXEE_LIVEERROR_Event(WORD wError);                     // 活检状态事件,非标准事件

private:
    CXfsRegValue                    m_cXfsReg;
    std::string                     m_strLogicalName;
    std::string                     m_strSPName;
    CSimpleMutex                    *m_pMutexGetStatus;
    CQtDLLLoader<ISPBaseCAM>        m_pBase;
    STINICONFIG                     m_stConfig;                     // INI结构体
    STCAMMODEINFO                   m_stCamModeInfo;                // 摄像模式结构体变量
    CWfsCAMStatus                   m_stStatus;                     // 状态结构体
    CWfsCAMStatus                   m_stStatusOLD;                  // 备份上一次状态结构体
    CWfsCAMCap                      m_stCaps;                       // 能力值结构体
    CExtraInforHelper               m_cStatExtra;                   // 状态扩展数据
    CExtraInforHelper               m_cCapsExtra;                   // 能力值扩展数据
    INT                             m_nRetErrOLD[32];               // 处理错误值保存(0:重新Open/1~32:模式Open)
    CHAR                            m_szFileName[MAX_PATH];         // 拍照文件名
    MainWindow                      *showWin;                       // 摄像窗口
    QSharedMemory                   *m_qSharedMemData;              // 摄像数据共享内存
    BOOL                            m_bCmdRunning;                  // 是否命令执行中
    WORD                            m_wDisplayOK;                   // Display是否已执行
    CErrorDetail                    m_clErrorDet;                   // ErrorDetail处理类实例
    WORD                            m_wDeviceShowWinMode[DEV_CNT];  // 设备窗口显示方式, 通过INI设定转换为WIN_RUN_XXX
                                                                    // 以设备类型为下标, LIDX_XXX,未定义位保留备用
    CHAR                            m_szWinProcessPath[DEV_CNT][256];// 摄像窗口外接程序, 对应"设备窗口显示方式", 外接程序窗口使用
    WORD                            m_wIsDevOpenFlag;               // 设备Open标记, 用于记录Open是否成功/断线(参考宏定义CAM_MODE_XXX)
    WORD                            m_wWindowsRunMode;              // 当前已启动的窗口运行的摄像模式(参考CAM_MODE_XXX)
    CHAR                            m_szBuffer[1024];               // 公用Buffer

private:    // 活检图像状态事件监听线程相关
    BOOL                           m_bLiveEventWaitExit;            // 活检状态监听事件处理退出标记
    std::thread                    m_thRunLiveEventWait;
    void                           ThreadLiveEvent_Wait(WORD wMode);

private:    // 指定银行分类特殊处理

signals:    // 窗口处理信号 XFS_CAM_DEC.cpp
    void vSignShowWin(STSHOWWININFO stCamShowInfo);              // 新建并打开窗口
    void vSignHideWin();                                            // 关闭并销毁窗口

public slots:   // 窗口处理 信号槽 XFS_CAM_DEC.cpp
    void vSlotShowWin(STSHOWWININFO stCamShowInfo);              // 显示窗口,对应vSignShowWin信号
    void vSlotHideWin();                                            // 结束显示窗口,对应vSignHideWin信号
};

#endif // XFS_CAM_H

// -------------------------------------- END --------------------------------------
