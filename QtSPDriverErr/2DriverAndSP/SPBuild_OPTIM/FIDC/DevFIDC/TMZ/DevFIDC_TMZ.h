/***************************************************************
* 文件名称：DevFIDC_TMZ.h
* 文件描述：非接功能处理接口封装 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年10月20日
* 文件版本：1.0.0.1
****************************************************************/

#ifndef DEVFIDC_TMZ_H
#define DEVFIDC_TMZ_H

#include "IDevIDC.h"
#include "QtTypeInclude.h"
#include "DevImpl_TMZ.h"
#include "../XFS_FIDC/def.h"

#define LOG_NAME_DEV     "DevFIDC_TMZ.log"

//平安叠卡检测
#define  CXFS_FIDC_MULTICARD                (-0x3009)
//////////////////////////////////////////////////////////////////////////
class CDevFIDC_TMZ : public CLogManage, public IDevIDC, public ConvertVarIDC
{
public:
    CDevFIDC_TMZ(LPCSTR lpDevType);
    virtual ~CDevFIDC_TMZ();

public:
    // 释放接口
    virtual int Release();
    // 打开与设备的连接
    virtual int Open(const char *pMode);
    // 关闭与设备的连接
    virtual int Close();
    // 取消
    virtual int Cancel(unsigned short usMode = 0);
    // 设备复位
    virtual int Reset(MEDIA_ACTION enMediaAct, unsigned short usParam = 0);
    // 取设备状态
    virtual int GetStatus(STDEVIDCSTATUS &stStatus);
    // 介质控制
    virtual int MediaControl(MEDIA_ACTION enMediaAct, unsigned long ulParam = 0);
    // 介质读写
    virtual int MediaReadWrite(MEDIA_RW_MODE enRWMode, STMEDIARW &stMediaData);
    // 芯片读写
    virtual int ChipReadWrite(CHIP_RW_MODE enChipMode, STCHIPRW &stChipData);
    // 设置数据
    virtual int SetData(unsigned short usType, void *vData = nullptr);
    // 获取数据
    virtual int GetData(unsigned short usType, void *vData);
    // 获取版本
    virtual int GetVersion(unsigned short usType, char* szVer, int nSize);

private:
    CSimpleMutex        m_MutexAction;
    CDevImpl_TMZ        m_pDevImpl;

private:
    BOOL                m_bCancelReadCard;                          // 取消读卡标记
    BOOL                m_bICActive;                                // 卡片是否处于激活中

private:
    INT AcceptMedia(DWORD dwTimeOut);                               // 等待放卡处理
    INT ControlLight(STGLIGHTSCONT stLignt);                        // 指示灯控制
    INT ControlBeep(STBEEPCONT stBeep);                             // 鸣响控制
    INT ConvertCode_Impl2IDC(INT nRet);                                   // Impl错误码转换为IDC错误码
};



#endif // DEVFIDC_MT50_H
