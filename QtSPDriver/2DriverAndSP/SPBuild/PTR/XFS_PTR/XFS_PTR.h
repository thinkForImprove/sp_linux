/***************************************************************
* 文件名称：XFS_PTR.h
* 文件描述：凭条模块命令子处理接口 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2019年6月15日
* 文件版本：1.0.0.1

* 版本历史信息2
* 变更说明：1. Rawdata打印增加自动换行;2.JAM时dev为HWErr
* 票   号：30-00-00-00(FT#0008)
* 变更日期：2019年8月1日
* 文件版本：1.0.0.2
*
* 版本历史信息3
* 变更说明：1. 共通处理FORM相关统合所有PTR系处理(包含RPR/CPR/CSR)
*         2. XFS_PTR处理分为XFS_PTR.cpp和XFS_PTR_DEC.cpp,分别对应主命令处理和命令子处理
*         3. RPR SP 版本号变更(HWPTRSTE01000100),用于区分原RPR处理模式
*         4. 当前支持BK-C310/BT-NH80M，HOTS暂不支持
* 变更日期：2021年8月17日
* 文件版本：1.0.0.2
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
static const char *ThisFile = "XFS_PTR.cpp";

#define LOG_NAME     "XFS_PTR.log"


/*************************************************************************
// 未分类 宏定义
*************************************************************************/
// INI中指定[int型]设备类型 转换为 STR型
#define DEVTYPE2STR(n) \
    (n == DEV_HOTS ? DEV_HOTS_STR : \
      (n == DEV_SNBC_BKC310 ? DEV_SNBC_BKC310_STR : \
        (n == DEV_SNBC_BTNH80 ? DEV_SNBC_BTNH80_STR : DEV_SNBC_BTNH80_STR)))

/*************************************************************************
// Taken标记 定义
*************************************************************************/
enum WAIT_TAKEN_FLAG
{
    WTF_NONE = 0,   //不等待
    WTF_TAKEN = 1,  //等待用户取纸
};

/*************************************************************************
// 设备类型 定义
*************************************************************************/
enum PTR_TYPE
{
    PTR_TYPE_RECEIPT = 1,
    PTR_TYPE_JOURNAL = 2,
};

/*************************************************************************
// INI配置结构体 定义
*************************************************************************/
struct stPTRConfig
{
    PTR_TYPE            type;
    char                szDevDllName[256];
    char                szSDKPath[256];                 // 设备SDK库路径
    int                 nDriverType;
    bool                bDetectBlackStripe;
    int                 nVerifyField;
    WORD                wCutPaperMode;                  // 切纸方式
    int                 nFeed;
    int                 nPageSize;
    int                 nPageLine;
    DWORD               dwMarkHeader;
    bool                bEnableSplit;
    WORD                wPrintFontMode;                 // 指定打印字体格式
    int                 nTakeSleep;
    int                 nLineSize;                      // 30-00-00-00(FT#0008)
    int                 nRawDataInPar;                  // 30-00-00-00(FT#0045) RawData入参模式
    INT                 nRawFontType;                   // RawData打印字体
    CHAR                szRawFontName[256];             // RawData打印字体名
    INT                 nRawFontSize[3];                // RawData打印字体号(0字号,1高度,2宽度)
    int                 nBlockMove;                     // 物理黑标偏移
    STDEVOPENMODE       stDevOpenMode;                  // 设备打开模式   // 30-00-00-00(FT#0067)
    WORD                wPrintMode;                     // 打印方式
    WORD                wPrtBarcodeMode[5];             // 打印条形码码格式
    WORD                wPrtPDF417Mode[7];              // 打印PDF417码格式
    WORD                wPrtQrcodeMode[4];              // 打印条形码码格式
    CHAR                szBarFontList[24][32];          // Field.FONT属性指定条码关键字列表
    int                 nFieldCPIMode;                  // Field CPI(字符宽)单位(0:缺省,1:毫米,2:0.1毫米,缺省0)
    int                 nFieldLPIMode;                  // Field LPI(行高)单位(0:缺省,1:毫米,2:0.1毫米,缺省0)
    STPRINTMODE         stPrtModeRaw;                   // RawData打印模式
    STPRINTMODE         stPrtModeForm;                  // PrintForm打印模式

    stPTRConfig()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(stPTRConfig));
    }
};

// wPrtBarcodeMode[5]说明: [0]指定条码类型,[1]基本元素宽度点数,[2]条码高度,
//                        [3]指定HRI字符的字体类型,[4]指定HRI字符的位置
// wPrtPDF417Mode[7]说明: [0]基本元素宽度点数,[1]元素高度点数,[2]行数,[3]列数,
//                       [4]外观比高度,[5]外观比宽度,[6]纠错级别
// wPrtQrcodeMode[4]说明: [0]基本元素宽度点数,[1]符号类型,[2]语言模式,
//                        [3]纠错级别

/*************************************************************************
// 主处理类
*************************************************************************/
class CXFS_PTR : public ICmdFunc, public CTextPrinter, public ConvertVar
{

public:
    CXFS_PTR();
    virtual~ CXFS_PTR();

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
    HRESULT InnerReadForm(LPWFSPTRREADFORM pInData);
    HRESULT InnerMediaControl(DWORD dwControl);
    HRESULT InnerRawData(BOOL bExpectResp, ULONG nSize, LPBYTE pData);
    HRESULT InnerReset(DWORD dwMediaControl, USHORT usBinIndex);
protected:
    // 重载CSPBaseClass的方法
    HRESULT OnInit();
    HRESULT OnExit();

    // 子类继承实现的函数,PrintForm最后处理(该处理当前支持HOTS/BKC310/BTNH80M打印机)
    // 新增打印机可在在该接口中根据设备类型进行分类处理
    virtual HRESULT EndForm(PrintContext *pContext);

protected:
    bool LoadDevDll(LPCSTR ThisModule);
    void InitConifig();
    HRESULT InitStatus();
    HRESULT EndForm_SNBC(PrintContext *pContext);   // 新北洋打印机处理1
    HRESULT EndForm_Print_SNBC(PrintContext *pContext);  // 打印机处理2
    // 文本打印前处理(RawData专用)
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
    void UpdateDeviceStatus(int iRet);
    HRESULT StartOpen(BOOL bReConn = FALSE);
    HRESULT SetInit();
    INT  GetBarFontMode(LPSTR lpData);

protected:
    BOOL                    m_bNeedKeepJammedStatus;
    bool                    m_bNeedReset;
    BOOL                    m_bPaperCutted;
    BOOL                    m_bReset;
    stPTRConfig             m_sConfig;
    WAIT_TAKEN_FLAG         m_WaitTaken;
private:
    CQtDLLLoader<IDevPTR>          m_pPrinter;
    CQtDLLLoader<ISPBasePTR>       m_pBase;
    CXfsRegValue                   m_cXfsReg;
    std::string                    m_strLogicalName;
    std::string                    m_strSPName;
    STPRINTFORMAT                  m_stPrintFormat;
    CSimpleMutex                  *m_pMutexGetStatus;
protected:
    CExtraInforHelper              m_cExtra;
    CWfsPtrStatus                  m_sStatus;
    CWfsPtrCaps                    m_sCaps;
    BOOL                           m_bBusy;     // 30-00-00-00(FT#0066)

private:
    DWORD                          dwTakeTimeSize;
    INT                            m_nReConErr;
};

