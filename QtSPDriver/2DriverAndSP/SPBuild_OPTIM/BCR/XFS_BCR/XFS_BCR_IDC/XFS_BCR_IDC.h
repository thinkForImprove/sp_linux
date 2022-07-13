/***************************************************************************
* 文件名称: XFS_BCR_IDC.h
* 文件描述: 条码阅读模块命令处理接口(IDC命令系) 头文件
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年7月13日
* 文件版本: 1.0.0.1
***************************************************************************/
#pragma once

#include "IDevBCR.h"
#include "ISPBaseIDC.h"
#include "QtTypeInclude.h"
#include "IDCFORM/IDCForm.h"
#include "ErrorDetail.h"
#include "def.h"

#include <unistd.h>
#include <queue>
#include <qthread.h>

/***************************************************************************
// 宏定义
***************************************************************************/
// INI中指定[int型]设备类型 转换为 STR型
#define DEVTYPE2STR(n) \
    (n == XFS_NT0861 ? IDEV_NT0861_STR : "")

// 状态非ONLINE||BUSY时,返回HWERR
#define DEV_STAT_RET_HWERR(DSTAT) \
    if (DSTAT != WFS_BCR_DEVONLINE && DSTAT != WFS_BCR_DEVBUSY) \
    { \
        Log(ThisModule, __LINE__, "Device Status != ONLINE | BUSY, Return: %d.", WFS_ERR_HARDWARE_ERROR); \
        switch(DSTAT) \
        { \
            case WFS_BCR_DEVOFFLINE: SetErrorDetail((LPSTR)EC_XFS_DevOffLine); break; \
            case WFS_BCR_DEVNODEVICE: SetErrorDetail((LPSTR)EC_XFS_DevNotFound); break; \
            case WFS_BCR_DEVHWERROR: SetErrorDetail((LPSTR)EC_XFS_DevHWErr); break; \
            case WFS_BCR_DEVPOWEROFF: SetErrorDetail((LPSTR)EC_XFS_DevPowerOff); break; \
            case WFS_BCR_DEVUSERERROR: SetErrorDetail((LPSTR)EC_XFS_DevUserErr); break; \
            case WFS_BCR_DEVFRAUDATTEMPT: SetErrorDetail((LPSTR)EC_XFS_DevFraud); break; \
        } \
        return WFS_ERR_HARDWARE_ERROR; \
    }

// 等待用户取卡标志 定义
enum WAIT_TAKEN_FLAG
{
    WTF_NONE            = 0,    // 不等待
    WTF_TAKEN           = 1,    // 等待用户取卡
};


/***************************************************************************
// 主处理类                                                                 *
***************************************************************************/
class CXFS_BCR : public ICmdFunc, public CLogManage, public CIDCForm,
                 public ConvertVarBCR
{
public:
    CXFS_BCR();
    virtual ~CXFS_BCR();
public:
    // 开始运行SP
    long StartRun();

public: // 基本接口
    virtual HRESULT OnOpen(LPCSTR lpLogicalName);
    virtual HRESULT OnClose();
    virtual HRESULT OnStatus();
    virtual HRESULT OnWaitTaken();
    virtual HRESULT OnCancelAsyncRequest();
    virtual HRESULT OnUpdateDevPDL();

public: // IDC类型接口(用于条码阅读)
    // 查询命令
    virtual HRESULT GetStatus(LPWFSIDCSTATUS &lpStatus);
    virtual HRESULT GetCapabilities(LPWFSIDCCAPS &lpCaps);
    // 执行命令
    virtual HRESULT ReadRawData(LPWORD lpReadData, LPWFSIDCCARDDATA *&lppCardData, DWORD dwTimeOut);
    virtual HRESULT Reset(LPWORD lpResetIn);

protected:  // XFS_BCR_IDC_DEC.cpp 子处理相关接口
    HRESULT InnerOpen(BOOL bReConn = FALSE);                        // Open设备及初始化相关子处理
    BOOL LoadDevBCRDll(LPCSTR ThisModule);                          // 加载DevXXX动态库
    INT InitConfig();                                               // 加载INI设置    
    INT PrintIniBCR();                                              // INI配置输出
    void InitStatus();                                              // 状态结构体实例初始化
    void InitCaps();                                                // 能力值结构体实例初始化
    void UpdateExtra();                                             // 更新扩展数据
    WORD UpdateDeviceStatus();                                      // 设备状态实时更新
    HRESULT InnerAcceptAndReadTrack(DWORD dwReadOption, DWORD dwTimeOut);  // 读卡子处理(扫码)
    HRESULT InnerReset(WORD wAction);                               // 复位子处理

private:    // XFS_BCR_IDC_DEC.cpp 数据处理相关接口
    void SetTrackInfo(WORD wSource, WORD wStatus, ULONG uLen, LPBYTE pData);  // 读卡应答数据处理
    bool GetTrackInfo(WORD wSource, ULONG *pLen, LPBYTE pData, WORD *pWriteMetho);// 读卡应答数据处理
    INT SetErrorDetail(LPSTR lpCode = nullptr);                     // 设置ErrorDetail错误码

private:    // XFS_BCR_IDC_FIRE.cpp 事件消息子处理相关接口
    void FireHWEvent(DWORD dwHWAct, char *pErr);
    void FireStatusChanged(WORD wStatus);
    void FireCardInserted();
    void FireMediaRemoved();
    void FireMediaRetained();
    void FireMediaDetected(WORD ResetOut);
    void FireRetainBinThreshold(WORD wReBin);
    void FireInvalidTrackData(WORD wStatus, LPSTR pTrackName, LPSTR pTrackData);

private:    // 变量定义
    STINICONFIG                     m_stConfig;                     // INI结构体
    CQtDLLLoader<IDevBCR>           m_pBCRDev;                      // DevXXX调用实例
    CQtDLLLoader<ISPBaseIDC>        m_pBase;
    CSimpleMutex                    *m_pMutexGetStatus;
    CXfsRegValue                    m_cXfsReg;
    INT                             m_nRetErrOLD[8];                // 处理错误值保存(0:断线重连)
    WAIT_TAKEN_FLAG                 m_enWaitTaken;                  // Taken标记
    CExtraInforHelper               m_cStatExtra;                   // 状态扩展数据
    CExtraInforHelper               m_cCapsExtra;                   // 能力值扩展数据
    CWfsIDCStatus                   m_stStatus;                     // 全局状态
    CWfsIDCStatus                   m_stStatusOLD;                  // 上一次备份全局状态
    CErrorDetail                    m_clErrorDet;                   // ErrorDetail处理类实例

private:    // 命令应答数据变量
    CMultiMultiString               m_TrackData;                    // 磁道数据
    CWFSIDCCardDataPtrArray         m_CardDatas;                    // 封装的LPLFSIDCCARDDATA数组
    WFSIDCCAPS                      m_stCaps;                       // 能力值结构体
};

// -------------------------------- END -----------------------------------

