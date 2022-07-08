#pragma once
#include "CDMInterface.h"
#include "MultiString.h"
#include <string>
using namespace std;
#include <string.h>
//#include "NPArray.h"

#define MAX_ALGORITHM_COUNT (3)//最大算法数
//CCDMStatus类:
//功能：实现ICDMStatus接口，同时包装WFSCDMSTATUS数据
struct CCDMStatus : public ICDMStatus, public WFSCDMSTATUS
{
    void InitData()
    {
        memset((WFSCDMSTATUS *)this, 0, sizeof(WFSCDMSTATUS));  //清空本地数据

        memset(&m_Pos, 0, sizeof(m_Pos));
        m_lparrayPos[0] = &m_Pos;      //只第一个元素指向一个有效值
        m_lparrayPos[1] = NULL;         //第二个值总是NULL
        this->lppPositions = m_lparrayPos;
        m_Extra = NULL;
    }

    CCDMStatus()
    {
        InitData();
    }

    virtual ~CCDMStatus()
    {
    }

    virtual long SetDeviceSt(WORD fwDevice)
    {
        this->fwDevice = fwDevice;
        return 0;
    }

    virtual long SetSafeDoorSt(WORD fwSafeDoor)
    {
        this->fwSafeDoor = fwSafeDoor;
        return 0;
    }
    virtual long SetDispenserSt(WORD fwDispenser)
    {
        this->fwDispenser = fwDispenser;
        return 0;
    }

    virtual long SetIntermediateStackerSt(WORD fwIntermediateStacker)
    {
        this->fwIntermediateStacker = fwIntermediateStacker;
        return 0;
    }

    virtual long AddPositionSt(const WFSCDMOUTPOS &Position)
    {
        m_Pos = Position;
        return 0;
    }

    virtual long AddExtraSt(LPCSTR lpszKey, LPCSTR lpszValue)
    {
        char szBuf[1024] = {0};
        sprintf(szBuf, "%s=%s", lpszKey, lpszValue);
        m_Extra.Add(szBuf);
        this->lpszExtra = (LPSTR)(LPCSTR)m_Extra;

        return 0;
    }

private:
    CMultiString m_Extra;   //保存以'\0'分割"\0\0"结束的厂家自定义信息
    WFSCDMOUTPOS m_Pos;     //有效的位置元素
    LPWFSCDMOUTPOS m_lparrayPos[2];   //NULL结束的指针列表，只包含一个元素
};

//CCDMCaps类:
//功能：实现ICDMCaps接口，同时包装WFSCDMCAPS数据
struct CCDMCaps : public ICDMCaps, public WFSCDMCAPS
{
    void InitData()
    {
        memset((WFSCDMCAPS *)this, 0, sizeof(WFSCDMCAPS));  //清空本地数据
        this->wClass = WFS_SERVICE_CLASS_CDM;                //该域取固定值
        m_Extra = NULL;
    }

    CCDMCaps()
    {
        InitData();
    }

    virtual ~CCDMCaps()
    {
    }

    virtual long SetType(WORD fwType)
    {
        this->fwType = fwType;
        return 0;
    }

    virtual long SetMaxDispenseItems(WORD wMaxDispenseItems)
    {
        this->wMaxDispenseItems = wMaxDispenseItems;
        return 0;
    }

    virtual long SetCompound(BOOL bCompound)
    {
        this->bCompound = bCompound;
        return 0;
    }

    virtual long SetShutter(BOOL bShutter)
    {
        this->bShutter = bShutter;
        return 0;
    }

    virtual long SetShutterControl(BOOL bShutterControl)
    {
        this->bShutterControl = bShutterControl;
        return 0;
    }

    virtual long SetRetractAreas(WORD fwRetractAreas)
    {
        this->fwRetractAreas = fwRetractAreas;
        return 0;
    }

    virtual long SetRetractTransportActions(WORD fwRetractTransportActions)
    {
        this->fwRetractTransportActions = fwRetractTransportActions;
        return 0;
    }

