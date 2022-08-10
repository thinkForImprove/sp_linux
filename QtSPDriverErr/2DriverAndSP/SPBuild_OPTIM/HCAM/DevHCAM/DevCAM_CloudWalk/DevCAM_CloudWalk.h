#ifndef DEVCAM_CLOUDWALK_H
#define DEVCAM_CLOUDWALK_H


#include <QTimer>
#include <QThread>
#include <qthread.h>
#include <QSharedMemory>
#include <QProcess>

#include <qprocess.h>

#include "IDevCAM.h"
#include "DevCAM.h"
#include "DevImpl_CloudWalk.h"
#include "QtTypeInclude.h"
#include "qsharedmemory.h"
#include "../../XFS_CAM/ComInfo.h"

#define LOG_NAME_DEVCAM     "DevCAM_CloudWalk.log"

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
    // 命令取消
    virtual long Cancel();		// 30-00-00-00(FT#0031)
    // 取状态
    virtual long GetStatus(DEVCAMSTATUS &stStatus);
    // 打开窗口(窗口句柄，动作指示:0创建窗口/1销毁窗口, X/Y坐标,窗口宽高)
    virtual long Display(DWORD hWnd, WORD wAction, WORD wX, WORD wY, WORD wWidth, WORD wHeight);
    // 拍照 传下来的字符lpData按：”FileName=C:\TEMP\TEST;Text=HELLO;TextPositionX=160;TextPositionY=120;CaptureType=0”格式
    virtual long TakePicture(WORD wCamera, LPCSTR lpData);
    // 拍照(保存文件名,水印字串，图片类型，是否连续检测)(1BASE64/2JPG/4BMP;T连续检测)
    virtual long TakePictureEx(LPSTR lpFileName, LPCSTR lpCamData, WORD wPicType, bool isContinue,
                               DWORD dwTimeOut, WORD wCamara = TAKEPIC_PERSON);// 30-00-00-00(FT#0031)

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
    BOOL showLiveToImage(CWLiveImage live, LPSTR lpSaveFileName, WORD wSaveType);       // 活体检测结果生成图像

    void vWriteImageInfo(LPSTR lpImageFile);
    BOOL bPathCheckAndCreate(LPSTR lpsPath, BOOL bFolder);
    BOOL bWriteFile(LPSTR szPath, LPSTR szData, int iDataSize);
    BOOL bConnSharedMemory(LPSTR lpSharedName);
    void bDisConnSharedMemory();
    BOOL bGetbEnumReso();

private:
    CSimpleMutex                m_cMutex;
    CQtDLLLoader<IDevCAM>       m_pDev;
    DEVCAMSTATUS                m_stStatus;

    WORD                        m_wTakePicMode;                     // TakePic摄像模式(人脸/全景)
    WORD                        m_wTakePicMode_Set;                 // TakePic摄像模式(人脸/全景):SetData()设置
    CHAR                        m_pSaveFileName[MAX_PATH];          // 人脸摄像保存文件名
    CHAR                        m_pSaveFileNameRoom[MAX_PATH];      // 全景摄像保存文件名
    WORD                        m_wSaveFileType;
    BOOL                        m_bIsTakePicExStop;                 // TakePic命令是否已完成
    BOOL                        m_bCancel;                          // 是否下发取消命令 // 30-00-00-00(FT#0031)

    BOOL                        m_bDisplayOpenOK;                   // 是否已打开窗口

    QSharedMemory               *m_qSharedMemData;
    CHAR                        m_szSharedDataName[32+1];
    ULONG                       m_ulSharedDataSize;

    ST_CAM_CW_INIT_PARAM        m_stCamCwInitParam;
    ST_CAM_SAVEIMAGE_PAB        m_stCamSaveImagePab;
    ST_CAM_IMAGE_MOTHOD         m_stCamImageMothod;

    CWSize                      m_stDisplayReso;                    // Display命令指定分辨率
    CWSize                      m_TakePicFileReso;                  // 资料补拍分辨率(云从)

    WORD                        wBankNo;
    DEVCAMSTATUS                m_DevStatusOLD;                     // 记录上一次设备状态
    INT                         m_nRetErrOLD[8];                    // 处理错误值保存
    WORD                        m_wViewMode;                        // 图像绘制标记(0无动作/1暂停,缺省0)

    WORD                        m_wLiveStat;                        // 活检状态值
};

#endif // DEVCAM_CLOUDWALK_H
