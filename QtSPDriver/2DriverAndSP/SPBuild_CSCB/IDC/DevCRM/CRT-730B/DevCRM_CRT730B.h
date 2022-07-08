#ifndef DEVCRM_CRT730B_H
#define DEVCRM_CRT730B_H

#include <time.h>

#include "IDevCRM.h"
#include "DevCRM.h"
#include "DevImpl_CRT730B.h"
#include "QtTypeInclude.h"

#define LOG_NAME_DEVCRM     "DevCRM_CRT730B.log"

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
class CDevCRM_CRT730B : public IDevCRM, public CLogManage
{

public:
    CDevCRM_CRT730B();
    ~CDevCRM_CRT730B();

public:
    // 释放接口
    virtual void Release();
    // 打开连接
    virtual int Open(LPCSTR lpMode);
    // 设备初始化
    virtual int Init(CRMInitAction eActFlag);
    // 关闭连接
    virtual int Close();
    // 复位
    virtual int Reset();
    // 读取设备信息
    virtual int GetDevInfo(char *pInfo);
    // 读取暂存仓信息
    virtual int GetCardSlotInfo(STCRMSLOTINFO &stInfo);
    // 读取设备状态
    virtual int GetStatus(STCRMSTATUS &stStatus);
    // 指定卡号退卡
    virtual int CMEjectCard(const char *szCardNo);
    // 指定暂存仓编号退卡
    virtual int CMEjectCard(const int nSlotNo);
    // 指定卡号和暂存仓存卡
    virtual int CMRetainCard(const char *szCardNo, const int nSlotNo);
    // 移动暂存仓
    virtual int CMCardSlotMove(const int nMode, int &nSlotNo);
    // 设备复位
    virtual int CMReset();

public:
    // 设置数据
    virtual int SetData(void *vData, WORD wDataType = 0);
    // 获取数据
    virtual int GetData(void *vData, WORD wDataType = 0);
    // 获取版本号(1DevCRM版本/2固件版本/3设备软件版本/4其他)
    virtual void GetVersion(char* szVer, long lSize, ushort usType);

private:
    int     UpdataSTSlotData();
    INT     ConvertErrorCode(long lRet);
    CHAR*   ConvertErrCodeToStr(long lRet);

private:
    STCARDSLOTINFO              m_stCardSlotInfo;

private:
    CSimpleMutex                m_cMutex;
    //CQtDLLLoader<IDevCRM>       m_pDev;
    //DEVCRMSTATUS                m_stStatus;

    CDevImpl_CRT730B             m_devCRT730B;
    //ST_CRM_CRT730B_INIT_PARAM    m_stCRMCRT730BInitParam;
    CHAR                        m_szErrStr[1024];
};

#endif // DEVCRM_CRT730B_H
