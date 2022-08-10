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
#include "XFS_CAM_DEC.h"
#include "file_access.h"
#include "opencv2/opencv.hpp"
#include "XfsHelper.h"
#include "mainwindow.h"

/***************************************************************************
// 未分类 宏定义
***************************************************************************/


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
    INT InnerTakePicFrontMothod(WFSCAMTAKEPICTEX stTakePict,
                                STTAKEPICTUREPAR &stTakePar);       // TakePicture命令执行前处理
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
    INT                             m_nRetErrOLD[32];               // 处理错误值保存(0:重新Open/1~30:模式Open)
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
