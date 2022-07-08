/***************************************************************
* 文件名称: DevCAM_ZLF1000A3.h
* 文件描述: ZL-F1000A3高拍仪模块功能处理接口封装 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2022年2月17日
* 文件版本：1.0.0.1
****************************************************************/
#ifndef DEVCAM_ZLF1000A3_H
#define DEVCAM_ZLF1000A3_H

#include "IDevCAM.h"
#include "DevCAM.h"
#include "DevImpl_ZLF1000A3.h"
#include "QtTypeInclude.h"
#include "QtTypeDef.h"
#include "../../XFS_CAM/ComInfo.h"

#define LOG_NAME_DEV     "DevCAM_ZLF1000A3.log"

#define RUNNING             0       // 运行中
#define ENDING              1       // 终止
#define PAUSE               2       // 暂停

class CDevCAM_ZLF1000A3: public IDevCAM, public CLogManage
{

public:
    CDevCAM_ZLF1000A3();
    CDevCAM_ZLF1000A3(LPCSTR lpDevType);
    ~CDevCAM_ZLF1000A3();

public:
    // 释放接口
    virtual void Release();
    // 打开连接
    virtual long Open(LPCSTR lpMode);
    // 关闭连接
    virtual long Close();
    // 复位
    virtual long Reset();
    // 读取设备信息
    virtual long GetDevInfo(char *pInfo);
    // 命令取消
    virtual long Cancel();
    // 取状态
    virtual long GetStatus(DEVCAMSTATUS &stStatus);
    // 打开窗口(窗口句柄，动作指示:0创建窗口/1销毁窗口, X/Y坐标,窗口宽高)
    virtual long Display(DWORD hWnd, WORD wAction, WORD wX, WORD wY, WORD wWidth, WORD wHeight);
    // 拍照 传下来的字符lpData按：”FileName=C:\TEMP\TEST;Text=HELLO;TextPositionX=160;TextPositionY=120;CaptureType=0”格式
    virtual long TakePicture(WORD wCamera, LPCSTR lpData);
    // 拍照(保存文件名,水印字串，图片类型，是否连续检测)(1BASE64/2JPG/4BMP;T连续检测)
    virtual long TakePictureEx(LPSTR lpFileName, LPCSTR lpCamData, WORD wPicType, bool isContinue,
                               DWORD dwTimeOut, WORD wCamara = TAKEPIC_PERSON);

public:
    // 设置数据
    virtual long SetData(void *vData, WORD wDataType = 0);
    // 获取数据
    virtual long GetData(void *vData, WORD wDataType = 0);
    // 版本号(1DevCam版本/2固件版本/3其他)
    virtual void GetVersion(char* szVer, long lSize, ushort usType);

private:
    INT     ConvertErrorCode(INT nRet);                             // 转换为IDevPTR返回码/错误码
    CHAR*   ConvertErrCodeToStr(INT nRet);
    void    Init();
    BOOL    bConnSharedMemory(LPSTR lpSharedName);                  // 创建解除共享内存
    void    bDisConnSharedMemory();                                 // 解除关联共享内存

protected:
    void    ThreadDispGetData(WORD wWidth, WORD wHeight, DWORD dwSleepTime);// Display命令窗口显示数据流获取处理

private:
    CSimpleMutex                m_cMutex;
    DEVCAMSTATUS                m_stStatus;
    BOOL                        m_bCancel;                          // 是否下发取消命令
    CDevImpl_ZLF1000A3          m_devImpl;                          // 设备控制类实例
    WORD                        m_wCamType;                         // 打开类型(文档/人物)
    BOOL                        m_bDisplayOpenOK;                   // 是否已打开窗口
    std::thread                 m_threadDispDataGet;                // Display命令窗口显示数据流获取线程实例
    ULONG                       m_ThreadDispDataGetCnt;             // Display命令窗口显示数据流获取线程启动计数
    WORD                        m_threadDispRunStat;                // Display命令窗口显示数据流获取线程状态(0终止;1执行中;2暂停)
    QSharedMemory               *m_qSharedMemData;                  // Display命令窗口显示数据流共享内存实例
    CHAR                        m_szSharedDataName[32+1];           // Display命令窗口显示数据流共享内存名
    ULONG                       m_ulSharedDataSize;                 // Display命令窗口显示数据流共享内存大小
    INT                         m_nPid;                             // 当前PID

};


#endif // DEVCAM_ZLF1000A3_H
