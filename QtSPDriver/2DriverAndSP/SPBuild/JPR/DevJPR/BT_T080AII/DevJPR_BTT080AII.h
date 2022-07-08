/***************************************************************
* 文件名称：DevCPR_BTT080AII.h
* 文件描述：新北洋流水打印模块功能处理接口封装 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年10月20日
* 文件版本：1.0.0.1
****************************************************************/

#ifndef DEVRPR_BTT080AII_H
#define DEVRPR_BTT080AII_H

#include "IDevPTR.h"
#include "QtTypeInclude.h"
#include "DevImpl_BTT080AII.h"
#include "../XFS_JPR/def.h"

#define LOG_NAME_DEV     "DevJPR_BTT080AII.log"

class CDevJPR_BTT080AII : public IDevPTR, public CLogManage, public ConvertVar
{
public:
    CDevJPR_BTT080AII(LPCSTR lpDevType);
    virtual ~CDevJPR_BTT080AII();
public:
    virtual void Release();                                                             // 释放接口
    virtual int Open(const char *pMode);                                                // 打开与设备的连接
    virtual int Close();                                                                // 关闭与设备的连接
    virtual int Init();                                                                 // 设备初始化
    virtual int Reset();                                                                // 设备复位
    virtual int ResetEx(MEDIA_ACTION enMediaAct, unsigned short usParam);               // 设备复位
    virtual int GetStatus(DEVPTRSTATUS &stStatus);                                      // 取设备状态
    virtual int PrintData(const char *pStr, unsigned long ulDataLen);                   // 打印字串(无指定打印坐标)
    virtual int PrintDataOrg(const char *pStr, unsigned long ulDataLen, unsigned long ulOrgX, unsigned long ulOrgY);
    virtual int PrintImage(const char *szImagePath, unsigned long ulWidth, unsigned long ulHeight); // 图片打印(无指定打印坐标)
    virtual int PrintImageOrg(const char* szImagePath, unsigned long ulOrgX, unsigned long ulOrgY); // 指定坐标打印图片
    virtual int PrintForm(DEVPRINTFORMIN stPrintIn, DEVPRINTFORMOUT &stPrintOut);       // PrintForm打印
    virtual int ReadForm(DEVPTRREADFORMIN stReadIn, DEVPTRREADFORMOUT &stReadOut);      // ReadForm获取
    virtual int ReadImage(DEVPTRREADIMAGEIN stImageIn, DEVPTRREADIMAGEOUT &stImageOut); // ReadImage获取
    virtual int MeidaControl(MEDIA_ACTION enMediaAct, unsigned short usParam = 0);      // 介质控制
    virtual int SetData(void *vData, WORD wDataType = 0);                               // 设置数据
    virtual int GetData(void *vData, WORD wDataType = 0);                               // 获取数据
    virtual void GetVersion(char* szVer, long lSize, ushort usType);                    // 获取版本

private:
    INT     SetPrintMode(DEVPTRFONTPAR stFontPar);                                      // 设置字体属性
    INT     ConvertErrorCode(INT nRet);                                                 // 转换为IDevPTR返回码/错误码

private:
    CSimpleMutex        m_cMutex;
    CDevImpl_BTT080AII  g_devImpl;
    DEVICEHANDLE    m_hPrinter;                                                         //
    BOOL            m_bInitOk;                                                          // 设备初始化结果
    BYTE            byErrCode[7];                                                       // 错误码
    WORD            wDevState;                                                          // 设备状态
    WORD            wDevType;                                                           // 设备类型

private:
    WORD            m_wFontType;                                                        // 字体类型
    WORD            m_wDouWidth;                                                        // 倍宽(1~6)
    WORD            m_wDouHeight;                                                       // 倍高(1~6)
    WORD            m_wFontStyle;                                                       // 字体属性
};

#endif // DEVRPR_BTT080AII_H

