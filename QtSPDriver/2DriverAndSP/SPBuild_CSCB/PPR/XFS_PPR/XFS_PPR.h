#pragma once
//存折打印机
#include "PPRFORM/TextPrinter.h"
#include "PPRFORM/PTRData.h"
#include "PPRFORM/PTRForm.h"
#include "QtTypeDef.h"
#include "IDevPPR.h"
#include "ISPBasePTR.h"
#include "file_access.h"
#include "cjson_object.h"
#include "def.h"

#define IDS_ERR_ON_EXECUTE_ERR              "调用[%s]失败[%s]"
#define IDS_ERR_FORM_INVALID                "FORM无效[%s]"
#define IDS_ERR_MEDIA_INVALID               "MEDIA无效[%s]"
#define IDS_ERR_FORM_NOT_FOUND              "FORM[%s]没有找到"
#define IDS_ERR_MEDIA_NOT_FOUND             "MEDIA[%s]没有找到"
#define IDS_ERR_FILED_NOT_FOUND             "FIELD[%s]没有找到"
#define IDS_ERR_MEDIA_OVERFLOW              "打印FORM[%s]到MEDIA[%s]上溢出"
#define IDS_ERR_MEDIA_OVERFLOW_PRINTAREA    "打印FORM[%s]到MEDIA[%s]上溢出打印区域"
#define IDS_ERR_FIELD_ERROR                 "字段域[%s]错误"
#define IDS_ERR_FIELD_EMPTY                 "FORM[%s]中无字段域"
#define IDS_ERR_START_FORM                  "调用StartForm[%s]失败"
#define IDS_ERR_REQUIRED_FIELD              "打印FORM[%s]的字段[%s]没有提供数据"
#define IDS_ERR_PRINT_FIELD                 "打印FORM[%s]字段[%s]失败[%s]"
#define IDS_ERR_NO_CHARSET                  "系统缺少中文字符集"

#define DEVTYPE_CHG(n)      (n == 0 ? "0" : "")

#define BANK_CODE_PSBC      "403"               // 银行别码:邮储
#define BANK_NO_PSBC        1                   // INI指定银行编号::邮储
#define BANK_CODE2NO(X)     X == BANK_CODE_PSBC ? BANK_NO_PSBC : 0

enum WAIT_TAKEN_FLAG
{
    WTF_NONE = 0,   //不等待
    WTF_TAKEN = 1,  //等待用户取纸
};

