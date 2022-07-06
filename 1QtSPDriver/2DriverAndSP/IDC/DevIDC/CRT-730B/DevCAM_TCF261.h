#ifndef DEVCAM_TCF261_H
#define DEVCAM_TCF261_H


#include "IDevCAM.h"
#include "DevCAM.h"
#include "DevImpl_TCF261.h"
#include "QtTypeInclude.h"
#include "../../XFS_CAM/ComInfo.h"

#define LOG_NAME_DEVCAM     "DevCAM_TCF261.log"

class CDevCAM_TCF261 : public IDevCAM, public CLogManage
{

public:
    CDevCAM_TCF261();
    ~CDevCAM_TCF261();

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
    // 取状态
    virtual long GetStatus(DEVCAMSTATUS &stStatus);
    // 打开窗口(窗口句柄，动作指示:0创建窗口/1销毁窗口, X/Y坐标,窗口宽高)
    virtual long Display(DWORD hWnd, WORD wAction, WORD wX, WORD wY, WORD wWidth, WORD wHeight);
    // 拍照 传下来的字符lpData按：”FileName=C:\TEMP\TEST;Text=HELLO;TextPositionX=160;TextPositionY=120;CaptureType=0”格式
    virtual long TakePicture(WORD wCamera, LPCSTR lpData);
    // 拍照(保存文件名,水印字串，图片类型，是否连续检测)(1BASE64/2JPG/4BMP;T连续检测)
    virtual long TakePictureEx(LPSTR lpFileName, LPCSTR lpCamData, WORD wPicType, bool isContinue, WORD wTimeOut);

public:
    // 设置数据
    virtual long SetData(void *vData, WORD wDataType = 0);
    // 获取数据
    virtual long GetData(void *vData, WORD wDataType = 0);
    // 版本号(1DevCam版本/2固件版本/3其他)
    virtual void GetVersion(char* szVer, long lSize, ushort usType);


private:
    CSimpleMutex                m_cMutex;
    CQtDLLLoader<IDevCAM>       m_pDev;
    DEVCAMSTATUS                m_stStatus;

    CDevImpl_TCF261             m_devTCF261;
    ST_CAM_TCF261_INIT_PARAM    m_stCamTCF261InitParam;

    BOOL                        m_bDisplayOpenOK;                   // 是否已打开窗口
};

#endif // DEVCAM_TCF261_H
