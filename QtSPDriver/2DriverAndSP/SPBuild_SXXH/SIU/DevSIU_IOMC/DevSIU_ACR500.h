/****************************************************************************
 *
 * SIU ACR500控制版设备类
 * **************************************************************************/

#ifndef DEVSIU_ACR500_H
#define DEVSIU_ACR500_H

#include "IAllDevPort.h"
#include "QtTypeInclude.h"
#include "IDevSIU.h"
#include "QtTypeInclude.h"

#define ACR500_TIMEOUT  5000
#define ACR500_ACK      0x06
#define ACR500_NAK      0xE1
#define ACR500_BCCERROR 0xE2
#define ACR500_REPLY_LEN   0x12
#define ACR500_LED_NUM  7

//灯闪烁结构体定义
typedef struct __ACRLDEFLASH{
    ULONG ulNextCheckTime;                     //下次执行时间，单位ms
    bool  bOnFlag;                             //true:当前亮 false:当前灭
    ULONG ulOnTime;                            //亮时间
    ULONG ulOffTime;                           //灭时间
}ACRLEDFLASH, *LPACRLEDFLASH;

class CDevSIU_ACR500 : public CLogManage
{
public:
    CDevSIU_ACR500();
    ~CDevSIU_ACR500();

public:
    //打开设备
    bool Open(const char *pCommParam, bool bFlashSoftwareControl = true);
    //关闭设备
    bool Close();
    //复位设备
    bool Reset();
    //设置灯
    bool SetGuidLight(int iLightIndex, int iLightStatus);
    //获取灯的当前状态
    bool GetGuidLightStatus(unsigned char byLightStatus[7]);

    ACRLEDFLASH     m_stAcrLedFlash[ACR500_LED_NUM];
    bool SetGuidLightOnOff(int iLightIndex, int iLightStatus);
private:
    void ReadConfig();
    BYTE CalcBCC(LPBYTE lpData, int iSize);
    bool IsDevReady();
    long SendAndReadCmd(LPBYTE lpSendData, int iSendSize, LPBYTE lpRecvData, DWORD &dwInOutData);
private:
    CQtDLLLoader<IAllDevPort>   m_pDev;    
    CSimpleMutex                m_cMutex;

    int                         m_iACRFlashSlowOnTime;
    int                         m_iACRFlashSlowOffTime;
    int                         m_iACRFlashMediumOnTime;
    int                         m_iACRFlashMediumOffTime;
    int                         m_iACRFlashQuickOnTime;
    int                         m_iACRFlashQuickOffTime;

    bool                        m_bFlashSoftwareControl;
    pthread_t                   m_ulTid;
};

#endif // DEVSIU_ACR500_H