    virtual long SetRetractStackerActions(WORD fwRetractStackerActions)
    {
        this->fwRetractStackerActions = fwRetractStackerActions;
        return 0;
    }

    virtual long SetSafeDoor(BOOL bSafeDoor)
    {
        this->bSafeDoor = bSafeDoor;
        return 0;
    }

    virtual long SetCashBox(BOOL bCashBox)
    {
        this->bCashBox = bCashBox;
        return 0;
    }

    virtual long SetIntermediateStacker(BOOL bIntermediateStacker)
    {
        this->bIntermediateStacker = bIntermediateStacker;
        return 0;
    }

    virtual long SetItemsTakenSensor(BOOL bItemsTakenSensor)
    {
        this->bItemsTakenSensor = bItemsTakenSensor;
        return 0;
    }

    virtual long SetPositions(WORD fwPositions)
    {
        this->fwPositions = fwPositions;
        return 0;
    }

    virtual long SetMoveItems(WORD fwMoveItems)
    {
        this->fwMoveItems = fwMoveItems;
        return 0;
    }

    virtual long SetExchangeType(WORD fwExchangeType)
    {
        this->fwExchangeType = fwExchangeType;
        return 0;
    }

    virtual long AddExtraCp(LPCSTR lpszKey, LPCSTR lpszValue)
    {
        char szBuf[1024];
        sprintf(szBuf, "%s=%s", lpszKey, lpszValue);
        m_Extra.Add(szBuf);
        this->lpszExtra = (LPSTR)(LPCSTR)m_Extra;
        return 0;
    }

private:
    CMultiString m_Extra;    //保存以'\0'分割"\0\0"结束的厂家自定义信息
};

//CCDMMixTypes类
//功能：实现ICDMMixTypes接口，包含出钞机支持的一系列配钞算法

struct CCDMMixType :  public WFSCDMMIXTYPE
{
    void InitData()
    {
        memset((WFSCDMMIXTYPE *)this, 0, sizeof(WFSCDMMIXTYPE));
        delete this->lpszName;
    }

    CCDMMixType()
    {
        InitData();
    }

    virtual ~CCDMMixType()
    {
        delete this->lpszName;
    }

    virtual void SetMixNumber(USHORT usMixnumber)
    {
        this->usMixNumber = usMixnumber;
    }
    virtual void SetMixType(USHORT usMixType)
    {
        this->usMixType = usMixType;
    }
    virtual void SetSubType(USHORT usSubType)
    {
        this->usSubType = usSubType;
    }
    virtual void SetName(LPSTR lpszName)
    {
        DWORD dwLen = strlen(lpszName) + 1;
        this->lpszName = new char[dwLen];
        memcpy(this->lpszName, lpszName, dwLen);
        this->lpszName[dwLen - 1] = '\0';
    }
};

struct CCDMMixTypeList
{
    CCDMMixTypeList()
    {
        memset(m_MixTypeList, 0, sizeof(CCDMMixType *) * (MAX_ALGORITHM_COUNT + 1));
        memset(m_ppMixTypeList, 0, sizeof(LPWFSCDMMIXTYPE) * (MAX_ALGORITHM_COUNT + 1));
    };

    ~CCDMMixTypeList()
    {
        Clear();
    };

    void Clear()
    {
        int i = 0;
        while ((m_MixTypeList[i] != nullptr) && (i < MAX_ALGORITHM_COUNT + 1))
        {
            delete m_MixTypeList[i];
            m_MixTypeList[i] = nullptr;
            i++;
        }
    }

