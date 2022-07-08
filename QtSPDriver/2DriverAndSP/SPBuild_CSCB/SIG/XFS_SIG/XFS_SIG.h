#pragma once

#ifndef XFS_CAM_H
#define XFS_CAM_H

#include <QWidget>
#include <QMainWindow>
#include <QFileInfo>
#include <QTextStream>
#include <QDateTime>
#include "IDevSIG.h"
#include "ISPBaseSIG.h"
#include "QtTypeInclude.h"
#include "mainwindow.h"

class CXFS_SIG : public QObject, public ICmdFunc, public CLogManage
{
    Q_OBJECT

public:
    //explicit CXFS_SIG(QWidget *parent = nullptr);
    CXFS_SIG();
    ~CXFS_SIG();

public:
    // 开始运行SP
    HRESULT StartRun();

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
    virtual HRESULT Display(const WFSCAMDISP &stDisply, DWORD dwTimeout);
    virtual HRESULT GetSignature(const WFSCAMGETSIGNATURE &stSignature, WFSCAMSIGNDATA &stSignData, DWORD dwTimeout);
    virtual HRESULT Reset();

protected:
    // 读INI
    void InitConifig();
    // 初始化设备状态结构
    void InitStatus();
    // 更新状态
    bool UpdateStatus(const DEVCAMSTATUS &stStatus);
    // 设置颜色(默认黑色(0,0,0))
    COLORREF cSetColor(WORD r = 0, WORD g = 0, WORD b = 0);
    // 设置16位颜色
    COLORREF bSet16BitColor(DWORD col);
    // 删除过期图片
    void vDelImageInfoFile();
    // 读取图片数据
    bool bReadImageData(LPSTR lpImgPath, LPBYTE lpImgData);
    // 保存数据到图片
    int nSaveDataToImage(LPSTR pData, LPCSTR filename, int nLen);
    // 写入图片路径到文件(mode=1追加, mode=2覆盖)
    int nWriteData(QString filename, LPCSTR pData, int mode);
    // 获取图片创建时间
    void vGetImageCreateTime(QString filename, DWORD *time);
    // 获取文件数据总行数
    int nGetLine(QString filname, QString &strall);
    // 获取文件某一行数据
    QString qGetLineData(int nNum, int nAllLine, QString &strall);

    void FireNoSignatureData();
    void FireInvalidData();
private:
    SIGCONFIGINFO                           m_sSigIniConfig;        // 存储配置项
    SIGHOWWININFO                           m_stCamInfo;            // 存储窗口信息
    WFSCAMSTATUS                            m_stStatus;
    WFSCAMCAPS                              m_stCaps;
    WFSCAMDISP                              m_stDisplay;

    LPBYTE                                  m_pszData;
    ULONG                                   m_iLength;
    CHAR                                    szFileNameFromAp[MAX_PATH];
    CHAR                                    m_szLogType[MAX_PATH];
    CQtDLLLoader<ISPBaseSIG>                m_pBase;
    CQtDLLLoader<IDevSIG>                   m_pDev;
    char                                    m_szDevType[MAX_EXT];           // 设备型号

    CAutoEvent                              m_cCancelEvent;
    CXfsRegValue                            m_cXfsReg;
    std::string                             m_strLogicalName;
    std::string                             m_strSPName;
    CExtraInforHelper                       m_cExtra;
    CSimpleMutex                           *m_pMutexGetStatus;

    MainWindow                              *showWin;   // 签名窗口

signals:    // 信号
    void vSignShowWin(SIGHOWWININFO stSigShowInfo);    // 新建并打开窗口
    void vSignHideWin();    // 关闭并销毁窗口

public slots:   // 信号槽
    void vSlotShowWin(SIGHOWWININFO stSigShowInfo);
    void vSlotHideWin();
};

#endif // XFS_CAM_H
