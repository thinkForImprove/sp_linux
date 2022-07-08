/***************************************************************
* 文件名称：XFS_PPR.h
* 文件描述：存折打印模块命令处理接口 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年11月6日
* 文件版本：1.0.0.1
****************************************************************/

#pragma once


#include "string.h"
#include "QtTypeInclude.h"
#include <QTextCodec>
#include <stdlib.h>
#include <unistd.h>

#include "ISPBasePTR.h"
#include "IDevPTR.h"
#include "QtTypeDef.h"

#include "PTRFORM/TextPrinter.h"
#include "PTRFORM/PTRData.h"
#include "PTRFORM/PTRForm.h"


#include "def.h"


// 事件日志
static const char *ThisFile = "XFS_PPR.cpp";

#define LOG_NAME     "XFS_PPR.log"

/*************************************************************************
// Taken标记 定义
*************************************************************************/
enum WAIT_TAKEN_FLAG
{
    WTF_NONE = 0,   //不等待
    WTF_TAKEN = 1,  //等待用户取纸
};

/*************************************************************************
// 未分类 宏定义
*************************************************************************/
// INI中指定[int型]设备类型 转换为 STR型
#define DEVTYPE2STR(n) \
    (n == DEV_MB2 ? IDEV_MB2 : \
     (n == DEV_PRM ? IDEV_PRM : ""))

// 状态非ONLINE||BUSY时,返回HWERR
#define DEV_STAT_RET_HWERR(DSTAT) \
    if (DSTAT != WFS_PTR_DEVONLINE && DSTAT != WFS_PTR_DEVBUSY) \
        return WFS_ERR_HARDWARE_ERROR;

// 缺纸检查
#define PAPER_OUT_RET(PAPER) \
    if (PAPER == WFS_PTR_PAPEROUT) \
        return WFS_ERR_PTR_PAPEROUT;

// 介质放入等待处理 宏定义
#define MEDINS_CANCEL       0   // 取消
#define MEDINS_OK           1   // 介质放入
#define MEDINS_JAM          2   // 出现JAM
#define MEDINS_TIMEOUT      3   // 放入超时
#define MEDINS_OFFLINE      4   // 断线
#define MEDINS_HWERR        5   // 故障

/*************************************************************************
// 回收结构体 定义
*************************************************************************/
typedef struct st_Retract_config
{
    WORD    wRetBoxCount;       // 回收箱数目
    WORD    nRetractSup;        // 回收支持
    DWORD   dwRetractVol;       // 回收满数目
    DWORD   dwFullThreshold;    // 回收满阀值
    DWORD   dwRetractCnt;       // 回收计数
    WORD    wRetractStat;       // 回收状态(0正常/1将满/2满)
    st_Retract_config()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_Retract_config));
        wRetBoxCount = 1;
    }
} STRETRACTCFG, *LPSTRETRACTCFG;

/*************************************************************************
// INI配置结构体 定义
*************************************************************************/
typedef struct st_Ini_Config
{
    char                    szDevDllName[256];
    int                     nDeviceType;
    bool                    bDetectBlackStripe;
    int                     nVerifyField;
    int                     nPageSize;
    int                     nPageLine;
    bool                    bEnableSplit;
    int                     nLineSize;
    int                     nRawDataInPar;
    int                     nAutoResetErrList[64];      //
    int                     nAutoResetErrListCnt;       //
    int                     nReportHWErrEvent;         // 是否上报SYSE_HWERR事件
    int                     nPrintCalcelSup;            // 是否支持打印中取消
    char                    szSDKPath[256];             // 设备SDK库路径
    int                     nOpenFailRet;               // Open失败时返回值
    STDEVOPENMODE           stDevOpenMode;              // 设备打开模式
    int                     nStaggerMode;               // 介质上边界留白高度单位(0:行列值,1:毫米,2:0.1毫米,缺省0)
    int                     nFieldIdxStart;             // Field下标起始值
    int                     nFieldCPIMode;              // Field CPI(字符宽)单位(0:缺省,1:毫米,2:0.1毫米,缺省0)
    int                     nFieldLPIMode;              // Field LPI(行高)单位(0:缺省,1:毫米,2:0.1毫米,缺省0)
    char                    szReadTrackName[3][64];     // 读TrackName
    STINICONFIG_MB2         stConfig_MB2;               // MB2打印机特殊配置
    STINICONFIG_PRM         stConfig_PRM;               // PRM打印机特殊配置
    STCONFIGBEEP            stConfig_Beep;              // 设备鸣响设置
    st_Ini_Config()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_Ini_Config));
    }
}STINICONFIG, *LPSTINICONFIG;

