/****************************************************************************
 *
 * SIU ACR500控制版设备类
 * **************************************************************************/

#ifndef DEVSIU_ACR500_H
#define DEVSIU_ACR500_H

#include "IAllDevPort.h"
#include "QtTypeInclude.h"
#include "IDevSIU.h"

#define ACR500_TIMEOUT  5000
#define ACR500_ACK      0x06
#define ACR500_NAK      0xE1
#define ACR500_BCCERROR 0xE2
#define ACR500_REPLY_LEN   0x12

class CDevSIU_ACR500 : public CLogManage
{
public:
    CDevSIU_ACR500();
    ~CDevSIU_ACR500();

public:
    //打开设备
    bool Open(const char *pCommParam);
    //关闭设备
    bool Close();
    //复位设备
    bool Reset();
    //设置灯
    bool SetGuidLight(int iLightIndex, int iLightStatus);
    //获取灯的当前状态
    bool GetGuidLightStatus(unsigned char byLightStatus[7]);
private:
    BYTE byCalcBCC(LPBYTE lpData, int iSize);
    bool IsDevReady();
    long SendAndReadCmd(LPBYTE lpSendData, int iSendSize, LPBYTE lpRecvData, DWORD &dwInOutData);
private:
    CQtDLLLoader<IAllDevPort>   m_pDev;
    CQtSimpleMutexEx            m_cSysMutex;
};

#endif // DEVSIU_ACR500_H
