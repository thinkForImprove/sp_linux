/***************************************************************
* 文件名称：XFS_HCAM.h
* 文件描述：摄像头模块命令处理接口 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/
#pragma once

#ifndef XFS_HCAM_H
#define XFS_HCAM_H

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

/*************************************************************************
// 未分类 宏定义
*************************************************************************/
// 状态非ONLINE||BUSY时,返回HWERR
#define DEV_STAT_RET_HWERR(DSTAT) \
    if (DSTAT != WFS_CAM_DEVONLINE && DSTAT != WFS_CAM_DEVBUSY) \
    { \
        Log(ThisModule, __LINE__, "Device Status != ONLINE | BUSY, Return: %d|HWERR.", WFS_ERR_HARDWARE_ERROR); \
        switch(DSTAT) \
        { \
            case WFS_CAM_DEVOFFLINE: SetErrorDetail((LPSTR)EC_ERR_DevOffLine); break; \
            case WFS_CAM_DEVNODEVICE: SetErrorDetail((LPSTR)EC_ERR_DevNotFound); break; \
            case WFS_CAM_DEVHWERROR: SetErrorDetail((LPSTR)EC_ERR_DevHWErr); break; \
            case WFS_CAM_DEVPOWEROFF: SetErrorDetail((LPSTR)EC_ERR_DevPowerOff); break; \
            case WFS_CAM_DEVUSERERROR: SetErrorDetail((LPSTR)EC_ERR_DevUserErr); break; \
            case WFS_CAM_DEVFRAUDATTEMPT: SetErrorDetail((LPSTR)EC_ERR_DevFraud); break; \
        } \
        return WFS_ERR_HARDWARE_ERROR; \
    }

// 摄像窗口运行方式
#define WIN_RUN_SHARED      0x01    // 共享内存方式

/*************************************************************************
// 主处理类
*************************************************************************/
class CXFS_HCAM : public QObject, public ICmdFunc, public CLogManage, public ConvertVarCAM
{
    Q_OBJECT

public:
    CXFS_HCAM();
    ~CXFS_HCAM();

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


private:    // XFS_HCAM_DEC.cpp 子处理相关接口
    HRESULT InnerOpen(BOOL bReConn = FALSE);                        // Open设备及初始化相关子处理
    BOOL LoadDevHCAMDll(LPCSTR ThisModule);                         // 加载DevXXX动态库
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
    INT SetErrorDetail(LPSTR lpCode = nullptr);                     // 设置ErrorDetail错误码

private:    // XFS_HCAM_FIRE.cpp 事件消息子处理相关接口
    void FireHWEvent(DWORD dwHWAct, char *pErr);
    void FireStatusChanged(WORD wStatus);

private:
    CXfsRegValue                    m_cXfsReg;
    std::string                     m_strLogicalName;
    std::string                     m_strSPName;
    CSimpleMutex                    *m_pMutexGetStatus;
    CQtDLLLoader<ISPBaseCAM>        m_pBase;
    STINICONFIG                     m_stConfig;                     // INI结构体
    CQtDLLLoader<IDevCAM>           m_pCAMDev;                      // DevXXX调用实例
    CWfsCAMStatus                   m_stStatus;                     // 状态结构体
    CWfsCAMStatus                   m_stStatusOLD;                  // 备份上一次状态结构体
    CWfsCAMCap                      m_stCaps;                       // 能力值结构体
    CExtraInforHelper               m_cStatExtra;                   // 状态扩展数据
    CExtraInforHelper               m_cCapsExtra;                   // 能力值扩展数据
    INT                             m_nRetErrOLD[12];               // 处理错误值保存(0:HCAM断线重连)
    CHAR                            m_szFileName[MAX_PATH];         // 拍照文件名
    MainWindow                      *showWin;                       // 摄像窗口
    QSharedMemory                   *m_qSharedMemData;              // 摄像数据共享内存
    BOOL                            m_bCmdRunning;                  // 是否命令执行中
    BOOL                            m_bDisplayOK;                   // Display是否已执行
    CErrorDetail                    m_clErrorDet;                   // ErrorDetail处理类实例
    WORD                            m_wWindowsRunMode;              // 窗口运行方式(0:共享内存)


private:    // 指定银行分类特殊处理

signals:    // 窗口处理信号 XFS_CAM_DEC.cpp
    void vSignShowWin(STSHOWWININFO stCamShowInfo);              // 新建并打开窗口
    void vSignHideWin();                                            // 关闭并销毁窗口

public slots:   // 窗口处理 信号槽 XFS_CAM_DEC.cpp
    void vSlotShowWin(STSHOWWININFO stCamShowInfo);              // 显示窗口,对应vSignShowWin信号
    void vSlotHideWin();                                            // 结束显示窗口,对应vSignHideWin信号
};

#endif // XFS_HCAM_H