/*************************************************************************
// 主处理类
*************************************************************************/
class CXFS_PPR : public ICmdFunc, public CTextPrinter, public ConvertVar
{

public:
    CXFS_PPR();
    virtual~ CXFS_PPR();

public:
    // 开始运行SP
    long StartRun();

    // 基本接口
    virtual HRESULT OnOpen(LPCSTR lpLogicalName);
    virtual HRESULT OnClose();
    virtual HRESULT OnStatus();
    virtual HRESULT OnWaitTaken();
    virtual HRESULT OnCancelAsyncRequest();
    virtual HRESULT OnUpdateDevPDL();

    // PTR类型接口
    // INFOR
    virtual HRESULT GetStatus(LPWFSPTRSTATUS &lpStatus);
    virtual HRESULT GetCapabilities(LPWFSPTRCAPS &lpCaps);
    virtual HRESULT GetFormList(LPSTR &lpszFormList);
    virtual HRESULT GetMediaList(LPSTR &lpszMediaList);
    virtual HRESULT GetQueryForm(LPCSTR lpFormName, LPWFSFRMHEADER &lpFrmHeader);
    virtual HRESULT GetQueryMeida(LPCSTR lpMediaName, LPWFSFRMMEDIA &lpFrmMedia) ;
    virtual HRESULT GetQueryField(const LPWFSPTRQUERYFIELD lpQueryField, LPWFSFRMFIELD *&lpszMediaList);
    // EXECUTE
    virtual HRESULT MediaControl(const LPDWORD lpdwMeidaControl);
    virtual HRESULT PrintForm(const LPWFSPTRPRINTFORM lpPrintForm, DWORD dwTimeOut);
    virtual HRESULT ReadForm(const LPWFSPTRREADFORM lpReadForm, LPWFSPTRREADFORMOUT &lpReadFormOut, DWORD dwTimeOut);
    virtual HRESULT RawData(const LPWFSPTRRAWDATA lpRawData, LPWFSPTRRAWDATAIN &lpRawDataIn, DWORD dwTimeOut);
    virtual HRESULT MediaExtents(const LPWFSPTRMEDIAUNIT lpMediaUnit, LPWFSPTRMEDIAEXT &lpMediaExt);
    virtual HRESULT ResetCount(const LPUSHORT lpusBinNum);
    virtual HRESULT ReadImage(const LPWFSPTRIMAGEREQUEST lpImgRequest, LPWFSPTRIMAGE *&lppImage, DWORD dwTimeOut);
    virtual HRESULT Reset(const LPWFSPTRRESET lpReset);
    virtual HRESULT RetractMedia(const LPUSHORT lpusBinNum, LPUSHORT &lpusBinNumOut);
    virtual HRESULT DispensePaper(const LPWORD lpPaperSource);

public:
    // Fire消息
    void FireHWEvent(DWORD dwHWAct, char *pErr);
    void FireStatusChanged(WORD wStatus);
    void FireNoMedia(LPCSTR szPrompt);
    void FireMediaInserted();
    void FireFieldError(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure);
    void FireFieldWarning(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure);
    void FireRetractBinThreshold(USHORT BinNumber, WORD wStatus);
    void FireMediaTaken();
    void FirePaperThreshold(WORD wSrc, WORD wStatus);
    void FireTonerThreshold(WORD wStatus);
    void FireInkThreshold(WORD wStatus);
    void FireLampThreshold(WORD wStatus);
    void FireSRVMediaInserted();
    void FireMediaDetected(WORD wPos, USHORT BinNumber);

private:
    HRESULT InnerPrintForm(LPWFSPTRPRINTFORM pInData, DWORD dwTimeOut);
    HRESULT InnerReadForm(LPWFSPTRREADFORM pInData, DWORD dwTimeOut);
    HRESULT InnerReadImage(LPWFSPTRIMAGEREQUEST lpInData, LPWFSPTRIMAGE *&lppOutData, DWORD dwTimeOut);
    HRESULT InnerReadRawData(BOOL bExpectResp, ULONG nSize, LPBYTE pData, DWORD dwTimeOut);
    HRESULT InnerReset(DWORD dwMediaControl, USHORT usBinIndex);
    HRESULT InnerMediaControl(DWORD dwControl);

protected:
    // 重载CSPBaseClass的方法
    HRESULT OnInit();
    HRESULT OnExit();

