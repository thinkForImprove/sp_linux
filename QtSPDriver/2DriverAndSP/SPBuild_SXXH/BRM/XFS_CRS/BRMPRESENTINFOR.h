//BRMPRESENTINFOR.h
#ifndef BRM_PRESENT_INFOR_H
#define BRM_PRESENT_INFOR_H

#include "XFSCIM.H"
#include "XFSCDM.H"
#include "ExtraInforManager.h"
//#include "ExtraInforHelper.h"
class XFS_CRSImp;

// 永久化送钞结果数据
typedef struct _tag_brm_present_infor
{
    //构造函数，清空数据
    _tag_brm_present_infor()
    {
        bAvailable = FALSE;
        wPresentState = 0;
        memset(cCurrencyID, 0, sizeof(cCurrencyID));
        ulAmount = 0;
        usCount = 0;
        lpulValues = nullptr;
    }

    //析构函数，释放使用过程的分配的内存
    ~_tag_brm_present_infor()
    {
        if (lpulValues != nullptr)
        {
            delete [] lpulValues;
            lpulValues = nullptr;
        }
    }

    //pDeno->lpulValues直接指向本对象的lpulValues，外部不要delete
    void GetDenomination(LPWFSCDMDENOMINATION pDeno)
    {
        memcpy(pDeno->cCurrencyID, cCurrencyID, sizeof(cCurrencyID));
        pDeno->ulAmount = ulAmount;
        pDeno->usCount = usCount;
        pDeno->lpulValues = lpulValues;
        pDeno->ulCashBox = 0;
    }

    //设置状态
    void SetPresentState(WORD wPresentState)
    {
        this->wPresentState = wPresentState;
    }

    //设置数据
    void SetData(BOOL bAvailable,
                 WORD wPresentState,
                 const CHAR cCurrencyID[3],
                 ULONG ulAmount,
                 USHORT usCount,
                 LPULONG lpulValues)
    {
        this->bAvailable = bAvailable;                  // 数据是否可用
        this->wPresentState = wPresentState;
        memcpy(this->cCurrencyID, cCurrencyID, 3);
        this->ulAmount = ulAmount;
        this->usCount = usCount;
        if (this->lpulValues != nullptr)
        {
            delete [] this->lpulValues;
            this->lpulValues = nullptr;
        }
        if (usCount > 0)
        {
            this->lpulValues = new ULONG[usCount];
            memcpy(this->lpulValues, lpulValues, usCount * sizeof(ULONG));
        }
        m_Extra.Clear();
    }

    //得到数据是否有效
    BOOL GetAvailable() const
    {
        return bAvailable;
    }

    //得到PRESENT状态
    WORD GetPresentState() const
    {
        return wPresentState;
    }

    //增加扩展信息
    void AddExtra(LPCSTR lpszKey, LPCSTR lpszValue)
    {
        m_Extra.AddExtra(lpszKey, lpszValue);
    }

    //比较操作符
    bool operator==(_tag_brm_present_infor &other)
    {
        if (bAvailable != other.bAvailable) return FALSE;
        if (wPresentState != other.wPresentState) return FALSE;
        if (memcmp(cCurrencyID, other.cCurrencyID, 3) != 0) return FALSE;
        if (ulAmount != other.ulAmount) return FALSE;
        if (usCount != other.usCount) return FALSE;
        if ((lpulValues == nullptr || other.lpulValues == nullptr) &&
            lpulValues != other.lpulValues) return FALSE;
        if (lpulValues != nullptr &&
            other.lpulValues != nullptr &&
            memcmp(lpulValues, other.lpulValues, sizeof(ULONG) * usCount) != 0)
            return FALSE;
        if (!m_Extra.IsEqual(other.m_Extra))
            return FALSE;
        return TRUE;
    }

    //比较操作符
    bool operator!=(_tag_brm_present_infor &other)
    {
        return !(*this == other);
    }


    //赋值操作符
    _tag_brm_present_infor &operator=(_tag_brm_present_infor &other)
    {
        if (this == &other)
            return *this;
        bAvailable = other.bAvailable;
        wPresentState = other.wPresentState;
        memcpy(cCurrencyID, other.cCurrencyID, 3);
        ulAmount = other.ulAmount;
        usCount = other.usCount;
        if (lpulValues != nullptr)
        {
            delete [] lpulValues;
            lpulValues = nullptr;
        }
        if (usCount > 0)
        {
            lpulValues = new ULONG[usCount];
            memcpy(lpulValues, other.lpulValues, sizeof(ULONG) * usCount);
        }

        CExtraInforManager *pc = &other.m_Extra;
        m_Extra.CopyFrom(*pc);
        return *this;
    }

    //内部数据

    friend class XFS_CRSImp;
protected:
    BOOL    bAvailable;                 //数据是否可用
    WORD    wPresentState;              //PRESENT状态
    CHAR    cCurrencyID[3];             //币种ID
    ULONG   ulAmount;                   //金额
    USHORT  usCount;                    //面值个数，lpulValues尺寸
    LPULONG lpulValues;                 //面值
    CExtraInforManager m_Extra;     //额外信息
    //CExtraInforHelper m_Extra;

} BRMPRESENTINFOR, *LPBRMPRESENTINFOR;


#endif  //BRM_PRESENT_INFOR_H

