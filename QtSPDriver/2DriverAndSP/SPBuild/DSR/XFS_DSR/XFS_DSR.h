/***************************************************************
* 文件名称：XFS_DSR.h
* 文件描述：文档扫描模块命令处理接口 头文件
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


// DSR SP 版本号
//const BYTE byVRTU[17] = {"HWDSRSTE01000100"};

// 事件日志
static const char *ThisFile = "XFS_DSR.cpp";

#define LOG_NAME     "XFS_DSR.log"

/*************************************************************************
// Log输出　宏定义
*************************************************************************/
#define IDS_ERR_ON_EXECUTE_ERR              "调用[%s]失败[%s]"
#define IDS_ERR_FORM_INVALID                "FORM无效[%s]"
#define IDS_ERR_MEDIA_INVALID               "MEDIA无效[%s]"
#define IDS_ERR_FORM_NOT_FOUND              "FORM[%s]没有找到"
#define IDS_ERR_MEDIA_NOT_FOUND             "MEDIA[%s]没有找到"
#define IDS_ERR_FILED_NOT_FOUND             "FIELD[%s]没有找到"
#define IDS_ERR_MEDIA_OVERFLOW              "打印FORM[%s]到MEDIA[%s]上溢出"
#define IDS_ERR_MEDIA_OVERFLOW_PRINTAREA    "打印FORM[%s]到MEDIA[%s]上溢出打印区域"
#define IDS_ERR_FIELD_ERROR                 "字段域[%s]错误"
#define IDS_ERR_START_FORM                  "调用StartForm[%s]失败"
#define IDS_ERR_REQUIRED_FIELD              "打印FORM[%s]的字段[%s]没有提供数据"
#define IDS_ERR_PRINT_FIELD                 "打印FORM[%s]字段[%s]失败[%s]"
#define IDS_ERR_NO_CHARSET                  "系统缺少中文字符集"

#define IDS_INFO_CUTPAPER_SUCCESS   "凭条打印机切纸成功"
#define IDS_INFO_RESET_DEVICE       "开始修复打印机"
#define IDS_INFO_RESET_INFO         "打印机复位成功,无纸,返回值=[%d] 传入参数为：[%u]"
#define IDS_INFO_RESET_SUCCESS      "打印机复位成功,返回值=[%d] 传入参数为：[%u]"
#define IDS_INFO_STARTUP_INFO       "Printer StartUp, DeviceName=[%s], DeviceDLLName=[%s]"
#define IDS_INFO_PAPER_TAKEN        "纸被取走"

#define IDS_ERR_REESET_ERROR        "打印机复位错误, 返回值=[%d]"
#define IDS_ERR_RESET_AND_CUTPAPER  "打印机复位ControlMedia错误,返回值=[%d]"
#define IDS_ERR_JPR_UNSUPP_COMMAND  "流水打印机不支持此指令[%u]"
#define IDS_ERR_RPR_UNSUPP_COMMAND  "凭条打印机不支持此指令[%u]"
#define IDS_ERR_NO_PAPER_WHENCUT    "切纸时发现无纸"
#define IDS_ERR_PAPER_JAMMED        "打印机卡纸"
#define IDS_ERR_CUTPAPER_ERROR      "切纸错误(检测黑标:[%d],进纸:[%d])"
#define IDS_ERR_DEVIVE_STA          "当前设备状态不为ONLINE，当前设备状态为：[%d]"
#define IDS_ERR_PRINTSTRING_FAILD   "PrintString失败:[%d]"
#define IDS_ERR_PRINTIMAGE_FAILD    "打印图片错误:[%d]"
#define IDS_ERR_PRINT_FAILD         "打印错误:[%d]"
#define IDS_ERR_INIT_ERROR          "初始化设备错误:[%d]"
#define IDS_ERR_COMPORT_ERROR       "串口参数不正确"
#define IDS_ERR_LOADDLL_FAILD       "LoadLibrary[%s]失败:Error()=[%d]"
#define IDS_ERR_GetProcAdd_FAILD    "GetProcAddress[%s], CreatePrinterDevice)失败"
#define IDS_ERR_CreateDev_FAILD     "调用[%s]的CreatePrinterDevice[%s]失败"
#define IDS_ERR_Open_FAIlD          "调用[%s]的Open()失败:%[d]"
#define IDS_ERR_NO_CHARSET          "系统缺少中文字符集"

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
    (n == DEV_BSD216 ? IDEV_BSD216 : \
     ("UNK_DEV"))

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
    int                     nReportHWErrEvent;          // 是否上报SYSE_HWERR事件
    int                     nPrintCalcelSup;            // 是否支持打印中取消
    char                    szSDKPath[256];             // 设备SDK库路径
    int                     nOpenFailRet;               // Open失败时返回值
    STDEVOPENMODE           stDevOpenMode;              // 设备打开模式
    int                     nStaggerMode;               // 介质上边界留白高度单位(0:行列值,1:毫米,2:0.1毫米,缺省0)
    int                     nFieldIdxStart;             // Field下标起始值
    int                     nFieldCPIMode;              // Field CPI(字符宽)单位(0:缺省,1:毫米,2:0.1毫米,缺省0)
    int                     nFieldLPIMode;              // Field LPI(行高)单位(0:缺省,1:毫米,2:0.1毫米,缺省0)
    char                    szReadTrackName[3][64];     // 读TrackName

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
class CXFS_DSR : public ICmdFunc, public CLogManage, public ConvertVar
{

public:
    CXFS_DSR();
    virtual~ CXFS_DSR();

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
    virtual HRESULT PrintForm(const LPWFSPTRPRINTFORM lDSRintForm, DWORD dwTimeOut);
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
    HRESULT InnerReadImage(LPWFSPTRIMAGEREQUEST lpInData, LPWFSPTRIMAGE *&lppOutData, DWORD dwTimeOut);
    HRESULT InnerReset(DWORD dwMediaControl, USHORT usBinIndex);
    HRESULT InnerMediaControl(DWORD dwControl);

protected:
    // 重载CSPBaseClass的方法
    HRESULT OnInit();
    HRESULT OnExit();

protected:
    bool LoadDevDll(LPCSTR ThisModule);
    void InitConifig();
    long InitStatus();
    long InitCaps();
    long ConvertErrCode(long lRes);
    void UpdateDeviceStatus();
    HRESULT StartOpen(BOOL bReConn = FALSE);                                // Open子处理
    HRESULT SetInit();                                                      // Open设备后相关功能初始设置
    HRESULT MediaInsertWait(DWORD &dwTimeOut);                              // 等待介质放入
    int MediaInsertWait_Pub(DWORD &dwTimeOut);                              // 等待介质放入通用处理模式
    int RunAutoReset(LONG lErrCode);                                                     // 执行自动复位

protected:
    BOOL                        m_bNeedKeepJammedStatus;
    BOOL                        m_bPaperCutted;
    BOOL                        m_bReset;
    STINICONFIG                 m_stConfig;
    STRETRACTCFG                m_stRetractCfg;             // 回收相关
private:
    CQtDLLLoader<IDevPTR>       m_DSRinter;
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
    BOOL                        m_bCmdRuning;               // 是否命令执行中
    BOOL                        m_bCmdCanceled;             // 取消监听放折/打印动作
    WAIT_TAKEN_FLAG             m_WaitTaken;    
    DWORD                       m_dwRetractCntEvent;        // 记录上一次回收事件的计数
    WORD                        m_wNeedFireEvent[8];        // 需要特殊上报的事件

};

