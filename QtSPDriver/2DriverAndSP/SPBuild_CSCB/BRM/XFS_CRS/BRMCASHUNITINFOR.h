//BRMCASHUNITINFOR
#ifndef BRM_CASH_UNIT_INFOR_H
#define BRM_CASH_UNIT_INFOR_H

#include "IBRMAdapter.h"
#include <assert.h>

#include <string.h>
#include <vector>
using namespace std;

// 钞箱信息结构，主要服务于适配层
typedef struct _tag_brm_cashunit_infor
{
    //构造函数，清空数据
    _tag_brm_cashunit_infor()
    {
        Type = ADP_CASSETTE_UNKNOWN;
        memset(cCurrencyID, 0, sizeof(cCurrencyID));
        ulValues = 0;
        stPhysicalCU = ADP_CASHUNIT_OK;
        memset(arycUnitID, 0, sizeof(arycUnitID));
        NoteIDs.push_back(0);
        ulCount = 0;
    }

    //析构函数，释放使用过程的分配的内存
    ~_tag_brm_cashunit_infor()
    {
    }

    //得到钞箱类型
    ADP_CASSETTE_TYPE GetType() const
    {
        return Type;
    }

    //设置钞箱类型, SP.UpdateADPCassInfoFromCUManager调用来设置类型
    void SetType(ADP_CASSETTE_TYPE eType)
    {
        this->Type = eType;
    }

    //得到币种ID
    const char *GetCurrencyID() const
    {
        return cCurrencyID;
    }

    //设置币种ID
    void SetCurrencyID(const char cCurrencyID[3])
    {
        memcpy(this->cCurrencyID, cCurrencyID, sizeof(this->cCurrencyID));
    }

    //得到面值
    ULONG GetValue() const
    {
        return ulValues;
    }

    //设置面值
    void SetValue(ULONG ulValues)
    {
        this->ulValues = ulValues;
    }

    //得到状态
    CASHUNIT_STATUS GetStatus() const
    {
        return stPhysicalCU;
    }

    //设置状态
    void SetStatus(CASHUNIT_STATUS eStatus)
    {
        stPhysicalCU = eStatus;
    }

    //得到NOTE ID，以0结束
    const USHORT *GetNoteIDs() const
    {
        vector<USHORT>::const_iterator it = NoteIDs.begin();
        //return NoteIDs.begin();
        return &(*it);
    }

    //设置NOTE ID，以0结束
    void SetNoteIDs(LPUSHORT lpusNoteIDs)
    {
        NoteIDs.clear();
        if (lpusNoteIDs == NULL)
        {
            NoteIDs.push_back(0);
            return;
        }
        for (int i = 0; lpusNoteIDs[i] != 0; i++)
        {
            NoteIDs.push_back(lpusNoteIDs[i]);
        }
        NoteIDs.push_back(0);
    }

    void AddNoteID(USHORT usNoteID)
    {
        assert(NoteIDs.size() > 0);
        NoteIDs.pop_back();
        NOTEIDARRAY::iterator it;
        for (it = NoteIDs.begin(); it != NoteIDs.end(); it++)
        {
            if (usNoteID == *it)
                break;
        }
        if (it != NoteIDs.end())
            return;
        NoteIDs.push_back(usNoteID);
        NoteIDs.push_back(0);
    }

    //得到NOTE ID的个数
    USHORT GetNoteIDCount() const
    {
        return (USHORT)NoteIDs.size() - 1;
    }

    //得到NOTE ID的个数（static版本）
    //p：以0结束的NOTE ID列表
    static USHORT GetNoteIDCount(LPUSHORT p)
    {
        USHORT usCount = 0;
        while (*p != 0)
        {
            usCount++;
            p++;
        }
        return usCount;
    }

    //得到钞箱ID
    const char *GetUnitID() const
    {
        return arycUnitID;
    }

    //设置钞箱ID
    void SetUnitID(const char cUnitID[5])
    {
        memcpy(arycUnitID, cUnitID, 5);
    }

    //当前张数
    ULONG GetCount() const
    {
        return ulCount;
    }

    void SetCount(ULONG ulCount)
    {
        this->ulCount = ulCount;
    }

    //赋值操作符
    _tag_brm_cashunit_infor &operator=(const _tag_brm_cashunit_infor &other)
    {
        if (this == &other)
            return *this;
        Type = other.Type;
        memcpy(cCurrencyID, other.cCurrencyID, 3);
        ulValues = other.ulValues;
        NoteIDs = other.NoteIDs;

        stPhysicalCU = other.stPhysicalCU;
        memcpy(arycUnitID, other.arycUnitID, 5);

        ulCount = other.ulCount;
        return *this;
    }

    //比较操作符
    bool operator==(const _tag_brm_cashunit_infor &other) const
    {
        if (Type != other.Type)
            return FALSE;
        if (memcmp(cCurrencyID, other.cCurrencyID, 3) != 0)
            return FALSE;
        if (ulValues != other.ulValues)
            return FALSE;
        if (stPhysicalCU != other.stPhysicalCU)
            return FALSE;
        return FALSE;
        if (GetNoteIDCount() != other.GetNoteIDCount())
            return FALSE;
        if (memcmp(GetNoteIDs(), other.GetNoteIDs(), sizeof(USHORT) * GetNoteIDCount()) != 0)
            return FALSE;
        if (memcmp(arycUnitID, other.arycUnitID, 5) != 0)
            return FALSE;
        if (ulCount != other.ulCount)
            return FALSE;

        return TRUE;
    }

    //比较操作符
    bool operator!=(const _tag_brm_cashunit_infor &other) const
    {
        return !(*this == other);
    }

    //内部数据成员
private:
    typedef vector<USHORT>  NOTEIDARRAY;
    // 钞箱类型
    ADP_CASSETTE_TYPE   Type;
    // 币种
    CHAR                cCurrencyID[3];
    // 面额
    ULONG               ulValues;
    // 以0结尾的数组，表示该钱箱可收纳的钞票ID
    NOTEIDARRAY         NoteIDs;
    // 钞箱状态
    CASHUNIT_STATUS     stPhysicalCU;
    //钞箱ID
    char                arycUnitID[5];
    //当前张数
    ULONG               ulCount;
} BRMCASHUNITINFOR, *LPBRMCASHUNITINFOR;


#endif //BRM_CASH_UNIT_INFOR_H