    // 子类继承实现的函数,PrintForm最后处理
    // 新增打印机可在在该接口中根据设备类型进行分类处理
    virtual HRESULT EndForm(PrintContext *pContext);                        // 打印处理主分支
    HRESULT EndForm_MB2(DEVPRINTFORMIN stIn, DEVPRINTFORMOUT &stOut);       // 打印处理子分支: MB2存折打印机处理
    HRESULT EndForm_PRM(DEVPRINTFORMIN stIn, DEVPRINTFORMOUT &stOut);       // 打印处理子分支: PRM存折打印机处理
    virtual HRESULT EndReadForm(ReadContext *pContext);                     // 读处理主分支
    HRESULT EndReadForm_Pub(ReadContext *pContext);                         // 读处理子分支(共通): PRM/MB2存折打印机处理

protected:
    bool LoadDevDll(LPCSTR ThisModule);
    void InitConifig();
    HRESULT InitStatus();
    HRESULT InitCaps();
    void RemoveUnPrintableChar(ULONG ulInSize, const LPBYTE pInData, ULONG &ulOutSize, LPBYTE pOutData);
    void UpdateDeviceStatus();
    HRESULT StartOpen(BOOL bReConn = FALSE);                                // Open子处理
    HRESULT SetInit();                                                      // Open设备后相关功能初始设置
    HRESULT MediaInsertWait(DWORD &dwTimeOut);                              // 等待介质放入
    INT MediaInsertWait_Pub(DWORD &dwTimeOut);                              // 等待介质放入通用处理模式
    INT RunAutoReset(LONG lErrCode);                                        // 执行自动复位

protected:
    BOOL                        m_bNeedKeepJammedStatus;
    BOOL                        m_bPaperCutted;
    BOOL                        m_bReset;
    STINICONFIG                 m_stConfig;
    STRETRACTCFG                m_stRetractCfg;                             // 回收相关
private:
    CQtDLLLoader<IDevPTR>       m_pPrinter;
    CQtDLLLoader<ISPBasePTR>    m_pBase;
    CXfsRegValue                m_cXfsReg;
    std::string                 m_strLogicalName;
    std::string                 m_strSPName;
    CSimpleMutex                *m_pMutexGetStatus;
protected:
    CExtraInforHelper           m_cExtra;
    CWfsPtrStatus               m_stStatus, m_stStatusOLD;
    CWfsPtrCaps                 m_sCaps;

private:
    INT                         m_nReConErr;
    BOOL                        m_bCmdRuning;                               // 是否命令执行中
    BOOL                        m_bCmdCanceled;                             // 取消监听放折/打印动作
    WAIT_TAKEN_FLAG             m_WaitTaken;    
    DWORD                       m_dwRetractCntEvent;                        // 记录上一次回收事件的计数
    WORD                        m_wNeedFireEvent[8];                        // 需要特殊上报的事件

};

