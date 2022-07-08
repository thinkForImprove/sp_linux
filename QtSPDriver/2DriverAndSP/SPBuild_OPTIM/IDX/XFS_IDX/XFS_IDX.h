/***************************************************************
* 文件名称：XFS_IDX.h
* 文件描述：身份证读卡器模块命令处理接口 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年3月25日
* 文件版本：1.0.0.1
****************************************************************/

#pragma once

#include "IDevIDC.h"
#include "ISPBaseIDC.h"
#include "IDCXfsHelper.h"
#include "QtTypeInclude.h"
#include "def.h"
#include "file_access.h"
#include "data_convertor.h"

#include <unistd.h>


/*************************************************************************
// 宏定义 声明                                                            *
*************************************************************************/
// 状态非ONLINE||BUSY时,返回HWERR
#define DEV_STAT_RET_HWERR(DSTAT) \
    if (DSTAT != WFS_IDC_DEVONLINE && DSTAT != WFS_IDC_DEVBUSY) \
    { \
        Log(ThisModule, __LINE__, "Device Status != ONLINE | BUSY, Return: %d", WFS_ERR_HARDWARE_ERROR); \
        return WFS_ERR_HARDWARE_ERROR; \
    }

// 无卡状态检查
#define CHK_MEDIA_ISHAVE(m) \
    if (m == WFS_IDC_MEDIANOTPRESENT) \
    { \
        Log(ThisModule, __LINE__, "Media Status = %d|MEDIANOTPRESENT, 没有检测到卡, Return: %d", \
            m, WFS_ERR_IDC_NOMEDIA); \
        return WFS_ERR_IDC_NOMEDIA; \
    }

// 卡JAM状态检查
#define CHK_MEDIA_ISJAM(m) \
    if (m == WFS_IDC_MEDIAJAMMED) \
    { \
        Log(ThisModule, __LINE__, "Media Status = %d|MEDIAJAMED, 卡JAM, Return: %d", \
            m, WFS_ERR_IDC_MEDIAJAM); \
        return WFS_ERR_IDC_MEDIAJAM; \
    }

#define SCANIMAGE_SAVE_DEF  ""
#define HEADIMAGE_SAVE_DEF  ""

// 等待用户取卡标志
enum WAIT_TAKEN_FLAG
{
    WTF_NONE = 0,   //不等待
    WTF_TAKEN = 1,  //等待用户取卡
};

/*************************************************************************
// 结构体 声明                                                            *
*************************************************************************/
// ini获取
typedef
struct st_idx_ini_config
{
    CHAR                szDevDllName[256];
    WORD                wDeviceType;                        // 设备类型
    WORD                wResetCardAction;                   // Reset时卡动作
    WORD                wResetFailReturn;                   // Reset失败时返回标准
    WORD                wOpenResetSupp;                     // Open时是否执行Reset动作
    WORD                wRetainSupp;                        // 是否支持回收功能
    WORD                wRetainCardCount;                   // 吞卡计数
    WORD                wRetainThreshold;                   // 回收将满报警阀值
    WORD                wRetainFull;                        // 回收满阀值
    WORD                wEjectMode;                         // 退卡时完全弹出/保持在门口
    WORD                wReadEndRunEject;                   // Readrawdata命令执行完成后是否自动退卡
    WORD                wReadRawDataInParamMode;            // ReadRawData入参模式
    WORD                wBankNo;                            // 银行编号
    INT                 nOpenFailRet;                       // Open失败时返回值
    char                szSDKPath[256];                     // 设备SDK库路径
    WORD                wSDKVersion;                        // 设备SDK库版本
    STIMAGEPARAM        stImageParam;                       // 证件图像参数结构体
    BOOL                wDebugMode;                         // 调试
    st_idx_ini_config()
    {
        clear();
    }
    void clear()
    {
        memset(this, 0x00, sizeof(st_idx_ini_config));
    }
} STINICONFIG, *LPSTINICONFIG;


