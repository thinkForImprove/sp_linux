#ifndef DEVIDX_BSID81_H
#define DEVIDX_BSID81_H


#include "IDevIDX.h"
#include "QtTypeInclude.h"
#include "DevImpl_BSID81.h"

#define LOG_NAME_DEVIDX     "DevIDX_BSID81.log"

class CDevIDX_BSID81: public IDevIDX, public CLogManage
{

public:
    CDevIDX_BSID81();
    ~CDevIDX_BSID81();

public:
    // 释放接口
    virtual void Release();
    // 功能：打开与设备的连接
    virtual int Open(const char *pMode);
    // 设备初始化
    virtual int Reset(const int nActFlag);
    // 关闭与设备的连接
    virtual void Close();
    // 取设备状态
    virtual int GetStatus(DEVIDXSTATUS &stStatus);
    // 取卡位置
    virtual int GetCardPosition(CardPosition &enCardPos);
    // 吞卡
    virtual int RetainCard();
    // 退卡
    virtual int EjectCard();
    // 进卡
    virtual int AcceptCard(unsigned long ulTimeOut);
    // 读卡
    virtual int ReadTracks(STTRACK_INFO &stTrackInfo);
    // 设置数据
    virtual int SetData(void *vData, WORD wDataType = 0);
    // 获取数据
    virtual int GetData(void *vData, WORD wDataType = 0);

public:
    // 版本号(1DevCam版本/2固件版本/3其他)
    virtual void GetVersion(char* szVer, long lSize, ushort usType);

private: // 数据处理
    // 分类型证件数据组合
    BOOL vIDCardDataToTrack(LPVOID lpVoidData, LPSTR lpDestData, WORD wCardType);

    // BSID81错误码转换为DevIDX错误码
    LONG lErrCodeChgToIdx(LONG lDevCode);


private:
    CDevImpl_BSID81     m_pDevBSID81;   // 设备硬件调用类变量
    DEVIDXSTATUS        m_stOldStatus;  // 保存上一次设备状态
    DEVSTATUS           m_stDevStatus;
    CSimpleMutex        m_cMutex;

    CHAR                m_szHeadImgSavePath[256]; // 证件头像存放位置
    CHAR                m_szScanImgSavePath[256]; // 扫描图像存放位置
    CHAR                m_szScanImgFrontBuff[SAVE_IMG_MAX_SIZE];    // 证件扫描正面buffer
    CHAR                m_szScanImgBackBuff[SAVE_IMG_MAX_SIZE];     // 证件扫描背面buffer
    INT                 m_nScanImgFrontBuffLen;                     // 证件扫描正面buffer Len
    INT                 m_nScanImgBackBuffLen;                      // 证件扫描背面buffer Len
    WORD                m_wScanImgSaveType;                         // 证件扫描图像保存类型(BMP/JPG,缺省JPG)
    CHAR                m_szScanImgSaveTypeS[5];                    // 证件扫描图像保存类型(BMP/JPG,缺省JPG)
    FLOAT               m_fScanImgSaveZoomSc;                       // 证件扫描图像保存图片缩放比例(0.1-3.0,缺省2.0)
    WORD                m_wEjectMode;                               // 退卡动作模式(0保留在门口/1完全弹出)
};

#endif // DEVIDX_BSID81_H
