/***************************************************************
* 文件名称：XFS_CAM.h
* 文件描述：摄像头模块命令处理接口 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/

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
#include "ComInfo.h"
#include "file_access.h"
#include "opencv2/opencv.hpp"

#include "mainwindow.h"

/*************************************************************************
// 未分类 宏定义
*************************************************************************/
// TakePictureEx.wCamera: 长沙银行需求: 资料补拍
#define WFS_CAM_FILE    2

// INI中指定[int型]设备类型 转换为 STR型(用于加载DevCAM动态库使用)
#define DEVTYPE2STR(n) \
    (n == CAM_DEV_CLOUDWALK ? IDEV_YC0C98 : \
     (n == CAM_DEV_TCF261 ? IDEV_TCF261 : \
      (n == CAM_DEV_ZLF1000A3 ? IDEV_ZLF1000A3 : "")))

/*************************************************************************
// 主处理类
*************************************************************************/
class CXFS_CAM : public QObject, public ICmdFunc, public CLogManage
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

    // cam类型接口
    virtual HRESULT GetStatus(LPWFSCAMSTATUS &lpStatus);
    virtual HRESULT GetCapabilities(LPWFSCAMCAPS &lpCaps);
    virtual HRESULT TakePicture(const WFSCAMTAKEPICT &stTakePict, DWORD dwTimeout);
    virtual HRESULT TakePictureEx(const WFSCAMTAKEPICTEX &stTakePict, DWORD dwTimeout);
    virtual HRESULT Display(const WFSCAMDISP &stDisplay, DWORD dwTimeout);
    virtual HRESULT DisplayEx(const WFSCAMDISPEX &stDisplayEx, DWORD dwTimeout);
    virtual HRESULT GetSignature(const WFSCAMGETSIGNATURE &stGetSig, WFSCAMSIGNDATA &stSignData, DWORD dwTimeout)
    {
        return WFS_ERR_UNSUPP_COMMAND;
    }
    virtual HRESULT Reset();

public:
    // Fire消息
    void FireEXEE_LIVEERROR_Event(WORD wError);


private: // XFS_CAM_DEC.cpp
    HRESULT StartOpen();                                            // 30-00-00-00(FT#0031)
    bool UpdateStatus(const DEVCAMSTATUS &stStatus);                // 更新状态
    HRESULT InnerDisplay(const WFSCAMDISP &stDisplay, DWORD dwTimeout);
    HRESULT InnerDisplayEx(const WFSCAMDISPEX &stDisplayEx, DWORD dwTimeout);
    HRESULT InnerTakePicture(const WFSCAMTAKEPICT &stTakePict, DWORD dwTimeout);
    HRESULT InnerTakePictureEx(const WFSCAMTAKEPICTEX &stTakePict, DWORD dwTimeout);
    BOOL bCreateSharedMemory(LPSTR lpMemName, ULONG ulSize);        // 创建摄像共享内存
    void SharedMemRelease();                                        // 销毁摄像共享内存
    bool IsDevStatusOK();                                           // 设备状态是否正常
    void InitConifig();                                             // 读INI
    void InitStatus();                                              // 初始化设备状态结构
    HRESULT hErrCodeChg(LONG lDevCode);
    WORD ConvertLiveErr2WFS(WORD wError);
    BOOL bPathCheckAndCreate(LPSTR lpsPath, BOOL bFolder);
    void vDelImageInfoDir();
    void vDeleteDirectory(LPSTR lpDirName);
    INT  InnerTakePicAfterMothod();                                 // TakePicture命令完成后处理

private:
    ST_CAM_INI_CONFIG                       m_sCamIniConfig;
    ST_CAM_DEV_INIT_PARAM                   m_stDevInitParam;
    WFSCAMSTATUS                            m_stStatus;
    WFSCAMSTATUS                            m_stStatusOld;
    WFSCAMCAPS                              m_stCaps;

    CHAR                                    szFileNameFromAp[MAX_PATH];
    BOOL                                    bImageType;
    char                                    m_szLogType[MAX_PATH];
    CQtDLLLoader<ISPBaseCAM>                m_pBase;
    CQtDLLLoader<IDevCAM>                   m_pDev;                 // 人脸摄像DevCAM接口实例

    CAutoEvent                              m_cCancelEvent;
    CXfsRegValue                            m_cXfsReg;
    std::string                             m_strLogicalName;
    std::string                             m_strSPName;
    CExtraInforHelper                       m_cExtra;
    CSimpleMutex                           *m_pMutexGetStatus;

    BOOL                                    m_bDisplayOK;
    BOOL                                    m_bIsOpenOk;            // 30-00-00-00(FT#0031)

    MainWindow                              *showWin;               // 摄像窗口
    QSharedMemory                           *m_qSharedMemData;      // 摄像数据共享内存

    WORD                                    m_wDeviceType;          // 人脸摄像设备类型(INI指定)
    BOOL                                    m_bCmdRunning;          // 是否命令执行中
    WORD                                    m_wVRTCount;

private:    // 活检图像状态事件监听线程相关
    BOOL                                m_bLiveEventWaitExit;       // 活检状态监听事件处理退出标记
    std::thread                         m_thRunLiveEventWait;
    void                                ThreadLiveEvent_Wait();

private:    // 指定银行分类特殊处理
    INT  nBankMethod_CMB();                                         // 招商银行特殊处理

signals:    // 窗口处理信号 XFS_CAM_DEC.cpp
    void vSignShowWin(CAM_SHOWWIN_INFO stCamShowInfo);              // 新建并打开窗口
    void vSignHideWin();                                            // 关闭并销毁窗口

public slots:   // 窗口处理 信号槽 XFS_CAM_DEC.cpp
    void vSlotShowWin(CAM_SHOWWIN_INFO stCamShowInfo);              // 显示窗口,对应vSignShowWin信号
    void vSlotHideWin();                                            // 结束显示窗口,对应vSignHideWin信号
};

#endif // XFS_CAM_H
