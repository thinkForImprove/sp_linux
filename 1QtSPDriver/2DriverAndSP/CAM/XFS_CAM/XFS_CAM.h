#pragma once

#ifndef XFS_CAM_H
#define XFS_CAM_H

#include <QWidget>
#include <QMainWindow>

#include "IDevCAM.h"
#include "ISPBaseCAM.h"
#include "QtTypeInclude.h"
#include "ComInfo.h"
#include "libudev.h"
#include "mainwindow.h"

#define DEVTYPE_CHG(a)  a == 0 ? IDEV_TYPE_CW1 : "CAM"


class CXFS_CAM : public QObject, public ICmdFunc, public CLogManage
{
    Q_OBJECT

public:
    //explicit CXFS_CAM(QWidget *parent = nullptr);
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
    virtual HRESULT Display(const WFSCAMDISP &stTakePict, DWORD dwTimeout);
    virtual HRESULT Reset();


protected:
    // 读INI
    void InitConifig();
    // 初始化设备状态结构
    void InitStatus();
    // 更新状态
    bool UpdateStatus(const DEVCAMSTATUS &stStatus);
    // 更新扩展状态，strErrCode错误码的后三位，因前六位是固化的
    void UpdateExtra(string strErrCode, string strDevVer = "");
    // 设备状态是否正常
    bool IsDevStatusOK();
    BOOL bPathCheckAndCreate(LPSTR lpsPath, BOOL bFolder);
    HRESULT hErrCodeChg(LONG lDevCode);
    void vDelImageInfoDir();
    void vDeleteDirectory(LPSTR lpDirName);
    void SharedMemRelease();    // 销毁摄像共享内存
    BOOL bCreateSharedMemory(LPSTR lpMemName, ULONG ulSize); // 创建摄像共享内存
    INT SearchVideoIdxFromVidPid(LPSTR lpVid, LPSTR lpPid);
    HRESULT StartOpen();                                            // 30-00-00-00(FT#0031)
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
    CQtDLLLoader<IDevCAM>                   m_pDev;

    CAutoEvent                              m_cCancelEvent;
    CXfsRegValue                            m_cXfsReg;
    std::string                             m_strLogicalName;
    std::string                             m_strSPName;
    CExtraInforHelper                       m_cExtra;
    CSimpleMutex                           *m_pMutexGetStatus;

    BOOL                                    bDisplyOK;
    BOOL                                    m_bIsOpenOk;            // 30-00-00-00(FT#0031)
    WORD                                    m_wVRTCount;

    MainWindow                              *showWin;   // 摄像窗口
    QSharedMemory                           *m_qSharedMemData;// 摄像数据共享内存

signals:    // 信号
    void vSignShowWin(CAM_SHOWWIN_INFO stCamShowInfo);    // 新建并打开窗口
    void vSignHideWin();    // 关闭并销毁窗口

public slots:   // 信号槽
    void vSlotShowWin(CAM_SHOWWIN_INFO stCamShowInfo);
    void vSlotHideWin();
};

#endif // XFS_CAM_H
