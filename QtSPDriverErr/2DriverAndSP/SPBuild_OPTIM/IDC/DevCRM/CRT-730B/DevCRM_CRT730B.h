/***************************************************************
* 文件名称：DevIDC_CRT730B.h
* 文件描述：退卡模块功能处理接口封装 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年10月20日
* 文件版本：1.0.0.1
****************************************************************/

#ifndef DEVCRM_CRT730B_H
#define DEVCRM_CRT730B_H

#include <time.h>

#include "IDevCRM.h"
#include "DevCRM.h"
#include "DevImpl_CRT730B.h"
#include "QtTypeInclude.h"
#include "ErrorDetail.h"

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
    virtual int Release();
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
    virtual int SetData(unsigned short usType, void *vData = nullptr);
    // 获取数据
    virtual int GetData(unsigned short usType, void *vData);
    // 获取版本号(1DevCRM版本/2固件版本/3设备软件版本/4其他)
    virtual int GetVersion(unsigned short usType, char* szVer, int nSize);

private:
    int     UpdataSTSlotData();
    INT     ConvertErrorCode(long lRet);
    CHAR*   ConvertErrCodeToStr(long lRet);    
    INT     ConvertImplErrCode2ErrDetail(INT nRet);             // 根据Impl错误码设置错误错误码字符串

private:
    STCARDSLOTINFO                  m_stCardSlotInfo;

private:
    CSimpleMutex                    m_cMutex;
    CErrorDetail                    m_clErrorDet;

    CDevImpl_CRT730B                m_devCRT730B;
    STDEVICEOPENMODE                m_stOpenMode;                   // 设备打开方式
    CHAR                            m_szErrStr[1024];
    WORD                            m_wDeviceType;                  // 设备类型
};

#endif // DEVCRM_CRT730B_H
