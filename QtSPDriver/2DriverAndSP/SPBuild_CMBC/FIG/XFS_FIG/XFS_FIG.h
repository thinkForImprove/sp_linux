#pragma once

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <mutex>
#include <stack>
#include "QtTypeInclude.h"
#include "QtTypeDef.h"
#include "IDevFIG.h"
#include "FIGData.h"
#include "ISPBaseFIG.h"

struct STFIGConfig
{
    WORD                 wNeedSetUp;
    DWORD                dwTotalTimeOut;
    WORD                 usImageSwap;
    DWORD                dwUpdateTime;
    CHAR                 pszFeatureDataPath[MAX_PATH];
    WORD                 wDeviceType;
    WORD                 wBASE64Mode;
    WORD                 wReturnOpenVal;

    STFIGConfig() { Clear(); }
    void Clear() { memset(this, 0x00, sizeof(STFIGConfig)); }
};

class CXFS_FIG : public ICmdFunc, public CLogManage, public CStlOneThread
{
public:
    CXFS_FIG();
    virtual~ CXFS_FIG();

public:
    // 开始运行SP
    long StartRun();

    // 基本接口
    virtual HRESULT OnOpen(LPCSTR lpLogicalName);
    virtual HRESULT OnClose();
    virtual HRESULT OnStatus();
    virtual HRESULT OnWaitTaken();
    virtual HRESULT OnCancelAsyncRequest();
    virtual HRESULT OnUpdateDevPDL();

    // FIG类型接口
    // INFOR
    virtual HRESULT GetStatus(LPWFSPTRSTATUS &lpStatus);
    virtual HRESULT GetCapabilities(LPWFSPTRCAPS &lpCaps);

    // EXECUTE
    virtual HRESULT ReadImage(const LPWFSPTRIMAGEREQUEST lpImgRequest, LPWFSPTRIMAGE *&lppImage, DWORD dwTimeOut);
    virtual HRESULT Reset(const LPWFSPTRRESET lpReset);

    // 上报手指移开事件线程
    virtual void Run();
public:
    // Fire消息
    void FireHWEvent(DWORD dwHWAct, char *pErr);
    void FireStatusChanged(WORD wStatus);
    void FireNoMedia(LPCSTR szPrompt);
    void FireFingerPressed();
    void FireFingerPresented(WFSPTRMEDIAPRESENTED MediaPresented);
    void FireFieldError(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure);
    void FireFieldWarning(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure);
    void FireMediaTaken();
    void FireInkThreshold(WORD wStatus);
    void FireLampThreshold(WORD wStatus);
    void FireMediaDetected(WORD wPos, USHORT BinNumber);

protected:
    // 读INI
    int InitConifig();
    // 状态初始化
    void InitStatus();
    // 能力值初始化
    void InitCaps();
    // 加载设备库
    bool LoadDevDll(LPCSTR ThisModule);
    // 状态更新子处理
    int UpdateDevStatus(int nRet);
    // 更新状态
    void UpdateStatus();
    // 数据写入bmp图片
    int WriteBMP(LPSTR file, LPBYTE Input, int BMP_X, int BMP_Y);
    // 数据保存
    int SaveDataToImage(LPBYTE pszFeatureData, LPCSTR filename, int nLen);
    // 数据读取
    int ReadFeatureData(LPBYTE pszFingerData, LPCSTR filename, int nLen);
    // BASE64编码
    std::string BASE64Encode(const unsigned char* Data,int DataByte);
    // BASE64解码
    std::string BASE64Decode(const char* Data,int DataByte,int& OutByte);
    // 检测图像文件是否存在,存在返回TRUE
    BOOL isImgFileExists(LPSTR filename);
    // 读取图像文件获取特征数据
    int GetFeaFromFile(LPSTR filename, LPBYTE pszBmpData, LPBYTE pFea, int nLen);
    // 获取图片文件名后缀
    std::string getStringLastNChar(std::string str, ULONG lastN);
    // 监控手指按下线程函数
    void *ThreadFingerPressed();

    // ReadImage 日志相关函数
    LPCSTR GetImageSource(WORD ImageSource);
    LPCSTR GetDataSrcStatus(WORD DataSrcStatus);
    LPCSTR GetDataSize(DWORD Size);
    LPCSTR GetData(LPBYTE pData, DWORD size);

protected:
    BOOL                           m_bOpen;                        // 设备打开标志
    BOOL                           m_bReset;                       // 设备复位标志
    BOOL                           m_bIsFireTakenEvent;            // 是否发送手指移走事件标志
    BOOL                           m_bRetryOpen;                   // 是否重连
    BYTE                           m_cFilePath[MAX_PATH];          // 保存图像路径
    int                            m_iRawDataLen;                  // 图像数据大小
    int                            m_iBmpDataLen;                  // BMP 图像数据大小
    FINGERPRESSEDMODE              m_bIsPressed;                   // 指纹是否按下
    char                           m_szDevType[MAX_EXT];           // 设备型号
    BOOL                           m_bIsDevBusy;                   // 设备是否正在读取指纹
private:
    CQtDLLLoader<IDevFIG>          m_pFinger;
    CQtDLLLoader<ISPBaseFIG>       m_pBase;
    CXfsRegValue                   m_cXfsReg;
    std::string                    m_strLogicalName;
    std::string                    m_strSPName;
    CSimpleMutex                  *m_pMutexGetStatus;
private:
    stack<LASTSUCCESSFINGERMODE>   m_skLastSuccMode;                // 栈功能：存放标志位
    stack<LPBYTE>                  m_stFeatureData;                 // 栈功能: 存放采集特征数据
    BYTE*                          m_pszFeaData;                    // 功能: 采集特征数据
    BOOL                           m_bisNextMatch;                  // 标志位: 是否是下一次对比
protected:
    STFIGConfig                    m_stConfig;
    CExtraInforHelper              m_cExtra;
    CWfsFigStatus                  m_sStatus;
    WFSPTRSTATUS                   m_stStatusOld;
    CWfsFigCaps                    m_cCaps;
    STFPRVERSION                   stFprVersion;                    // 版本结构
    CAutoEvent                     m_SemCancel;

    //std::thread                    m_thFingerPressed;               // 手指按压线程
    std::mutex                     m_LockStatus;

public:
    // 数据成员
    CSPReadImageOut                 m_pData;
    //CWFSPTRIMAGEHelper             m_pData;
};
