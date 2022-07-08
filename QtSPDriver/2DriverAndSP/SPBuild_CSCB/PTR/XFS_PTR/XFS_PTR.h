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


// PTR SP 版本号
const BYTE byVRTU[17] = {"HWPTRSTE01000100"};

// 事件日志
static const char *ThisFile = "XFS_PTR.cpp";

#define LOG_NAME     "XFS_PTR.log"

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

// INI中指定[int型]设备类型 转换为 STR型
#define DEVTYPE2STR(n) \
    (n == DEV_HOTS ? DEV_HOTS_STR : \
      (n == DEV_SNBC_BKC310 ? DEV_SNBC_BKC310_STR : \
        (n == DEV_SNBC_BTNH80 ? DEV_SNBC_BTNH80_STR : DEV_SNBC_BTNH80_STR)))

enum WAIT_TAKEN_FLAG
{
    WTF_NONE = 0,   //不等待
    WTF_TAKEN = 1,  //等待用户取纸
};

enum PTR_TYPE
{
    PTR_TYPE_RECEIPT = 1,
    PTR_TYPE_JOURNAL = 2,
};

// INI配置结构体
struct stPTRConfig
{
    PTR_TYPE             type;
    char                 szDevDllName[256];
    int                  nDriverType;
    bool                 bDetectBlackStripe;
    int                  nVerifyField;
    int                  nFeed;
    int                  nPageSize;
    int                  nPageLine;
    DWORD                dwMarkHeader;
    bool                 bEnableSplit;
    int                  nTakeSleep;
    int                  nLineSize;         // 30-00-00-00(FT#0008)
    int                  nRawDataInPar;     // 30-00-00-00(FT#0045) RawData入参模式
};

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
    HRESULT ControlMedia(DWORD dwControl);
    HRESULT SendRawData(BOOL bExpectResp, ULONG nSize, LPBYTE pData);
    HRESULT ResetDevice(DWORD dwMediaControl, USHORT usBinIndex);
protected:
    // 重载CSPBaseClass的方法
    HRESULT OnInit();
    HRESULT OnExit();

    // 子类继承实现的函数,PrintForm最后处理(该处理当前支持HOTS/BKC310/BTNH80M打印机)
    // 新增打印机可在在该接口中根据设备类型进行分类处理
    virtual HRESULT EndForm(PrintContext *pContext);
    HRESULT EndForm_SNBC(PrintContext *pContext);   // 新北洋打印机处理
    HRESULT EndForm_HOTS(PrintContext *pContext);   // HOST打印机处理
    // 文本打印前处理(HOTS打印机专用)
    HRESULT PrintString(const char *pBuffer, DWORD dwSize, BOOL bIsFromPrint = FALSE);
    // 文本打印前处理(新北洋打印机专用,若新增打印机,可根据新机修改或新增处理接口)
    HRESULT AddPrintString(const char *pBuffer, DWORD dwSize, BOOL bIsFromPrint, char *pBuffOut, DWORD &dwSizeOut);
    // 图片打印(指定宽高,当前为HOST打印机专用)
    HRESULT PrintImage(LPCSTR szImagePath, int nDstWidth, int nDstHeight);
    // 图片打印(传入X,Y坐标,当前为BKC310/NH80M打印机使用)
    HRESULT PrintImageOrg(LPCSTR szImagePath, ULONG ulOrgX, ULONG ulOrgY);
    // 文本打印
    HRESULT PrintData(const char *pBuffer, DWORD dwSize);

protected:
    bool LoadDevDll(LPCSTR ThisModule);
    void InitConifig();
    long InitStatus();
    long UpdateStatus();
    //去掉不可打印字符
    void RemoveUnPrintableChar(ULONG ulInSize, const LPBYTE pInData, ULONG &ulOutSize, LPBYTE pOutData);
    long ConvertErrCode(long lRes);
    void UpdateDeviceStatus(int iRet);
    HRESULT StartOpen();

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

private:
    DWORD                          dwTakeTimeSize;
};

