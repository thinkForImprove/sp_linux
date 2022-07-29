/***************************************************************************
* 文件名称: DevCAM_DEF.h
* 文件描述：摄像功能设备功能处理共通类 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年6月6日
* 文件版本：1.0.0.1
****************************************************************************/

#ifndef DEVCAM_DEF_H
#define DEVCAM_DEF_H

#include <QTimer>
#include <QSharedMemory>

#include <unistd.h>
#include "QtTypeInclude.h"
#include "IDevCAM.h"
#include "ErrorDetail.h"
#include "../../XFS_CAM/def.h"

#include "DevImpl_DEF.h"


/***************************************************************************
// 无分类　宏定义
***************************************************************************/
// 窗口打开模式
#define DISP_OPEN_MODE_SDK      1       // SDK自建窗口
#define DISP_OPEN_MODE_THREAD   2       // 图像数据线程处理方式


/***************************************************************************
*
***************************************************************************/
class CDevCAM_DEF : public CLogManage, public ConvertVarCAM
{

public:
    CDevCAM_DEF();
    ~CDevCAM_DEF();

public:     // 非必要不重写: 窗口显示和拍照处理()
    // 摄像窗口处理(stDisplayIn: 入参参数, vParam: 其他参数)
    virtual INT InnerDisplay(STDISPLAYPAR stDisplayIn, void *vParam = nullptr);
    // 摄像拍照处理(stTakePicIn: 入参参数, vParam: 其他参数)
    virtual INT InnerTakePicture(STTAKEPICTUREPAR stTakePicIn, void *vParam = nullptr);

public:     // 可重写:共享内存建立/销毁
    virtual INT ConnSharedMemory(LPSTR lpSharedName);               // 连接共享内存
    virtual INT DisConnSharedMemory();                              // 断开共享内存

public:     // 可重写: 窗口显示和拍照处理内接口, 根据不同设备进行重载调用对应的设备接口处理
    virtual INT VideoCameraOpenFrontRun(STDISPLAYPAR stDisplayIn);  // 摄像窗口打开前处理
    virtual INT VideoCameraOpen(STDISPLAYPAR stDisplayIn);          // 打开设备摄像画面
    virtual INT VideoCameraClose();                                 // 关闭设备摄像画面
    virtual INT VideoCameraPause();                                 // 暂停设备摄像画面
    virtual INT VideoCameraResume();                                // 恢复设备摄像画面
    virtual INT GetViewImage(LPSTIMGDATA lpImgData, INT nWidth = 0,
                     INT nHeight = 0, DWORD dwParam = 0);           // 获取窗口显示数据
    virtual INT TakePicFrontRun(STTAKEPICTUREPAR stTakePicIn);      // 拍照前运行处理
    virtual INT TakePicAfterRun(STTAKEPICTUREPAR stTakePicIn);      // 拍照后运行处理
    virtual BOOL GetLiveDetectResult();                             // 获取检测结果
    virtual INT TakePicSaveImage(STTAKEPICTUREPAR stTakePicIn);     // 保存图像

public:
    INT DEF_ConvertImplErrCode2CAM(INT nRet);                       // Impl共同定义错误码转换为CAM错误码
    INT DEF_ConvertImplErrCode2ErrDetail(INT nRet);                 // 根据Impl共同定义错误码设置错误错误码字符串

public:
    EN_DISPLAY_STAT                 m_enDisplayStat;                // Display命令状态
    BOOL                            m_bCancel;                      // 是否下发取消命令
    INT                             m_nSaveStat[12];                // 保存必要的状态信息(0:命令值,)
    QSharedMemory                   *m_qSharedMemData;              // 共性内存句柄
    CHAR                            m_szSharedDataName[32+1];       // 共享内存名
    ULONG                           m_ulSharedDataSize;             // 共享内存大小
    INT                             m_nRefreshTime;                 // 摄像数据获取间隔
    CErrorDetail                    m_clErrorDet;                   // 错误码处理类实例
    WORD                            m_wDisplayOpenMode;             // 窗口打开模式
    INT                             m_nDisplayGetVideoMaxErrCnt;    // display采用图像帧方式刷新时,取图像帧数据接口错误次数上限

public:    // 线程处理
    std::thread                     m_thRunDisplay;                 // 窗口处理线程句柄
    INT                             m_nThreadRet;                   // 线程返回值

private:
    void    ThreadRunDisplay(WORD wWidth, WORD wHeight, DWORD dwRefreshTime = 10);            // 窗口处理进程

};

#endif // DEVCAM_DEF_H

// -------------------------------------- END --------------------------------------
