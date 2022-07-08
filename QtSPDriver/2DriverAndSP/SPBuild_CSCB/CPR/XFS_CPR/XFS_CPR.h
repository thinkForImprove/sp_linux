/***************************************************************
* 文件名称：XFS_CPR.h
* 文件描述：票据发放模块命令子处理接口 头文件
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
#include "data_convertor.h"
#include "def.h"
#include "string.h"
#include "QtTypeInclude.h"
#include <QTextCodec>
#include <stdlib.h>
#include <unistd.h>

//-------------------------宏定义------------------------------------
// 错误提示定义
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
#define IDS_ERR_FORM_ATTRI_INV              "FORM.%s无效"


// 银行编号 定义
#define BANK_NO_CSBC        1                   // INI指定银行编号:长沙银行
#define BANK_NO_PSBC        2                   // INI指定银行编号:邮储

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

enum WAIT_TAKEN_FLAG
{
    WTF_NONE = 0,   //不等待
    WTF_TAKEN = 1,  //等待用户取纸
};

//----------------------结构体 定义----------------------
#define BOX_COUNT       16      // 票据箱数据支持上限(包含存储箱和回收箱)
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
    BOOL bIsTally;          // 是否使用计数
    USHORT usBoxNo;         // 票据箱序号(1~16)
    USHORT usBoxType;       // 票据箱类型(0存储箱/1回收箱)
    USHORT usNoteType;      // 票据类型
    USHORT usNoteCount;     // 票据张数
    USHORT usThreshold;     // 报警阀值(HIGH回收箱/LOW存储箱)
    USHORT usFullThreshold; // FULL报警阀值(回收箱使用)
} STNOTEBOX, *LPSTNOTEBOX;
typedef struct NoteBox_List
{
    USHORT usBoxCount;         // 票据箱数目
    STNOTEBOX stNoteBox[BOX_COUNT];

    NoteBox_List()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(NoteBox_List));
    }
    INT nIsStorageBox()     // 是否有存储箱(返回数目)
    {
        INT nCount = 0;
        for (INT i = 0; i < BOX_COUNT; i ++)
        {
            if (stNoteBox[i].bIsHave == TRUE && stNoteBox[i].usBoxType == BOX_STORAGE)
            {
                nCount ++;
            }
        }
        return nCount;
    }
    INT nIsRetractBox()    // 是否有回收箱(返回数目)
    {
        INT nCount = 0;
        for (INT i = 0; i < BOX_COUNT; i ++)
        {
            if (stNoteBox[i].bIsHave == TRUE && stNoteBox[i].usBoxType == BOX_RETRACT)
            {
                nCount ++;
            }
        }
        return nCount;
    }
    INT nBoxIsHave(USHORT usBoxNo, USHORT usBoxType = BOX_STORAGE) // 指定类型+箱号是否存在
    {
        for (INT i = 0; i < BOX_COUNT; i ++)
        {
            if (stNoteBox[i].bIsHave == TRUE && stNoteBox[i].usBoxNo == usBoxNo &&
                stNoteBox[i].usBoxType == usBoxType)
            {
                return BOX_ISHAVE;       // 存在
            }
        }
        return BOX_NOHAVE;  // 不存在
    }
    INT nGetBoxCount(USHORT usBoxNo, USHORT usBoxType = BOX_STORAGE)   // 获取指定类型+箱号中票据数
    {
        for (INT i = 0; i < BOX_COUNT; i ++)
        {
            if (stNoteBox[i].bIsHave == TRUE && stNoteBox[i].usBoxNo == usBoxNo &&
                stNoteBox[i].usBoxType == usBoxType)
            {
                if (stNoteBox[i].usBoxType == BOX_STORAGE && stNoteBox[i].bIsTally == FALSE)
                {
                    return 1;   // 存储箱不支持计数,始终保持1张返回
                } else
                {
                    return stNoteBox[i].usNoteCount;
                }
            }
        }
        return BOX_NOHAVE;  // 不存在
    }
    INT nGetNoteTypeIsBox(USHORT usBoxNo, USHORT usBoxType = BOX_STORAGE) // 获取指定类型+箱号中票据类型
    {
        for (INT i = 0; i < BOX_COUNT; i ++)
        {
            if (stNoteBox[i].bIsHave == TRUE && stNoteBox[i].usBoxNo == usBoxNo &&
                stNoteBox[i].usBoxType == usBoxType)
            {
                return stNoteBox[i].usNoteType;
            }
        }
        return BOX_NOHAVE;
    }
    INT nGetBoxOrder(USHORT usBoxNo, USHORT usBoxType = BOX_STORAGE)   // 获取指定类型+箱号的顺序索引(用于INI)
    {
        for (INT i = 0; i < BOX_COUNT; i ++)
        {
            if (stNoteBox[i].bIsHave == TRUE && stNoteBox[i].usBoxNo == usBoxNo &&
                stNoteBox[i].usBoxType == usBoxType)
            {
                return i;
            }
        }
        return BOX_NOHAVE;  // 不存在
    }
    INT nGetBoxStat(USHORT usBoxNo, USHORT usBoxType = BOX_STORAGE)   // 获取指定类型+箱号的状态
    {
        for (INT i = 0; i < BOX_COUNT; i ++)
        {
            if (stNoteBox[i].bIsHave == TRUE && stNoteBox[i].usBoxNo == usBoxNo &&
                stNoteBox[i].usBoxType == usBoxType)
            {
                if (usBoxType == BOX_STORAGE)   // 存储箱
                {
                    if (stNoteBox[i].bIsTally != TRUE)  // 不进行计数,存储箱始终为Full
                    {
                        return BOX_FULL;
                    } else
                    {
                        if (stNoteBox[i].usNoteCount == 0)
                        {
                            return BOX_EMPTY;
                        } else
                        if (stNoteBox[i].usNoteCount <= stNoteBox[i].usThreshold)
                        {
                            return BOX_HIGHLOW;
                        } else
                        {
                            return BOX_FULL;
                        }
                    }

                } else
                {
                    if (stNoteBox[i].usNoteCount >= stNoteBox[i].usFullThreshold)
                    {
                        return BOX_FULL;
                    } else
                    if (stNoteBox[i].usNoteCount >= stNoteBox[i].usThreshold)
                    {
                        return BOX_HIGHLOW;
                    } else
                    {
                        return BOX_EMPTY;
                    }
                }
            }
        }
        return BOX_NOHAVE;  // 不存在
    }
    INT nSetStorageCount(USHORT usBoxNo, USHORT usCnt, BOOL bIsAdd = TRUE)    // 设置存储箱计数
    {
        INT nCount = 0;
        for (INT i = 0; i < BOX_COUNT; i ++)
        {
            if (stNoteBox[i].bIsHave == TRUE && stNoteBox[i].usBoxType == BOX_STORAGE)
            {
                if (stNoteBox[i].bIsTally == TRUE)  // 支持计数(INI)
                {
                    nCount = (bIsAdd == TRUE ? stNoteBox[i].usNoteCount + usCnt : stNoteBox[i].usNoteCount - usCnt);
                    stNoteBox[i].usNoteCount = (nCount < 0 ? 0 : nCount);
                }
                return stNoteBox[i].usNoteCount;
            }
        }
        return BOX_NOHAVE;
    }
    INT nSetRetractCount(USHORT usBoxNo, USHORT usCnt, BOOL bIsAdd = TRUE)    // 设置回收箱计数
    {
        INT nCount = 0;
        for (INT i = 0; i < BOX_COUNT; i ++)
        {
            if (stNoteBox[i].bIsHave == TRUE && stNoteBox[i].usBoxType == BOX_RETRACT)
            {
                nCount = (bIsAdd == TRUE ? stNoteBox[i].usNoteCount + usCnt : stNoteBox[i].usNoteCount - usCnt);
                stNoteBox[i].usNoteCount = (nCount < 0 ? 0 : nCount);
                return stNoteBox[i].usNoteCount;
            }
        }
        return BOX_NOHAVE;
    }    
    INT nGetRetainNum(USHORT usNo = 0)    // 取第一个
    {
        for (INT i = 0; i < BOX_COUNT; i ++)
        {
            if (stNoteBox[i].bIsHave == TRUE && stNoteBox[i].usBoxType == BOX_RETRACT)
            {
                return stNoteBox[i].usBoxNo;
            }
        }
        return BOX_NOHAVE;
    }
} NOTEBOXLIST, *LPNOTEBOXLIST;

// RSC-D400M设备特殊设置
typedef
struct st_BT8500M_cfg
{
    std::string stdFontBuffer;      // 字体信息
    USHORT usUseDistAreaSupp;       // 是否使用Form/INI设置识别范围
    FLOAT  fPictureZoom;            // 图片缩放
    USHORT usDPIx;                  // X方向DPI
    USHORT usDPIy;                  // Y方向DPI
    st_BT8500M_cfg()
    {
        Clear();
    }
    void Clear()
    {
        usUseDistAreaSupp = 1;
        stdFontBuffer.clear();
        usDPIx = 0;
        usDPIy = 0;
    }
}STCFG_BT8500M, *LPSTCFG_BT8500M;

// ini获取
typedef
struct st_cpr_ini_config
{
    CHAR                    szDevDllName[256];                  // DevXXX动态库
    USHORT                  nDriverType;                        // 设备类型
    INT                     nVerifyField;                       // Form->Field检查方式
    INT                     nOpenFailRet;                       // Open失败时返回值
    USHORT                  usBank;                             // 指定银行
    CHAR                    szBankCode[12];                     // 银行别码
    CHAR                    szSDKPath[256];                     // 设备SDK库路径
    USHORT                  usResetMode;                        // 复位方式(0软复位/1硬复位,缺省0)
    STCFG_BT8500M           stCfg_BT8500M;                      // BT-8500M设备配置
    map<USHORT, std::string>       m_Map_NoteNo_GetList;        // 缺省票号获取Map
    map<USHORT, std::string>       m_Map_NotePar_GetList;       // 缺省票面获取Map
    st_cpr_ini_config()
    {
        memset(this, 0x00, sizeof(st_cpr_ini_config));
        m_Map_NoteNo_GetList.clear();
        m_Map_NotePar_GetList.clear();
    }
}STCPRINICONFIG, *LPSTCPRINICONFIG;

//-----------------处理类 定义-------------------------
class CXFS_CPR : public ICmdFunc, public CTextPrinter
{

public:
    CXFS_CPR();
    virtual~ CXFS_CPR();

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
    HRESULT StartOpen(BOOL bReConn = FALSE);// Open设备及初始化相关子处理
    void InitConifig();
    long InitStatus();                      // 初始化状态类变量
    long InitCaps();                        // 初始化能力值类变量
    void UpdateDeviceStatus();              // 状态获取
    //HRESULT ControlMedia(DWORD dwControl);  // 介质控制处理
    HRESULT InnerPrintForm(LPWFSPTRPRINTFORM pInData);  // 格式化打印处理
    long InnerReadForm(LPWFSPTRREADFORM pInData);
    long InnerReadImage(LPWFSPTRIMAGEREQUEST lpImgRequest, LPWFSPTRIMAGE *&lppImage, DWORD dwTimeOut);
    void SetStorageBoxCount(USHORT usBoxNo, USHORT usCnt, BOOL bIsAdd = TRUE);            // 设置出票箱计数并记录
    void SetRetractBoxCount(USHORT usBoxNo, USHORT usCnt, BOOL bIsAdd = TRUE);            // 设置回收箱计数并记录

    //去掉不可打印字符
    void RemoveUnPrintableChar(ULONG ulInSize, const LPBYTE pInData, ULONG &ulOutSize, LPBYTE pOutData);

protected:  // 重载函数
    // 重载CSPBaseClass的方法
    HRESULT OnInit();
    HRESULT OnExit();

    // 重载TextPrinter的方法
    virtual HRESULT ControlMedia(DWORD dwControl);      // 介质控制处理
    virtual HRESULT SendRawData(BOOL bExpectResp, ULONG nSize, LPBYTE pData);
    virtual HRESULT Reset(DWORD dwMediaControl, USHORT usBinIndex);
    virtual HRESULT EndForm(PrintContext *pContext);    // 重写该方法,处理PrintForm打印数据

    //以下是子类必须实现的函数,EndForm调用它
    virtual HRESULT PrintImage(LPCSTR szImagePath, int nDstWidth, int nDstHeight);
    virtual HRESULT PrintImageOrg(LPCSTR szImagePath, ULONG ulOrgX, ULONG ulOrgY);
    virtual long PrintData(const char *pBuffer, DWORD dwSize);
    virtual long PrintMICR(const char *pBuffer, DWORD dwSize);
    virtual HRESULT PrintString(const char *pBuffer, DWORD dwSize, BOOL bIsFromPrint = FALSE);
    virtual HRESULT AddPrintString(const char *pBuffer, DWORD dwSize, BOOL bIsFromPrint, char *pBuffOut, DWORD &dwSizeOut);
    virtual HRESULT ReadItem(PRINT_ITEM *pItem);

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
    INT ConvertErrCode(INT nRet);                  // 错误码转换为WFS格式
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
    HRESULT  GetDefFieldToJSON(LPSTR lpStr, CJsonObject &cJson);

protected:
    BOOL                    m_bNeedKeepJammedStatus;
    bool                    m_bNeedReset;
    BOOL                    m_bPaperCutted;
    BOOL                    m_bReset;
    STCPRINICONFIG          m_stConfig;
    WAIT_TAKEN_FLAG         m_WaitTaken;
    NOTEBOXLIST             m_stNoteBoxList;        // 票据箱信息    
    WFSPTRRETRACTBINS       m_stWFSRetractBin[16];  // 回收箱信息
    WFSPTRRETRACTBINS       m_stWFSRetractBinOLD[16];  // 回收箱信息

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
    CWfsPtrStatus                  m_stStatus;      // 当前状态
    CWfsPtrCaps                    m_sCaps;

private:
    DWORD   dwTakeTimeSize;
    DWORD   dwMediaWidth;
    DWORD   dwMediaHeight;
    CHAR    m_szErrCode[32];
    CHAR    m_szErrCodeDetail[1024];
    INT     m_nReConRet;

private:    // INI
    RECT    m_stMargin;   // 介质边距
};