    void AddMixTape(USHORT usMixnumber, USHORT usMixType, USHORT usSubType, LPSTR lpszName)
    {
        int i = 0;
        while (true)
        {
            if (i >= MAX_ALGORITHM_COUNT)
            {
                break;
            }
            else if (m_MixTypeList[i] == nullptr)
            {
                m_MixTypeList[i] = new CCDMMixType;
                m_MixTypeList[i]->SetMixNumber(usMixnumber);
                m_MixTypeList[i]->SetMixType(usMixType);
                m_MixTypeList[i]->SetSubType(usSubType);
                m_MixTypeList[i]->SetName(lpszName);
                m_ppMixTypeList[i] = (LPWFSCDMMIXTYPE)m_MixTypeList[i];
                break;
            }
            i++;
        }
    }

    operator LPWFSCDMMIXTYPE *()
    {
        return &m_ppMixTypeList[0];
    }

    LPWFSCDMMIXTYPE m_ppMixTypeList[MAX_ALGORITHM_COUNT + 1];
    CCDMMixType *m_MixTypeList[MAX_ALGORITHM_COUNT + 1];
};

/*
//CCDMCurrencyExp类
//功能：实现ICDMCurrencyExp接口，包含设备支持的所有钞票的信息
struct CCDMCurrencyExp :
{
    void ClearData()
    {
        int i = 0;
        while ((m_MixTypeList[i] != nullptr) && (i < MAX_ALGORITHM_COUNT + 1))
        {
            delete m_MixTypeList[i];
            m_MixTypeList[i] = nullptr;
            i++;
        }
    }

    void AddMixTape(USHORT usMixnumber, USHORT usMixType, USHORT usSubType, LPSTR lpszName)
    {
        int i = 0;
        while (true)
        {
            if (i >= MAX_ALGORITHM_COUNT)
            {
                break;
            }
            else if (m_MixTypeList[i] == nullptr)
            {
                m_MixTypeList[i] = new CCDMMixType;
                m_MixTypeList[i]->SetMixNumber(usMixnumber);
                m_MixTypeList[i]->SetMixType(usMixType);
                m_MixTypeList[i]->SetSubType(usSubType);
                m_MixTypeList[i]->SetName(lpszName);
                m_ppMixTypeList[i] = (LPWFSCDMMIXTYPE)m_MixTypeList[i];
                break;
            }
            i++;
        }
    }

    operator LPWFSCDMMIXTYPE*()
    {
        return &m_ppMixTypeList[0];
    }

    LPWFSCDMMIXTYPE m_ppMixTypeList[MAX_ALGORITHM_COUNT + 1];
    CCDMMixType* m_MixTypeList[MAX_ALGORITHM_COUNT + 1];
};*/



//CCDMPresentStatus类
//功能：实现ICDMPresentStatus接口，同时包装WFSCDMPRESENTSTATUS数据
struct CCDMPresentStatus : public ICDMPresentStatus, public WFSCDMPRESENTSTATUS
{
    void ClearData()
    {
        if (this->lpDenomination != NULL)
        {
            if (this->lpDenomination->lpulValues != NULL)
            {
                delete[] this->lpDenomination->lpulValues;
                this->lpDenomination->lpulValues = NULL;
            }
            delete this->lpDenomination;
            this->lpDenomination = NULL;
        }
        memset((WFSCDMPRESENTSTATUS *)this, 0, sizeof(WFSCDMPRESENTSTATUS));
        m_Extra = NULL;
    }

    CCDMPresentStatus()
    {
        memset((WFSCDMPRESENTSTATUS *)this, 0, sizeof(WFSCDMPRESENTSTATUS));
    }

    virtual ~CCDMPresentStatus()
    {
        ClearData();
    }

    //设置配钞操作的内容
    //返回值 0 成功
    virtual long SetDenomination(const LPWFSCDMDENOMINATION pDeno)
    {
        if (this->lpDenomination == NULL)
        {
            this->lpDenomination = new WFSCDMDENOMINATION;
            memset(this->lpDenomination, 0, sizeof(WFSCDMDENOMINATION));
        }

        //释放掉先前分配的内存
        if (this->lpDenomination->lpulValues != NULL)
        {
            delete[] this->lpDenomination->lpulValues;
            this->lpDenomination->lpulValues = NULL;
        }

        *this->lpDenomination = *pDeno;

        //保证钞票列表不为空且列表大小不等于0或者列表为空且大小为0
        if (pDeno->lpulValues != NULL &&
            pDeno->usCount != 0)
        {
            this->lpDenomination->lpulValues = new ULONG[pDeno->usCount];
            memcpy(this->lpDenomination->lpulValues, pDeno->lpulValues,
                   sizeof(ULONG) * pDeno->usCount);
        }
        else
        {
            this->lpDenomination->lpulValues = NULL;
            this->lpDenomination->usCount = 0;
        }
        return 0;
    }