/*************************************************************************
// 主处理类                                                               *
*************************************************************************/
class CXFS_IDX : public ICmdFunc, public CLogManage, public ConvertVarIDC
{
public:
    CXFS_IDX();
    virtual ~CXFS_IDX();
public:
    // 开始运行SP
    long StartRun();

public:
    // 基本接口
    virtual HRESULT OnOpen(LPCSTR lpLogicalName);
    virtual HRESULT OnClose();
    virtual HRESULT OnStatus();
    virtual HRESULT OnWaitTaken();
    virtual HRESULT OnCancelAsyncRequest();
    virtual HRESULT OnUpdateDevPDL();

    // IDC类型接口
    // 查询命令
    virtual HRESULT GetStatus(LPWFSIDCSTATUS &lpStatus);
    virtual HRESULT GetCapabilities(LPWFSIDCCAPS &lpCaps);
    // 执行命令
    virtual HRESULT EjectCard(DWORD dwTimeOut);
    virtual HRESULT RetainCard(LPWFSIDCRETAINCARD &lpRetainCardData);    
    virtual HRESULT ResetCount();
    virtual HRESULT ReadRawData(LPWORD lpReadData, LPWFSIDCCARDDATA *&lppCardData, DWORD dwTimeOut);
    virtual HRESULT Reset(LPWORD lpResetIn);

private:    // XFS_FIDC_DEC.cpp 子处理相关接口
    HRESULT InnerOpen(BOOL bReConn = FALSE);                        // Open设备及初始化相关子处理
    int InitConfig();                                               // 读INI
    void InitStatus();                                              // 状态初始化
    void InitCaps();                                                // 能力值初始化
    void UpdateExtra();                                             // 更新扩展数据
    HRESULT InnerReadRawData(DWORD dwReadOption, DWORD dwTimeOut);  // 读卡子处理
    HRESULT InnerReset(WORD wAction, BOOL bIsHaveCard);             // 复位子处理
    HRESULT InnerEject();                                           // 退卡子处理
    HRESULT InnerRetainCard();                                      // 吞卡子处理
    long UpdateStatus();                                            // 状态更新子处理
    void SetTrackInfo(WORD wSource, WORD wStatus, ULONG uLen, LPBYTE pData);// ReadRawData回参处理    
    INT SetRetainCardCount(WORD wCount);                            // 设置回收箱计数并记录INI

private:    // XFS_FIDC_FIRE.cpp 消息事件处理相关接口
    void FireHWEvent(DWORD dwHWAct, char *pErr);                    // 硬件故障事件
    void FireStatusChanged(WORD wStatus);                           // 状态变化事件
    void FireCardInserted();                                        // 进卡事件
    void FireMediaRemoved();                                        // 退卡/移走卡事件
    void FireMediaRetained();                                       //　吞卡事件
    void FireRetainBinThreshold(WORD wReBin);                       // 回收相关事件
    void FireMediaDetected(WORD ResetOut);                          // 复位时检测到卡事件
    void FireInvalidTrackData(WORD wStatus, LPSTR pTrackName, LPSTR pTrackData);// 出现无效磁道事件

private:
    CQtDLLLoader<IDevIDC>           m_pDev;
    CQtDLLLoader<ISPBaseIDC>        m_pBase;
    CSimpleMutex                    *m_pMutexGetStatus;

    STINICONFIG                     m_stConfig;
    CXfsRegValue                    m_cXfsReg;
    std::string                     m_strLogicalName;
    std::string                     m_strSPName;
    WAIT_TAKEN_FLAG                 m_enWaitTaken;
    WORD                            m_wRetainThresholdFire;             // 记录当前回收临界值事件标记

private:    // 返回数据
    CWfsIDCStatus                   m_stStatus, m_stStatusOLD;
    WFSIDCCAPS                      m_Caps;
    CWFSIDCCardDataPtrArray         m_CardDatas;
    WFSIDCRETAINCARD                m_stWFSIdcRetainCard;               // 吞卡命令应答
    CExtraInforHelper               m_cExtra;
    INT                             m_nReConErr;                        // 断线重连错误码记录

};


//////////////////////////////////////////////////////////////////////////
