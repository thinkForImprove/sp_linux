/***************************************************************
* 文件名称：DevPPR_PRM.h
* 文件描述：PRM存折打印模块功能处理接口封装 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年10月20日
* 文件版本：1.0.0.1
****************************************************************/

#ifndef DEVDSR_BSD216_H
#define DEVDSR_BSD216_H

#include "IDevPTR.h"
#include "QtTypeInclude.h"
#include "DevImpl_BSD216.h"
#include "../XFS_DSR/def.h"

#define LOG_NAME_DEV     "DevDSR_BSD216.log"

class CDevDSR_BSD216 : public IDevPTR, public CLogManage
{
public:
    CDevDSR_BSD216(LPCSTR lpDevType);
    virtual ~CDevDSR_BSD216();
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
    INT     ConvertErrorCode(INT nRet);                                                 // 转换为IDevPTR返回码/错误码
    CHAR*   ConvertErrCodeToStr(INT nRet);

private:
    CSimpleMutex    m_Mutex;
    CDevImpl_BSD216    g_devImpl;
    BOOL            m_bInitOk;                              // 设备初始化结果
    BYTE            byErrCode[7];                           // 错误码
    WORD            wDevState;                              // 设备状态
    WORD            wDevType;                               // 设备类型
    CHAR            m_szErrStr[1024];
    WORD            m_wGetStatErrCount;                     // 获取状态失败次数计数
};

#endif // DEVRPR_PRM_H

