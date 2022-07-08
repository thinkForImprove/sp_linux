/***************************************************************
* 文件名称：XFS_CSR.h
* 文件描述：票据受理模块命令子处理接口 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/

#pragma once

#include "PTRFORM/TextPrinter.h"
#include "PTRFORM/PTRData.h"
#include "PTRFORM/PTRForm.h"
#include "QtTypeDef.h"
#include "IDevPTR.h"
#include "ISPBasePTR.h"
#include "file_access.h"
#include "cjson_object.h"
#include "def.h"

/*************************************************************************
// 未分类 宏定义
*************************************************************************/
// 下发DevXXX缺省JSON定义
#define NOTE_GETJSON        "{\"UseJsonArea\":0}"

// 设备Check 定义: 非ONLINE/BUSY,硬件错误;BUSY,设备未准备好
#define CHECK_DEVICE() \
    if (m_stStatus.fwDevice != WFS_PTR_DEVONLINE && m_stStatus.fwDevice != WFS_PTR_DEVBUSY) \
    { \
        Log(ThisModule, -1, "Device != ONLINE|BUSY, Return: %d.", WFS_ERR_HARDWARE_ERROR); \
        return WFS_ERR_HARDWARE_ERROR; \
    } \
    if (m_stStatus.fwDevice == WFS_PTR_DEVBUSY) \
    { \
        Log(ThisModule, -1, "Device == BUSY, Return: %d.", WFS_ERR_DEV_NOT_READY); \
        return WFS_ERR_DEV_NOT_READY; \
    }

// Check: 通道内有票,报错并记录LOG
#define ERR_MEDIA_PRESENT() \
    if (m_stStatus.fwMedia == WFS_PTR_MEDIAPRESENT) \
    { \
        Log(ThisModule, -1, "Media Status = %d[MEDIAPRESENT)], 通道内有票据存在, Return: %d.", \
            WFS_PTR_MEDIAPRESENT, WFS_ERR_PTR_SEQUENCEINVALID); \
        return WFS_ERR_PTR_SEQUENCEINVALID; \
    }

// Check: 通道内无票,报错并记录LOG
#define ERR_MEDIA_NOTPRESENT() \
    if (m_stStatus.fwMedia == WFS_PTR_MEDIANOTPRESENT) \
    { \
        Log(ThisModule, -1, "Media Status = %d[MEDIANOTPRESENT)], 通道内没有票据存在, Return: %d.", \
            WFS_PTR_MEDIAPRESENT, WFS_ERR_PTR_SEQUENCEINVALID); \
        return WFS_ERR_PTR_SEQUENCEINVALID; \
    }

/*************************************************************************
// Taken标记 定义
*************************************************************************/
enum WAIT_TAKEN_FLAG
{
    WTF_NONE = 0,   //不等待
    WTF_TAKEN = 1,  //等待用户取纸
};

