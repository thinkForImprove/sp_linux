#ifndef DEVCAM_CLOUDWALK_H
#define DEVCAM_CLOUDWALK_H


#include <QWidget>
#include <QMainWindow>
#include <QTimer>
#include <QThread>
#include <qthread.h>
#include <QSharedMemory>
#include <QProcess>

#include <qprocess.h>

#include "IDevCAM.h"
#include "DevImpl_CloudWalk.h"
#include "QtTypeInclude.h"
#include "qsharedmemory.h"
#include "../../XFS_CAM/ComInfo.h"

#define LOG_NAME_DEVCAM     "DevCAM_CloudWalk.log"

#define CAMERA_OPEN 0
#define CAMERA_CLOSE 1

class CDevCAM_CloudWalk:
        public IDevCAM,
        public CDevImpl_CloudWalk
{

public:
    CDevCAM_CloudWalk();
    ~CDevCAM_CloudWalk();

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

public: // 摄像图框及图像获取函数
    void preview(const uchar* data, int width, int height, int channels, int type = 0);
    void showFace(const long errCode, const long timestamp, const CWLiveFaceDetectInformation_t* pFaceInformations, int nFaceNumber);
    void showLive(int errCode, const float* scores, const float* distances, int nFaceNumber);

    void vWriteImageInfo(LPSTR lpImageFile);
    BOOL bPathCheckAndCreate(LPSTR lpsPath, BOOL bFolder);
    BOOL bWriteFile(LPSTR szPath, LPSTR szData, int iDataSize);
    BOOL bConnSharedMemory(LPSTR lpSharedName);
    void bDisConnSharedMemory();

private:
    CSimpleMutex                m_cMutex;
    CQtDLLLoader<IDevCAM>       m_pDev;
    DEVCAMSTATUS                m_stStatus;

    CHAR                        m_pSaveFileName[MAX_PATH];
    WORD                        m_wSaveFileType;
    BOOL                        m_bIsTakePicExStop;                 // TakePic命令是否已完成

    BOOL                        m_bDisplayOpenOK;                   // 是否已打开窗口

    QSharedMemory               *m_qSharedMemData;
    CHAR                        m_szSharedDataName[32+1];
    ULONG                       m_ulSharedDataSize;

    ST_CAM_OPENTYPE             m_stCamOpenType;
    ST_CAM_CW_INIT_PARAM        m_stCamCwInitParam;
    ST_CAM_SAVEIMAGE_CFG        m_stCamSaveImageCfg;
    INT                         m_nRetErrOLD[8];                    // 处理错误值保存
    DEVCAMSTATUS                m_DevStatusOLD;                     // 记录上一次设备状态


    WORD                        wBankNo;
};

#endif // DEVCAM_CLOUDWALK_H
