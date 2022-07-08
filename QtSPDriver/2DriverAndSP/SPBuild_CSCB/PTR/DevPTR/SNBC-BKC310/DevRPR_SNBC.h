#ifndef DEVRPR_SNBC_H
#define DEVRPR_SNBC_H

#include "IDevPTR.h"
#include "DevRPR.h"
#include "QtTypeInclude.h"
#include "DevImpl_BKC310.h"
#include "BKC310Def.h"
#include "../XFS_PTR/def.h"

#define LOG_NAME_BKC310     "DevCPR_BKC310.log"
#define LOG_NAME_BTNH80     "DevCPR_BTNH80.log"

class CDevPTR_SNBC : public IDevPTR, public CLogManage
{
public:
    CDevPTR_SNBC(LPCSTR lpDevType);
    virtual ~CDevPTR_SNBC();
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
    virtual int ReadForm(DEVPTRREADFORMIN stReadIn, DEVPTRREADFORMOUT &stReadOut);      // ReadForm获取
    virtual int ReadImage(DEVPTRREADIMAGEIN stImageIn, DEVPTRREADIMAGEOUT &stImageOut); // ReadImage获取
    virtual int MeidaControl(MEDIA_ACTION enMediaAct, unsigned short usParam = 0);      // 介质控制
    virtual int SetData(void *vData, WORD wDataType = 0);                               // 设置数据
    virtual int GetData(void *vData, WORD wDataType = 0);                               // 获取数据
    virtual void GetVersion(char* szVer, long lSize, ushort usType);                    // 获取版本

private:
    INT     ConvertErrorCode(INT nRet);                     // 转换为IDevPTR返回码/错误码
    CHAR*   ConvertErrCodeToStr(INT nRet);

private:
    CDevImpl_BKC310  g_devBKC310Impl;
    DEVICEHANDLE    m_hPrinter;     //
    BOOL            m_bInitOk;      // 设备初始化结果
    BYTE            byErrCode[7];   // 错误码
    WORD            wDevState;      // 设备状态
    WORD            wDevType;      // 设备类型
};

#endif // DEVRPR_SNBC_H