//----------------------结构体 定义----------------------
#define BOX_COUNT       16      // 箱数据支持上限
#define BOX_ISHAVE      0       // 存在
#define BOX_NOHAVE      -1      // 不存在
#define BOX_STORAGE     0       // 发票箱(存储箱)
#define BOX_RETRACT     1       // 回收箱
#define BOX_EMPTY       0       // 箱空
#define BOX_HIGHLOW     1       // 箱将满/箱将空
#define BOX_FULL        2       // 箱满
typedef struct st_NoteBox
{
    BOOL bIsHave;           // 是否使用中
    USHORT usBoxNo;         // 回收箱序号(1~16)
    USHORT usNoteCount;     // 当前票据张数
    USHORT usHighThreshold; // HIGH报警阀值
    USHORT usFullThreshold; // FULL报警阀值
    BOOL   bRetractFull;    // 物理状态满(T/F)
} STNOTEBOX, *LPSTNOTEBOX;
typedef struct NoteBox_List
{
    USHORT usBoxCount;         // 回收箱数目
    STNOTEBOX stNoteBox[BOX_COUNT];

    NoteBox_List()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(NoteBox_List));
    }
    INT GetBoxCount()     // 返回回收箱数目
    {
        INT nCount = 0;
        for (INT i = 0; i < BOX_COUNT; i ++)
        {
            if (stNoteBox[i].bIsHave == TRUE)
            {
                nCount ++;
            }
        }
        return nCount;
    }
    INT GetBoxNo(USHORT usSerial)   // 根据顺序号取有效箱号
    {
        if (usSerial >= 0 && usSerial < 16)
        {
            if (stNoteBox[usSerial].bIsHave == TRUE)
            {
                return stNoteBox[usSerial].usBoxNo;
            }
        }
        return BOX_NOHAVE;  // 不存在
    }
    INT BoxIsHave(USHORT usBoxNo) // 指定箱号是否存在
    {
        for (INT i = 0; i < BOX_COUNT; i ++)
        {
            if (stNoteBox[i].bIsHave == TRUE && stNoteBox[i].usBoxNo == usBoxNo)
            {
                return BOX_ISHAVE;       // 存在
            }
        }
        return BOX_NOHAVE;  // 不存在
    }
    INT GetNoteCount(USHORT usBoxNo)   // 获取指定箱中票据数
    {
        for (INT i = 0; i < BOX_COUNT; i ++)
        {
            if (stNoteBox[i].bIsHave == TRUE && stNoteBox[i].usBoxNo == usBoxNo)
            {
                return stNoteBox[i].usNoteCount;
            }
        }
        return BOX_NOHAVE;  // 不存在
    }
    INT GetBoxOrder(USHORT usBoxNo)   // 获取指定箱号的顺序索引(用于INI)
    {
        for (INT i = 0; i < BOX_COUNT; i ++)
        {
            if (stNoteBox[i].bIsHave == TRUE && stNoteBox[i].usBoxNo == usBoxNo)
            {
                return i;
            }
        }
        return BOX_NOHAVE;  // 不存在
    }
    INT GetBoxStat(USHORT usBoxNo)   // 获取指定箱号的状态
    {
        for (INT i = 0; i < BOX_COUNT; i ++)
        {
            if (stNoteBox[i].bIsHave == TRUE && stNoteBox[i].usBoxNo == usBoxNo)
            {
                if (stNoteBox[i].usNoteCount >= stNoteBox[i].usFullThreshold)
                {
                    return BOX_FULL;
                } else
                if (stNoteBox[i].usNoteCount >= stNoteBox[i].usHighThreshold)
                {
                    return BOX_HIGHLOW;
                } else
                {
                    return BOX_EMPTY;
                }
            }
        }
        return BOX_NOHAVE;  // 不存在
    }
    INT SetNoteCount(USHORT usBoxNo, USHORT usCnt, BOOL bIsAdd = TRUE)    // 设置回收计数
    {
        INT nCount = 0;
        for (INT i = 0; i < BOX_COUNT; i ++)
        {
            if (stNoteBox[i].bIsHave == TRUE && stNoteBox[i].usBoxNo == usBoxNo)
            {
                nCount = (bIsAdd == TRUE ? stNoteBox[i].usNoteCount + usCnt : stNoteBox[i].usNoteCount - usCnt);
                stNoteBox[i].usNoteCount = (nCount < 0 ? 0 : nCount);
                return stNoteBox[i].usNoteCount;
            }
        }
        return BOX_NOHAVE;
    }
    INT GetRetraceFull(USHORT usBoxNo)   // 获取物理票箱Full状态
    {
        for (INT i = 0; i < BOX_COUNT; i ++)
        {
            if (stNoteBox[i].bIsHave == TRUE && stNoteBox[i].usBoxNo == usBoxNo)
            {
                return (stNoteBox[i].bRetractFull == TRUE ? BOX_FULL : BOX_EMPTY);
            }
        }
        return BOX_NOHAVE;
    }
    INT SetRetraceFull(USHORT usBoxNo, BOOL bIsStat)   // 设置物理票箱Full状态
    {
        for (INT i = 0; i < BOX_COUNT; i ++)
        {
            if (stNoteBox[i].bIsHave == TRUE && stNoteBox[i].usBoxNo == usBoxNo)
            {
                stNoteBox[i].bRetractFull = bIsStat;
                return (stNoteBox[i].bRetractFull == TRUE ? BOX_FULL : BOX_EMPTY);
            }
        }
        return BOX_NOHAVE;
    }
} NOTEBOXLIST, *LPNOTEBOXLIST;

// RSC-D400M设备特殊设置
typedef
struct st_RSCD400M_cfg
{
    USHORT usLeftNoteBoxNo;         // 左票箱指定编号
    USHORT usRightNoteBoxNo;        // 右票箱指定编号
    std::string stdFontBuffer;      // 字体信息
    USHORT usUseDistAreaSupp;       // 是否使用Form/INI设置识别范围
    FLOAT  fPictureZoom;            // 图片缩放
    USHORT usDPIx;                  // X方向DPI
    USHORT usDPIy;                  // Y方向DPI
    st_RSCD400M_cfg()
    {
        Clear();
    }
    void Clear()
    {
        usLeftNoteBoxNo = 0;
        usRightNoteBoxNo = 0;
        usUseDistAreaSupp = 1;
        stdFontBuffer.clear();
        usDPIx = 0;
        usDPIy = 0;
    }
}STCFG_RSCD400M, *LPSTCFG_RSCD400M;
// ini获取
typedef
struct st_csr_ini_config
{
    CHAR                    szDevDllName[256];                  // DevXXX动态库
    USHORT                  nDriverType;                        // 设备类型
    INT                     nVerifyField;                       // Form->Field检查方式
    INT                     nOpenFailRet;                       // Open失败时返回值
    USHORT                  usBank;                             // 指定银行
    CHAR                    szBankCode[12];                     // 银行别码
    CHAR                    szSDKPath[256];                     // 设备SDK库路径
    STCFG_RSCD400M          stCfg_RSCD400M;                     // RSC-D400M设备配置
    map<USHORT, std::string>       m_Map_NotePar_GetList;       // 缺省票面获取Map

