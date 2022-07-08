/***************************************************************
* 文件名称：XFS_JPR.h
* 文件描述：流水打印模块命令处理接口 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年10月14日
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
static const char *ThisFile = "XFS_JPR.cpp";

#define LOG_NAME     "XFS_JPR.log"


/*************************************************************************
// 未分类 宏定义
*************************************************************************/
// INI中指定[int型]设备类型 转换为 STR型
#define DEVTYPE2STR(n) \
    (n == DEV_BTT080AII ? IDEV_BTT080AII : "")

// 状态非ONLINE||BUSY时,返回HWERR
#define DEV_STAT_RET_HWERR(DSTAT) \
    if (DSTAT != WFS_PTR_DEVONLINE && DSTAT != WFS_PTR_DEVBUSY) \
        return WFS_ERR_HARDWARE_ERROR;

// 缺纸检查
#define PAPER_OUT_RET(PAPER) \
    if (PAPER == WFS_PTR_PAPEROUT) \
        return WFS_ERR_PTR_PAPEROUT;

/*************************************************************************
// INI配置结构体 定义
*************************************************************************/
typedef struct st_Ini_Config
{
    char                 szDevDllName[256];
    int                  nDeviceType;
    bool                 bDetectBlackStripe;
    int                  nVerifyField;
    int                  nPageSize;
    int                  nPageLine;
    bool                 bEnableSplit;
    int                  nLineSize;
    int                  nRawDataInPar;
    CHAR                 szSDKPath[256];        // 设备SDK库路径
    INT                  nOpenFailRet;          // Open失败时返回值
    STDEVOPENMODE        stDevOpenMode;         // 设备打开模式
    WORD                wPrintMode;                     // 打印方式
    WORD                wPrtBarcodeMode[5];             // 打印条形码码格式
    WORD                wPrtPDF417Mode[7];              // 打印PDF417码格式
    WORD                wPrtQrcodeMode[4];              // 打印条形码码格式
    CHAR                szBarFontList[24][32];          // Field.FONT属性指定条码关键字列表
    int                 nFieldCPIMode;                  // Field CPI(字符宽)单位(0:缺省,1:毫米,2:0.1毫米,缺省0)
    int                 nFieldLPIMode;                  // Field LPI(行高)单位(0:缺省,1:毫米,2:0.1毫米,缺省0)
    STPRINTMODE         stPrtModeRaw;                   // RawData打印模式
    STPRINTMODE         stPrtModeForm;                  // PrintForm打印模式

    st_Ini_Config()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_Ini_Config));
    }
}STINICONFIG, *LPSTINICONFIG;

// wPrtBarcodeMode[5]说明: [0]指定条码类型,[1]基本元素宽度点数,[2]条码高度,
//                        [3]指定HRI字符的字体类型,[4]指定HRI字符的位置
// wPrtPDF417Mode[7]说明: [0]基本元素宽度点数,[1]元素高度点数,[2]行数,[3]列数,
//                       [4]外观比高度,[5]外观比宽度,[6]纠错级别
// wPrtQrcodeMode[4]说明: [0]基本元素宽度点数,[1]符号类型,[2]语言模式,
//                        [3]纠错级别

/*************************************************************************
// 主处理类
*************************************************************************/
class CXFS_JPR : public ICmdFunc, public CTextPrinter, public ConvertVar
{

public:
    CXFS_JPR();
    virtual~ CXFS_JPR();

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
    HRESULT InnerPrintForm(LPWFSPTRPRINTFORM pInData);
    HRESULT InnerRawData(BOOL bExpectResp, ULONG nSize, LPBYTE pData);
    HRESULT InnerReset(DWORD dwMediaControl, USHORT usBinIndex);

protected:
    // 重载CSPBaseClass的方法
    HRESULT OnInit();
    HRESULT OnExit();

    // 子类继承实现的函数,PrintForm最后处理
    // 新增打印机可在在该接口中根据设备类型进行分类处理
    virtual HRESULT EndForm(PrintContext *pContext);
protected:
    bool LoadDevDll(LPCSTR ThisModule);
    void InitConifig();
    HRESULT InitStatus();
    HRESULT InitCaps();
    HRESULT EndForm_BTT080AII(PrintContext *pContext);      // 新北洋打印机处理
    HRESULT EndForm_Print(PrintContext *pContext);          // 打印机处理2
    // 文本打印前处理
    HRESULT PrintString(const char *pBuffer, DWORD dwSize, BOOL bIsFromPrint = FALSE);
    HRESULT PrintString2(const char *pBuffer, DWORD dwSize, BOOL bIsFromPrint = FALSE);
    // 文本打印前处理(新北洋打印机专用,若新增打印机,可根据新机修改或新增处理接口)
    HRESULT AddPrintString(const char *pBuffer, DWORD dwSize, BOOL bIsFromPrint, char *pBuffOut, DWORD &dwSizeOut);
    // 图片打印(指定宽高,当前为HOST打印机专用)
    HRESULT PrintImage(LPCSTR szImagePath, int nDstWidth, int nDstHeight);
    // 图片打印(传入X,Y坐标,当前为BKC310/NH80M打印机使用)
    HRESULT PrintImageOrg(LPCSTR szImagePath, ULONG ulOrgX, ULONG ulOrgY);
    // 文本打印
    HRESULT PrintData(const char *pBuffer, DWORD dwSize);
    //去掉不可打印字符
    void RemoveUnPrintableChar(ULONG ulInSize, const LPBYTE pInData, ULONG &ulOutSize, LPBYTE pOutData);
    void UpdateDeviceStatus();
    HRESULT StartOpen(BOOL bReConn = FALSE);
    // Open设备后相关功能初始设置
    HRESULT SetInit();
    INT  GetBarFontMode(LPSTR lpData);

protected:
    BOOL                    m_bNeedKeepJammedStatus;
    BOOL                    m_bPaperCutted;
    BOOL                    m_bReset;
    STINICONFIG             m_stConfig;
private:
    CQtDLLLoader<IDevPTR>          m_pPrinter;
    CQtDLLLoader<ISPBasePTR>       m_pBase;
    CXfsRegValue                   m_cXfsReg;
    std::string                    m_strLogicalName;
    std::string                    m_strSPName;
    CSimpleMutex                  *m_pMutexGetStatus;
protected:
    CExtraInforHelper              m_cExtra;
    CWfsPtrStatus                  m_sStatus;
    CWfsPtrCaps                    m_sCaps;

private:
    INT                            m_nReConErr;
    BOOL                           m_bCmdRuning;            // 是否命令执行中
};

