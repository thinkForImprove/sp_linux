/***************************************************************
* 文件名称: DevIDX_BSID81.h
* 文件描述: 身份证模块功能处理接口封装 头文件
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2023年3月25日
* 文件版本: 1.0.0.1
****************************************************************/

#ifndef DEVIDX_BSID81_H
#define DEVIDX_BSID81_H


#include "IDevIDC.h"
#include "QtTypeInclude.h"
#include "DevImpl_BSID81.h"
#include "../XFS_IDX/def.h"

#define LOG_NAME_DEV     "DevIDX_BSID81.log"

/**************************************************************************
*
***************************************************************************/
class CDevIDX_BSID81: public IDevIDC, public CLogManage, public ConvertVarIDC
{

public:
    CDevIDX_BSID81();
    ~CDevIDX_BSID81();

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
    //virtual int ChipReadWrite(CHIP_RW_MODE enChipMode, STCHIPRW &stChipData);
    // 设置数据
    virtual int SetData(unsigned short usType, void *vData = nullptr);
    // 获取数据
    virtual int GetData(unsigned short usType, void *vData);
    // 获取版本
    virtual int GetVersion(unsigned short usType, char* szVer, int nSize);

private:
    INT nEjectCard(DWORD dwParam);                                  // 退卡处理
    INT AcceptMedia(DWORD dwTimeOut);                               // 等待放卡处理

private: // 数据处理
    // 分类型证件数据组合
    BOOL vIDCardDataToTrack(LPVOID lpVoidData, LPSTR lpDestData, WORD wCardType);
    BOOL vIDCardDataToTrack_CSBC(LPVOID lpVoidData, LPSTR lpDestData, WORD wCardType);  // 长沙银行数据处理
    BOOL vIDCardDataToTrack_SXXH(LPVOID lpVoidData, LPSTR lpDestData, WORD wCardType);  // 陕西信合数据处理 // 40-00-00-00(FT#0010)

    INT ConvertCode_Impl2IDC(INT nRetCode);                               // Impl错误码转换为IDC错误码


private:
    CDevImpl_BSID81     m_pDevImpl;                                 // 设备硬件调用类变量
    STDEVIDCSTATUS      m_stStatusOLD;                              // 保存上一次设备状态
    DEVSTATUS           m_stDevStatus;
    CSimpleMutex        m_cMutex;
    BOOL                m_bReCon;                                   // 是否断线重连状态
    WORD                m_wBankNo;                                  // 银行
    CHAR                m_szHeadImgSavePath[256];                   // 证件头像存放位置
    CHAR                m_szScanImgSavePath[256];                   // 扫描图像存放位置
    CHAR                m_szHeadImgName[256];                       // 头像名(空不使用)
    CHAR                m_szScanImgFrontName[256];                  // 证件正面图像名(空不使用)
    CHAR                m_szScanImgBackName[256];                   // 证件背面图像名(空不使用)
    CHAR                m_szScanImgFrontBuff[SAVE_IMG_MAX_SIZE];    // 证件扫描正面buffer
    CHAR                m_szScanImgBackBuff[SAVE_IMG_MAX_SIZE];     // 证件扫描背面buffer
    INT                 m_nScanImgFrontBuffLen;                     // 证件扫描正面buffer Len
    INT                 m_nScanImgBackBuffLen;                      // 证件扫描背面buffer Len
    WORD                m_wScanImgSaveType;                         // 证件扫描图像保存类型(BMP/JPG,缺省JPG)
    CHAR                m_szScanImgSaveTypeS[5];                    // 证件扫描图像保存类型(BMP/JPG,缺省JPG)
    FLOAT               m_fScanImgSaveZoomSc;                       // 证件扫描图像保存图片缩放比例(0.1-3.0,缺省2.0)
    WORD                m_wScanImageFrontIsInfor;                   // 证件扫描图像是否以人像信息面为正面
    BOOL                m_bCancelReadCard;                          // 取消读卡标记
};

#endif // DEVIDX_BSID81_H
