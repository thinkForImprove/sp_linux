/***************************************************************
* 文件名称：DevIDC_CRT350N.h
* 文件描述：读卡器功能处理接口封装 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年10月20日
* 文件版本：1.0.0.1
****************************************************************/

#ifndef DEVIDC_CRT350N_H
#define DEVIDC_CRT350N_H

#include "IDevIDC.h"
#include "QtTypeInclude.h"
#include "DevImpl_CRT350N.h"
#include "../XFS_IDC/def.h"
#include "data_convertor.h"
#include "ErrorDetail.h"

#define LOG_NAME_DEV     "DevIDC_CRT350N.log"

/**************************************************************************
*
***************************************************************************/
class CDevIDC_CRT350N : public CLogManage, public IDevIDC, public ConvertVarIDC
{
public:
    CDevIDC_CRT350N(LPCSTR lpDevType);
    virtual ~CDevIDC_CRT350N();

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
    CDevImpl_CRT350N    m_pDevImpl;
    CErrorDetail        m_clErrorDet;

private:
    WORD                m_wDeviceType;                              // 设备类型
    BOOL                m_bCancelReadCard;                          // 取消读卡标记
    BOOL                m_bICActive;                                // 卡片是否处于激活(上电)中
    BOOL                m_bICPress;                                 // 卡片是否处于触点接触状态
    BOOL                m_bIsChipHave;                              // 卡是否有芯片(硬件无法检测是否有芯片时使用)
    STDEVICEOPENMODE    m_stOpenMode;                               // 设备打开方式
    CHAR                m_szResetParam[11 + 1];                     // 复位辅助参数
    WORD                m_wShutStat;                                // 闸门状态
    WORD                m_wInCardParam;                             // 进卡参数
    INT                 m_nAuxParam[64];                            // 辅助参数
    INT                 m_nTamperSensorStat;                        // 防盗钩状态
    INT                 m_nRetErrOLD[8];                            // 处理错误值保存(0:USB动态库/1:设备连接/
                                                                    //  2:设备初始化/3/4)
    BOOL                m_bIsTeaseCardProtectRun;                   // 防逗卡保护是否启动
    QTime               m_qtTeaseCardProtRunDate;                   // 记录防逗卡保护启动时间
    WORD                m_wTeaseReInCardCount;                      // 记录防逗卡进卡计数
    BOOL                m_bIsSkimmingHave;                          // 检知发现异物标记


private:
    INT AcceptMedia(MEDIA_ACTION enMediaAct, DWORD dwTimeOut);      // 进卡处理
    INT ControlLight(STGLIGHTSCONT stLignt);                        // 指示灯控制
    INT ControlBeep(STBEEPCONT stBeep);                             // 鸣响控制    
    INT SetResetParam();                                            // 设置复位/初始化辅助参数
    INT IsTeaseCardProtect();                                       // 防逗卡检测处理
    INT ConvertImplErrCode2IDC(INT nRet);                           // Impl错误码转换为IDC错误码
    INT ConvertImplErrCode2ErrDetail(INT nRet);                     // 根据Impl错误码设置错误错误码字符串
};



#endif // DEVIDC_CRT350N_H