    st_csr_ini_config()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_csr_ini_config));
        m_Map_NotePar_GetList.clear();
    }
}STCSRINICONFIG, *LPSTCSRINICONFIG;

//-----------------处理类 定义-------------------------
class CXFS_CSR : public ICmdFunc, public CTextPrinter, public ConvertVar
{

public:
    CXFS_CSR();
    virtual~ CXFS_CSR();

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

protected:  // 功能处理
    HRESULT StartOpen(BOOL bReConn = FALSE);   // Open设备及初始化相关子处理
    void InitConifig();
    HRESULT InitStatus();                                                               // 初始化状态类变量
    HRESULT InitCaps();                                                                 // 初始化能力值类变量
    void UpdateDeviceStatus();                                                          // 状态获取
    HRESULT InnerControlMedia(DWORD dwControl);                                         // 介质控制处理
    HRESULT InnerPrintForm(LPWFSPTRPRINTFORM pInData);                                  // 格式化打印处理
    HRESULT InnerReadImage(LPWFSPTRIMAGEREQUEST lpImgRequest, LPWFSPTRIMAGE *&lppImage, DWORD dwTimeOut);
    HRESULT InnerReadForm(LPWFSPTRREADFORM pInData);
    HRESULT MediaRetract(const LPUSHORT lpusBinNum, LPUSHORT &lpusBinNumOut);
    HRESULT MediaInsertWait(DWORD dwTimeOut);
    void SetRetractBoxCount(USHORT usBoxNo, USHORT usCnt, BOOL bIsAdd = TRUE);          // 设置回收箱计数并记录

protected:  // 重载函数
    // 重载CSPBaseClass的方法
    HRESULT OnInit();
    HRESULT OnExit();

    // 重载TextPrinter的方法
    virtual HRESULT EndForm(PrintContext *pContext);                                // 重写该方法,处理PrintForm打印数据

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
    WORD ConvertPaperCode(INT nCode);                                               // 票箱号转换为WFS格式
    WORD NoteTypeConvert(LPSTR lpNoteType, WORD wBank);                             // 指定票据类型转换为DevCPR定义
    WORD NoteTypeIsHave(LPSTR lpNoteType, WORD wBox);                               // 指定票据类型是否对应票箱存在
    HRESULT  GetFormFieldToJSON(LPSTR lpForm, CJsonObject &cJson);
    HRESULT  GetDefFieldToJSON(LPSTR lpStr, CJsonObject &cJson);

protected:
    BOOL                    m_bNeedKeepJammedStatus;
    bool                    m_bNeedReset;
    BOOL                    m_bPaperCutted;
    BOOL                    m_bReset;
    STCSRINICONFIG          m_stConfig;
    NOTEBOXLIST             m_stNoteBoxList;            // 回收箱结构体信息
    WFSPTRRETRACTBINS       m_stWFSRetractBin[16];      // 回收箱信息(当前)
    WFSPTRRETRACTBINS       m_stWFSRetractBinOLD[16];   // 回收箱信息(上一次)
    WAIT_TAKEN_FLAG         m_WaitTaken;
private:
    CQtDLLLoader<IDevPTR>          m_pPrinter;
    CQtDLLLoader<ISPBasePTR>       m_pBase;
    CXfsRegValue                   m_cXfsReg;
    std::string                    m_strLogicalName;
    std::string                    m_strSPName;
    //STPRINTFORMAT                  m_stPrintFormat;
    CSimpleMutex                  *m_pMutexGetStatus;
protected:
    CExtraInforHelper              m_cExtra;
    CWfsPtrStatus                  m_stStatus;          // 当前状态信息
    CWfsPtrCaps                    m_sCaps;

private:
    DWORD   dwTakeTimeSize;
    DWORD   dwMediaWidth;
    DWORD   dwMediaHeight;
    CHAR    m_szErrCode[32];
    CHAR    m_szErrCodeDetail[1024];
    BOOL    m_bCancelFlag;
    INT     m_nReConRet;
    BOOL    m_bInsertEventRep;                          // Insert允许事件上报标记
    BOOL    m_bCmdRunning;                              // 当前是否有命令执行中

private:    // INI
    WORD    m_wBankNo;
    RECT    m_stMargin;                                 // 介质边距


};

