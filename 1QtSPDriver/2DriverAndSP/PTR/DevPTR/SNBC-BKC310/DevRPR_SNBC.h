#pragma once

#include "IDevPTR.h"
#include "QtTypeInclude.h"
#include "BKC310_DevImpl.h"
#include "BKC310Def.h"



class CDevPTR_SNBC : public IDevPTR, public CLogManage
{
public:
    CDevPTR_SNBC(LPCSTR lpDevType);
    CDevPTR_SNBC(WORD wType);
    CDevPTR_SNBC(WORD wType, LPCSTR lpDevType);
    virtual ~CDevPTR_SNBC();
public:
    virtual long Release();
    // 打开与设备的连接
    virtual long Open(const char *pMode);
    virtual long OpenDev(const unsigned short usMode);
    // 关闭与设备的连接
    virtual void Close();
    // 设备初始化
    virtual long Init();
    // 设置当前打印格式
    virtual long SetPrintFormat(const STPRINTFORMAT &stPrintFormat);
    // 打印字串(无指定打印坐标), 打印格式使用上一次调用SetPrintFormat设置的格式，如果没有设置则使用默认参数
    virtual long PrintData(const char *pStr, unsigned long ulDataLen);
    // 图片打印(无指定打印坐标)
    virtual long PrintImage(const char *szImagePath, unsigned long ulWidth, unsigned long ulHeight);
    // 指定坐标打印图片
    virtual long PrintImageOrg(const char* szImagePath, unsigned long ulOrgX, unsigned long ulOrgY);
    // 切纸, bDetectBlackStripe：是否检测黑标, ulFeedSize，切纸前走纸的长度，单位0.1毫米
    virtual long CutPaper(bool bDetectBlackStripe, unsigned long ulFeedSize);
    // 查询一次设备状态
    virtual long QueryStatus();
    // 得到打印机状态, pPrinterStatus，返回打印机状态, pPaperStatus，返回纸状态, pTonerStatus，返回TONER状态
    virtual void GetStatus(PaperStatus &PrinterStatus, TonerStatus &PaperStatus, OutletStatus &TonerStatus);
    // 设置当前打印模式, stPrintMode：定义当前打印模式的具体内容
    virtual long SetPrintMode(const STPRINTMODE &stPrintMode);
    // 页模式下传打印字串, 打印格式使用上一次调用SetPrintFormat设置的格式，如果没有设置则使用默认参数
    virtual long PrintPageTextOut(const char *pStr, unsigned long ulDataLen, unsigned long ulOrgX, unsigned long ulOrgY);
    // 页模式下传打印图片
    virtual long PrintPageImageOut(const char *szImagePath, unsigned long ulOrgX, unsigned long ulOrgY);
    // 执行页模式数据批量打印
    virtual long PrintPageData();
    // 行模式打印字串, 打印格式使用上一次调用SetPrintFormat设置的格式，如果没有设置则使用默认参数
    virtual long PrintLineData(const char *pStr, unsigned long ulDataLen, unsigned long ulOrgX = 0);
    // 获取处理错误码
    virtual char *GetErrCode();
    // 获取动态库版本
    virtual char *GetVersion();
    // 获取固件版本
    virtual bool GetFWVersion(char *szFWVer, unsigned long *ulSize);
    // 获取硬件设备指令处理返回码
    virtual unsigned long GetDevErrCode();
    // 校正标记纸的起始位置, byMakePos : 标记位置值，单位: 0.1mm，上限31.8mm
    virtual long ChkPaperMarkHeader(unsigned int uiMakePos);

private:
    BOOL GetDevStatus(LPPrintStatus lpPTRStatus);
    // 设置错误码
    virtual void SetDevErrCode(const BYTE *byErrCd);

private:
    CPossdkIntf     m_possdkIntf;   // SNBC-BKC310动态库相关
    DEVICEHANDLE    m_hPrinter;     //
    BOOL            m_bInitOk;      // 设备初始化结果
    BYTE            byErrCode[7];   // 错误码
    WORD            wDevState;      // 设备状态
    PaperStatus     m_ePaperStatus; // 打印纸状态
    TonerStatus     m_eTonerStatus; // TONER状态
    OutletStatus    m_eOutletStatus;// 出纸口状态
    WORD            wDevType;       // 设备类型
};
