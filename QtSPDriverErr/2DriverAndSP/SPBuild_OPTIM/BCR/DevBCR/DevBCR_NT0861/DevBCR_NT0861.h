/**************************************************************************
* 文件名称：DevBCR_NT0861.h
* 文件描述：条码阅读功能处理接口封装 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年7月13日
* 文件版本：1.0.0.1
**************************************************************************/

#ifndef DEVBCR_NT0861_H
#define DEVBCR_NT0861_H

#include "IDevBCR.h"
#include "QtTypeInclude.h"
#include "DevImpl_NT0861.h"
#include "../XFS_BCR/def.h"
#include "data_convertor.h"
#include "ErrorDetail.h"

#define LOG_NAME_DEV     "DevBCR_NT0861.log"

/**************************************************************************
*
***************************************************************************/
class CDevBCR_NT0861 : public CLogManage, public IDevBCR, public ConvertVarBCR
{
public:
    CDevBCR_NT0861(LPCSTR lpDevType);
    virtual ~CDevBCR_NT0861();

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
    virtual int Reset(unsigned short usParam = 0);
    // 取设备状态
    virtual int GetStatus(STDEVBCRSTATUS &stStatus);
    // 扫码
    virtual int ReadBCR(STREADBCRIN stReadIn, STREADBCROUT &stReadOut);
    // 设置数据
    virtual int SetData(unsigned short usType, void *vData = nullptr);
    // 获取数据
    virtual int GetData(unsigned short usType, void *vData);
    // 获取版本
    virtual int GetVersion(unsigned short usType, char* szVer, int nSize);

private:
    CSimpleMutex        m_MutexAction;
    CDevImpl_NT0861     m_pDevImpl;
    CErrorDetail        m_clErrorDet;

private:
    WORD                m_wDeviceType;                              // 设备类型
    BOOL                m_bCancelReadBcr;                           // 取消扫码标记
    STDEVICEOPENMODE    m_stOpenMode;                               // 设备打开方式
    INT                 m_nRetErrOLD[8];                            // 处理错误值保存(0:USB动态库/1:设备连接/
                                                                    //  2:设备初始化/3取扫码数据/4)
    INT                 m_nDevStatOLD;                              // 上一次记录设备状态
    BOOL                m_bIsReadBcrRunning;                        // 是否处于读码中

private:
    INT ControlLight(STGLIGHTSCONT stLignt);                        // 指示灯控制
    INT ControlBeep(STBEEPCONT stBeep);                             // 鸣响控制
    INT ConvertImplErrCode2BCR(INT nRet);                           // Impl错误码转换为BCR错误码
    INT ConvertImplErrCode2ErrDetail(INT nRet);                     // 根据Impl错误码设置错误错误码字符串
    WORD ConvertSymImpl2Dev(INT nSym);                              // 条码类型Impl转换为Dev格式
    WORD ConvertSymDev2Impl(INT nSym);                              // 条码类型Dev转换为Impl格式
};


#endif // DEVBCR_NT0861_H

// -------------------------------- END -----------------------------------

