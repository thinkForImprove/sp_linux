#ifndef DEVCRD_CRT591H_H
#define DEVCRD_CRT591H_H

#include <time.h>

#include "IDevCRD.h"
#include "DevCRD.h"
#include "DevImpl_CRT591H.h"
#include "QtTypeInclude.h"

#define LOG_NAME_DEVCRD     "DevCRD_CRT591H.log"

// 暂存仓信息
typedef
struct st_card_slot_data
{
    struct stSlot
    {
        BOOL    bIsHave;            // 是否有卡
        CHAR    szCardNo[128+1];    // 卡号
        CHAR    szInTime[14+1];     // 存入时间
    }stSlots[5];

    st_card_slot_data()
    {
        Clear();
    }

    void Clear()
    {
        memset(this, 0x00, sizeof(st_card_slot_data));
    }

    void Clear(WORD wSlotNo)
    {
        if (wSlotNo < 1 || wSlotNo > 5)
            return;

        stSlots[wSlotNo - 1].bIsHave = FALSE;
        memset(stSlots[wSlotNo - 1].szCardNo, 0x00, sizeof(stSlots[wSlotNo - 1].szCardNo));
        memset(stSlots[wSlotNo - 1].szInTime, 0x00, sizeof(stSlots[wSlotNo - 1].szInTime));
    }

    INT SlotHaveCount()
    {
        INT nCount = 0;
        for (INT i = 0; i < 5; i ++)
        {
            if (stSlots[i].bIsHave == TRUE)
            {
                nCount ++;
            }
        }
        return nCount;
    }

}STCARDSLOTINFO, LPSTCARDSLOTINFO;

//-------------------------------------------------------------------------------
class CDevCRD_CRT591H : public IDevCRD, public CLogManage
{

public:
    CDevCRD_CRT591H();
    ~CDevCRD_CRT591H();

public:
    // 释放端口
    virtual void Release();
    // 打开与设备的连接
    virtual int Open(const char *pMode);
    // 设备初始化
    virtual int Init(EM_CRD_MEDIA_ACT emActFlag);
    // 关闭与设备的连接
    virtual int Close();
    // 设备复位
    virtual int Reset();
    // 读取设备状态
    virtual int GetDevStat(STCRDDEVSTATUS &stStat);
    // 读取设备信息
    virtual int GetUnitInfo(STCRDUNITINFO &stInfo);
    // 发卡
    virtual int DispenseCard(const int nUnitNo);

public:
    // 设置数据
    virtual int SetData(void *vData, WORD wDataType = 0);
    // 获取数据
    virtual int GetData(void *vData, WORD wDataType = 0);
    // 获取版本号(1DevCRD版本/2固件版本/3设备软件版本/4其他)
    virtual void GetVersion(char* szVer, long lSize, ushort usType);

private:
    INT     ConvertErrorCode(long lRet);
    CHAR*   ConvertErrCodeToStr(long lRet);

private:
    STCARDSLOTINFO              m_stCardSlotInfo;

private:
    CSimpleMutex                m_cMutex;
    CQtDLLLoader<IDevCRD>       m_pDev;
    //DEVCRDSTATUS                m_stStatus;

    CDevImpl_CRT591H             m_devCRT591H;
    //ST_CRD_CRT591H_INIT_PARAM    m_stCRDCRT591HInitParam;
    CHAR                        m_szErrStr[1024];
};

#endif // DEVCRD_CRT591H_H