//----------------------结构体 定义----------------------
//----------------------结构体 定义----------------------
// 票据箱列表结构体定义
#define NOTEBOX_COUNT   16          // 最多支持箱数目
typedef struct st_NoteBox
{
    BOOL bIsHave;       // 是否使用中
    WORD wBoxNo;        // 票据箱序号(1~16)
    WORD wBoxType;      // 票据箱类型(0存储箱/1回收箱)
    WORD wNoteType;     // 票据类型
    WORD wNoteCount;    // 票据张数
    WORD wThreshold;    // 报警阀值(HIGH回收箱/LOW存储箱)
    WORD wFullThreshold;// FULL报警阀值(回收箱使用)
} STNOTEBOX, *LPSTNOTEBOX;
typedef struct NoteBox_List
{
    WORD wBoxCount;         // 票据箱数目
    STNOTEBOX NoteBox[16];

    NoteBox_List()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(NoteBox_List));
    }
    INT bIsRetractBox(WORD wBoxNo) // 指定箱号是否回收箱(1~16)
    {
        for (INT i = 0; i < wBoxCount; i ++)
        {
            if (NoteBox[i].bIsHave == TRUE && NoteBox[i].wBoxNo == wBoxNo &&
                NoteBox[i].wBoxType == 1)
            {
                return 1;   // 存在且为回收箱
            }
            if (NoteBox[i].bIsHave == FALSE && NoteBox[i].wBoxNo == wBoxNo)
            {
                return -1;  // 指定箱号不存在
            }
            if (NoteBox[i].bIsHave == TRUE && NoteBox[i].wBoxNo == wBoxNo &&
                NoteBox[i].wBoxType != 1)
            {
                return 0;   // 存在但非回收箱
            }

        }
        return -1;
    }
    USHORT nIsRetractBox()    // 是否有回收箱(返回数目)
    {
        USHORT nCount = 0;
        for (INT i = 0; i < wBoxCount; i ++)
        {
            if (NoteBox[i].bIsHave == TRUE && NoteBox[i].wBoxType == 1)
            {
                nCount ++;
            }
        }
        return nCount;
    }
    INT bIsStorageBox(WORD wBoxNo) // 指定箱号是否存储箱
    {
        for (INT i = 0; i < wBoxCount; i ++)
        {
            if (NoteBox[i].bIsHave == TRUE && NoteBox[i].wBoxNo == wBoxNo &&
                NoteBox[i].wBoxType == 0)
            {
                return 1;   // 存在且为存储箱
            }
            if (NoteBox[i].bIsHave == FALSE && NoteBox[i].wBoxNo == wBoxNo)
            {
                return -1;  // 指定箱号不存在
            }
            if (NoteBox[i].bIsHave == TRUE && NoteBox[i].wBoxNo == wBoxNo &&
                NoteBox[i].wBoxType != 0)
            {
                return 0;   // 存在但非存储箱
            }
        }
        return -1;
    }
    INT nIsStorageBox()     // 是否有存储箱
    {
        INT nCount = 0;
        for (INT i = 0; i < wBoxCount; i ++)
        {
            if (NoteBox[i].bIsHave == TRUE && NoteBox[i].wBoxType == 0)
            {
                nCount ++;
            }
        }
        return nCount;
    }
    INT nGetBoxCount(WORD wBoxNo)   // 指定箱中票据数
    {
        for (INT i = 0; i < wBoxCount; i ++)
        {
            if (NoteBox[i].bIsHave == TRUE && NoteBox[i].wBoxNo == wBoxNo)
            {
                return NoteBox[i].wNoteCount;
            }
        }
        return -1;  // 不存在
    }
    INT nGetNoteTypeIsBox(WORD wBoxNo) // 指定箱号中票据类型
    {
        for (INT i = 0; i < wBoxCount; i ++)
        {
            if (NoteBox[i].bIsHave == TRUE && NoteBox[i].wBoxNo == wBoxNo)
            {
                if (NoteBox[i].wBoxType == 1)
                {
                    return 0;   // 支持所有票据(通常回收)
                }
                return NoteBox[i].wNoteType;
            }
        }
        return -1;
    }

} NOTEBOXLIST, *LPNOTEBOXLIST;

class CXFS_PPR : public ICmdFunc, public CTextPrinter
{

public:
    CXFS_PPR();
    virtual~ CXFS_PPR();

public:
    // 开始运行SP
    long StartRun();

    // 基本接口
    virtual HRESULT OnOpen(LPCSTR lpLogicalName);   // Open设备及初始化相关
    virtual HRESULT OnClose();                      // 关闭设备
    virtual HRESULT OnStatus();                     // 实时状态更新
    virtual HRESULT OnWaitTaken();                  // Taken事件处理
    virtual HRESULT OnCancelAsyncRequest();         // 命令取消
    virtual HRESULT OnUpdateDevPDL();               // 固件升级

    virtual HRESULT GetStatus(LPWFSPTRSTATUS &lpStatus);
    virtual HRESULT GetCapabilities(LPWFSPTRCAPS &lpCaps);
    virtual HRESULT GetFormList(LPSTR &lpszFormList);
    virtual HRESULT GetMediaList(LPSTR &lpszMediaList);
    virtual HRESULT GetQueryForm(LPCSTR lpFormName, LPWFSFRMHEADER &lpFrmHeader);
    virtual HRESULT GetQueryMeida(LPCSTR lpMediaName, LPWFSFRMMEDIA &lpFrmMedia) ;
    virtual HRESULT GetQueryField(const LPWFSPTRQUERYFIELD lpQueryField, LPWFSFRMFIELD *&lpszMediaList);
    virtual HRESULT GetReadFormList(LPSTR &lpszFormList);

