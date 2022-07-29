/***************************************************************************
* 文件名称: DevCAM_TCF261.h
* 文件描述：摄像功能处理接口封装 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年6月6日
* 文件版本：1.0.0.1
****************************************************************************/

#ifndef DEVCAM_TCF261_H
#define DEVCAM_TCF261_H


#include "IDevCAM.h"
#include "DevCAM.h"
#include "DevImpl_TCF261.h"
#include "QtTypeInclude.h"
#include "ErrorDetail.h"
#include "DevCAM_DEF/DevCAM_DEF.h"
#include "../../XFS_CAM/def.h"

#define LOG_NAME_DEV        "DevCAM_TCF261.log"

/***************************************************************************
*
***************************************************************************/
class CDevCAM_TCF261 : public IDevCAM,/*public CLogManage, public ConvertVarCAM, */
                       public CDevCAM_DEF
{
public:
    CDevCAM_TCF261(LPCSTR lpDevType);
    ~CDevCAM_TCF261();

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
    INT ConvertImplErrCode2CAM(INT nRet);                           // Impl错误码转换为CAM错误码
    INT ConvertImplErrCode2ErrDetail(INT nRet);                     // 根据Impl错误码设置错误错误码字符串
    INT ConvertImplLiveErr2Dev(INT nRet);                           // Impl错误码转换为CAM活检状态码

private:    // 重写CDevCAM_DEF类接口
    INT VideoCameraOpenFrontRun(STDISPLAYPAR stDisplayIn);          // 摄像窗口打开前处理
    INT VideoCameraOpen(STDISPLAYPAR stDisplayIn);                  // 打开设备摄像画面
    INT VideoCameraClose();                                         // 关闭设备摄像画面
    INT VideoCameraPause();                                         // 暂停设备摄像画面
    INT VideoCameraResume();                                        // 恢复设备摄像画面
    INT GetViewImage(LPSTIMGDATA lpImgData, INT nWidth = 0,
                     INT nHeight = 0, DWORD dwParam = 0);           // 获取窗口显示数据
    INT TakePicFrontRun(STTAKEPICTUREPAR stTakePicIn);              // 拍照前运行处理
    INT TakePicAfterRun(STTAKEPICTUREPAR stTakePicIn);              // 拍照后运行处理
    BOOL GetLiveDetectResult();                                     // 获取检测结果
    INT TakePicSaveImage(STTAKEPICTUREPAR stTakePicIn);             // 保存图像

private:
    CSimpleMutex                    m_cMutex;
    CDevImpl_TCF261                 m_pDevImpl;

private:
    STDEVICEOPENMODE                m_stOpenMode;                   // 设备打开方式
    STVIDEOPAMAR                    m_stVideoParam;                 // 设备摄像模式
    BOOL                            m_bReCon;                       // 是否断线重连状态
    INT                             m_nRetErrOLD[12];               // 处理错误值保存(0:Open/1:vidpid检查/
                                                                    //             2:/3:/4)
    INT                             m_nClipMode;                    // 图像镜像模式转换
    INT                             m_nFrameResoWH[2];              // 截取画面帧的分辨率(0:Width, 1:Height)
    CHAR                            m_szPersonImgFile[MAX_PATH];    // 特殊处理: 人脸图像名
    CHAR                            m_szPersonNirImgFile[MAX_PATH]; // 特殊处理: 人脸红外图像名
    STDEVCAMSTATUS                  m_stStatusOLD;                  // 记录上一次状态
    INT                             m_nImplStatOLD;                 // 保留上一次获取的Impl状态变化
    BOOL                            m_bDevPortIsHaveOLD[2];         // 记录设备是否连接上
};

#endif // DEVCAM_TCF261_H

// -------------------------------------- END --------------------------------------

