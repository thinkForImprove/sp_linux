/***************************************************************
* 文件名称: DevHCAM_JDY5001A0809.h
* 文件描述：摄像功能处理接口封装 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年6月6日
* 文件版本：1.0.0.1
****************************************************************/

#ifndef DEVHCAM_JDY5001A0809_H
#define DEVHCAM_JDY5001A0809_H

#include <QTimer>
#include <QSharedMemory>

#include "QtTypeInclude.h"
#include "IDevCAM.h"
#include "ErrorDetail.h"
#include "DevImpl_JDY5001A0809.h"
#include "../../XFS_HCAM/def.h"

#define LOG_NAME_DEV     "DevHCAM_JDY5001A0809.log"

/**************************************************************************
*
***************************************************************************/
class CDevHCAM_JDY5001A0809 : public IDevCAM, public CLogManage, public ConvertVarCAM
{

public:
    CDevHCAM_JDY5001A0809(LPCSTR lpDevType);
    ~CDevHCAM_JDY5001A0809();

public:
    // 打开连接
    virtual int Open(LPCSTR lpMode);
    // 关闭连接
    virtual int Close();
    // 复位
    virtual int Reset();
    // 命令取消
    virtual int Cancel(unsigned short usMode);
    // 取状态
    virtual int GetStatus(STDEVCAMSTATUS &stStatus);
    // 摄像窗口处理(stDisplayIn: 入参参数, vParam: 其他参数)
    virtual int Display(STDISPLAYPAR stDisplayIn, void *vParam = nullptr);
    // 摄像拍照处理(stTakePicIn: 入参参数, vParam: 其他参数)
    virtual int TakePicture(STTAKEPICTUREPAR stTakePicIn, void *vParam = nullptr);
    // 设置数据
    virtual int SetData(unsigned short usType, void *vData = nullptr);
    // 获取数据
    virtual int GetData(unsigned short usType, void *vData);
    // 取版本号
    virtual int GetVersion(unsigned short usType, char* szVer, int nSize);


private:
    INT SetVideoMode(STVIDEOPAMAR stVM);                            // 设置摄像参数
    INT ConnSharedMemory(LPSTR lpSharedName);                       // 连接共享内存
    INT DisConnSharedMemory();                                      // 断开共享内存
    INT ConvertImplErrCode2CAM(INT nRet);                           // Impl错误码转换为CAM错误码
    INT ConvertImplErrCode2ErrDetail(INT nRet);                     // 根据Impl错误码设置错误错误码字符串

private:
    CSimpleMutex                    m_cMutex;
    CDevImpl_JDY5001A0809           m_pDevImpl;

private:
    WORD                            m_wDeviceType;                  // 设备类型
    STDEVICEOPENMODE                m_stOpenMode;                   // 设备打开方式
    STVIDEOPAMAR                    m_stVideoParam;                 // 设备摄像模式
    EN_DISPLAY_STAT                 m_enDisplayStat;                // Display命令状态
    CErrorDetail                    m_clErrorDet;                   // 错误码处理类实例
    BOOL                            m_bReCon;                       // 是否断线重连状态
    QSharedMemory                   *m_qSharedMemData;              // 共性内存句柄
    CHAR                            m_szSharedDataName[32+1];       // 共享内存名
    ULONG                           m_ulSharedDataSize;             // 共享内存大小
    BOOL                            m_bCancel;                      // 是否下发取消命令
    INT                             m_nRefreshTime;                 // 摄像数据获取间隔
    INT                             m_nDevStatOLD;                  // 保留上一次状态变化
    INT                             m_nSaveStat[12];                // 保存必要的状态信息(0:命令值,)
    INT                             m_nRetErrOLD[12];               // 处理错误值保存(0:USB动态库/1:设备连接/
                                                                    //             2:设备初始化/3/4)
    INT                             m_nClipMode;                    // 图像镜像模式转换
private:    // 线程处理
    std::thread                     m_thRunDisplay;                 // 窗口处理线程句柄
    INT                             m_nThreadRet;                   // 线程返回值

private:
    void    ThreadRunDisplay(WORD wWidth, WORD wHeight);            // 窗口处理进程

};

#endif // DEVHCAM_JDY5001A0809_H