    // PTR类型接口(EXECUTE)
    virtual HRESULT MediaControl(const LPDWORD lpdwMeidaControl);                                                   // 介质控制
    virtual HRESULT PrintForm(const LPWFSPTRPRINTFORM lpPrintForm, DWORD dwTimeOut);                                // 格式化打印
    virtual HRESULT ReadForm(const LPWFSPTRREADFORM lpReadForm, LPWFSPTRREADFORMOUT &lpReadFormOut, DWORD dwTimeOut);// 格式化读
    virtual HRESULT RawData(const LPWFSPTRRAWDATA lpRawData, LPWFSPTRRAWDATAIN &lpRawDataIn, DWORD dwTimeOut);      // 无格式打印
    virtual HRESULT MediaExtents(const LPWFSPTRMEDIAUNIT lpMediaUnit, LPWFSPTRMEDIAEXT &lpMediaExt);                // 获得插入物理设备中的媒介的长宽度
    virtual HRESULT ResetCount(const LPUSHORT lpusBinNum);                                                          // 将媒介回收计数由当前值归零
    virtual HRESULT ReadImage(const LPWFSPTRIMAGEREQUEST lpImgRequest, LPWFSPTRIMAGE *&lppImage, DWORD dwTimeOut);  // 获取图象数据
    virtual HRESULT Reset(const LPWFSPTRRESET lpReset);                                                             // 复位
    virtual HRESULT RetractMedia(const LPUSHORT lpusBinNum, LPUSHORT &lpusBinNumOut);                               // 媒介回收
    virtual HRESULT DispensePaper(const LPWORD lpPaperSource);                                                      // 纸张移动
    virtual HRESULT SetGuidanceLight(const LPWFSPTRSETGUIDLIGHT lpSetGuidLight);                                    // 指示灯控制
    virtual HRESULT EndForm(PrintContext *pContext);

protected:  // 功能处理
    void InitConfig();
    long InitStatus();                      // 初始化状态类变量
    long InitCaps();                        // 初始化能力值类变量
    long UpdateStatus();                    // 状态更新
    void UpdateDeviceStatus();              // 状态获取
    //HRESULT ControlMedia(DWORD dwControl);  // 介质控制处理
    HRESULT InnerPrintForm(LPWFSPTRPRINTFORM pInData);  // 格式化打印处理
    HRESULT InnerReadForm(LPWFSPTRREADFORM pInData);

    //去掉不可打印字符
    void RemoveUnPrintableChar(ULONG ulInSize, const LPBYTE pInData, ULONG &ulOutSize, LPBYTE pOutData);
    BOOL NeedFormatString() const;

protected:  // 重载函数
    // 重载CSPBaseClass的方法
    HRESULT OnInit();
    HRESULT OnExit();

    // 重载TextPrinter的方法
    virtual HRESULT ControlMedia(DWORD dwControl);      // // 介质控制处理
    virtual HRESULT SendRawData(BOOL bExpectResp, ULONG nSize, LPBYTE pData);
    virtual HRESULT Reset(DWORD dwMediaControl, USHORT usBinIndex);
    virtual HRESULT EndFormToJSON(PrintContext *pContext, CJsonObject &cJsonData);