    virtual long SetPresentState(WORD wPresentState)
    {
        this->wPresentState = wPresentState;
        return 0;
    }

    virtual long AddExtra(LPCSTR lpszKey, LPCSTR lpszValue)
    {
        char szBuf[1024];
        sprintf(szBuf, "%s=%s", lpszKey, lpszValue);
        m_Extra.Add(szBuf);
        this->lpszExtra = (LPSTR)(LPCSTR)m_Extra;
        return 0;
    }

private:
    CMultiString m_Extra;         //保存以'\0'分割"\0\0"结束的厂家自定义信息
};

//CCDMDenomination类
//功能：实现ICDMDenomination接口，同时包装WFSCDMDENOMINATION数据
struct CCDMDenomination : public ICDMDenomination, public WFSCDMDENOMINATION
{
    void ClearData()
    {
        if (this->lpulValues != NULL)
        {
            delete[] this->lpulValues;
            this->lpulValues = NULL;
        }
        memset((WFSCDMDENOMINATION *)this, 0, sizeof(WFSCDMDENOMINATION));
    }

    CCDMDenomination()
    {
        memset((WFSCDMDENOMINATION *)this, 0, sizeof(WFSCDMDENOMINATION));
    }

    virtual ~CCDMDenomination()
    {
        ClearData();
    }

    //SetCurrency、GetCurrency分别为设置和获取币种的ID，例如"CNY",如果是多币种，则为3个0x20
    virtual void SetCurrency(char cCurrencyID[3])
    {
        memcpy(this->cCurrencyID, cCurrencyID, sizeof(this->cCurrencyID));
    }

    virtual const char *GetCurrency() const
    {
        return this->cCurrencyID;
    }

    virtual void SetAmount(ULONG Amount)
    {
        this->ulAmount = Amount;
    }

    virtual ULONG GetAmount() const
    {
        return this->ulAmount;
    }

    virtual void SetCashBox(ULONG CashBox)
    {
        this->ulCashBox = CashBox;
    }

    virtual ULONG GetCashBox() const
    {
        return this->ulCashBox;
    }

    //设置在本次交易中每一个钞箱各出了多少张钞票
    //Count[in]:Values数组的大小
    //Values[in]:本次交易中各个钞箱各出了多少张钞票
    //返回值   0  成功  WFS_ERR_INVALID_DATA 失败
    virtual long SetValues(USHORT Count, LPULONG Values)
    {
        if ((Count != 0 && Values == NULL))
        {
            return WFS_ERR_INVALID_DATA;
        }

        this->usCount = Count;

        if (Count == 0)             //当Count == 0，this->lpulValues只能取NULL
        {
            if (this->lpulValues != NULL)
            {
                delete[] this->lpulValues;
            }
            this->lpulValues = NULL;
        }
        else                        //当Count != 0,this->lpulValues必定不为NULL
        {
            if (this->lpulValues != NULL)
            {
                delete[] this->lpulValues;
            }
            this->lpulValues = new ULONG[Count];
            memcpy(this->lpulValues, Values, sizeof(ULONG) * Count);
        }
        return 0;
    }

    //功能：获取所有钞箱出钞数数组
    //Count[out]: 返回数组大小
    //返回值: 所有钞箱出钞数数组
    virtual const LPULONG GetValues(USHORT &Count) const
    {
        Count = this->usCount;
        return this->lpulValues;
    }
};