    //以下是子类必须实现的函数,EndForm调用它
    virtual HRESULT PrintImage(LPCSTR szImagePath, int nDstWidth, int nDstHeight);
    virtual HRESULT PrintImageOrg(LPCSTR szImagePath, ULONG ulOrgX, ULONG ulOrgY);
    virtual long PrintData(const char *pBuffer, DWORD dwSize);
    virtual long PrintMICR(const char *pBuffer, DWORD dwSize);
    virtual HRESULT PrintString(const char *pBuffer, DWORD dwSize, BOOL bIsFromPrint = FALSE);
    virtual HRESULT AddPrintString(const char *pBuffer, DWORD dwSize, BOOL bIsFromPrint, char *pBuffOut, DWORD &dwSizeOut);
    virtual HRESULT ReadItem(CMultiString cFieldInfo);
    virtual long PrintData2(const char *pBuffer, unsigned long ulDataLen,LONG ulOrgX, ULONG ulOrgY);

public:     // 事件消息
    void FireHWEvent(DWORD dwHWAct, char *pErr);                                    // 上报Device HWERR事件
    void FireStatusChanged(WORD wStatus);                                           // 上报状态变化事件
    void FireNoMedia(LPCSTR szPrompt);                                              // 上报无媒介事件
    void FireMediaInserted();                                                       // 上报媒介放入事件
    void FireFieldError(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure);      // 上报Field错误事件
    void FireFieldWarning(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure);	// 上报Field警告事件
    void FireRetractBinThreshold(USHORT BinNumber, WORD wStatus);                   // 上报回收箱变化事件
    void FireMediaTaken();                                                          // 上报媒介取走事件
    void FirePaperThreshold(WORD wSrc, WORD wStatus);                               // 上报纸状态/票箱状态变化事件
    void FireTonerThreshold(WORD wStatus);                                          // 上报碳带状态变化事件
    void FireInkThreshold(WORD wStatus);                                            // 上报墨盒状态变化事件
    void FireLampThreshold(WORD wStatus);                                           // 上报灯状态变化事件
    void FireSRVMediaInserted();                                                    //
    void FireMediaDetected(WORD wPos, USHORT BinNumber);                            // 上报复位中检测到设备内有媒介事件

private:    // 格式转换相关
    WORD ConvertErrCode(INT nRet);                  // 错误码转换为WFS格式
    WORD ConvertDeviceStatus(WORD wDevStat);        // 设备状态转换为WFS格式
    WORD ConvertMediaStatus(WORD wMediaStat);       // Media状态转换为WFS格式
    WORD ConvertPaperStatus(WORD wPaperStat);       // Paper状态转换为WFS格式
    WORD ConvertTonerStatus(WORD wTonerStat);       // Toner状态转换为WFS格式
    WORD ConvertInkStatus(WORD wInkStat);           // Ink状态转换为WFS格式
    WORD ConvertRetractStatus(WORD wRetractStat);   // Retract状态转换为WFS格式
    WORD ConvertPaperCode(INT nCode);               // 票箱号转换为WFS格式
    WORD NoteTypeConvert(LPSTR lpNoteType, WORD wBank);// 指定票据类型转换为DevCPR定义    
    WORD NoteTypeIsHave(LPSTR lpNoteType, WORD wBox);// 指定票据类型是否对应票箱存在
    HRESULT  GetFormFieldToJSON(LPSTR lpForm, CJsonObject &cJson);


protected:
    BOOL                    m_bNeedKeepJammedStatus;
    bool                    m_bNeedReset;
    BOOL                    m_bPaperCutted;
    BOOL                    m_bReset;
    stPTRConfig             m_sConfig;
    WAIT_TAKEN_FLAG         m_WaitTaken;
    NOTEBOXLIST             m_stNoteBoxList;        // 票据箱信息
private:
    CQtDLLLoader<IDevPPR>          m_pPrinter;
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
    DWORD   dwTakeTimeSize;
    DWORD   dwMediaWidth;
    DWORD   dwMediaHeight;
    STPRTALIGNMODE                 m_stPrtAlignModeList[16];
    LPUSHORT lpRetractBoxIdx;
    ICmdFunc                            *m_pCmdFunc;
    BOOL                        bCancelInsertMedia;
    BOOL                        bInsertMediaTimeOut;
    LONG m_MediaTop;
    LONG m_MediaBotton;
    CMultiString m_Fields;  //test by guojy
};

