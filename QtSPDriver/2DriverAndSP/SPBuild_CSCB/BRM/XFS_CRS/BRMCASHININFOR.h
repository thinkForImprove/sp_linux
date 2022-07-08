//BRMCASHININFOR.h
#ifndef BRM_CASH_IN_INFOR_H
#define BRM_CASH_IN_INFOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "XFSCIM.H"
#include "ExtraInforHelper.h"
#include <assert.h>

class XFS_CRSImp;

// 永久化进钞结果
typedef struct _tag_brm_cash_in_infor
{
    //构造函数，清空数据
    _tag_brm_cash_in_infor()
    {
        bAvailable = FALSE;
        wStatus = 0;
        usNumOfRefused = 0;
        usNumOfNoteNumbers = 0;
        lpNoteNumber = NULL;
        ulTotalNumber = 0;
    }

    //析构函数，释放使用过程的分配的内存
    ~_tag_brm_cash_in_infor()
    {
        if (lpNoteNumber != NULL)
        {
            delete [] lpNoteNumber;
            lpNoteNumber = NULL;
        }
    }

    //触发CashInStart
    //在SP类的CashInStart中调用
    void CashInStart()
    {
        bAvailable = TRUE;
        wStatus = WFS_CIM_CIACTIVE;
        usNumOfRefused = 0;
        usNumOfNoteNumbers = 0;
        if (lpNoteNumber != NULL)
        {
            delete [] lpNoteNumber;
            lpNoteNumber = NULL;
        }
        ulTotalNumber = 0;
        m_Extra.Clear();
    }

    //得到数据是否有效
    BOOL GetAvailable() const
    {
        return bAvailable;
    }

    //得到CashInStatus状态
    WORD GetState() const
    {
        return wStatus;
    }

    //设置状态
    void SetState(WORD wStatus)
    {
        this->wStatus = wStatus;
    }

    //得到拒钞数
    USHORT GetRefusedCount() const
    {
        return usNumOfRefused;
    }

    //增加拒钞数
    void AddRefusedCount(USHORT usNumOfRefused)
    {
        this->usNumOfRefused += usNumOfRefused;
    }

    //得到NoteNumber的个数
    USHORT GetNoteNumberCount() const
    {
        return usNumOfNoteNumbers;
    }

    //增加NoteNumber
    //如果NoteID不存在则增加，否则把张数回上原张数上
    void AddNoteNumber(USHORT usNoteID, ULONG ulCount)
    {
        //在现存的NoteNumber列表中查找NoteID
        USHORT i;
        for (i = 0; i < usNumOfNoteNumbers; i++)
        {
            if (lpNoteNumber[i].usNoteID == usNoteID)
            {
                break;
            }
        }

        //如果没有找到，则重新分配内存，并拷贝原数据
        if (i == usNumOfNoteNumbers)
        {
            LPWFSCIMNOTENUMBER pNew = new WFSCIMNOTENUMBER[usNumOfNoteNumbers + 1];
            if (lpNoteNumber != NULL)
            {
                memcpy(pNew, lpNoteNumber, sizeof(WFSCIMNOTENUMBER) * usNumOfNoteNumbers);
                delete [] lpNoteNumber;
            }
            lpNoteNumber = pNew;
            lpNoteNumber[i].ulCount = 0;
            lpNoteNumber[i].usNoteID = usNoteID;
            usNumOfNoteNumbers++;
        }

        //增加张数
        lpNoteNumber[i].ulCount += ulCount;
        ulTotalNumber += ulCount;
    }

    //得到第usIndex个NoteNumber的ID
    //usIndex：从0开始
    USHORT GetIDOfNoteNumber(USHORT usIndex) const
    {
        assert(usIndex < usNumOfNoteNumbers);
        return lpNoteNumber[usIndex].usNoteID;
    }

    //得到第usIndex个NoteNumber的张数
    //usIndex：从0开始
    ULONG GetCountOfNoteNumber(USHORT usIndex) const
    {
        assert(usIndex < usNumOfNoteNumbers);
        return lpNoteNumber[usIndex].ulCount;
    }

    //得到总张数
    ULONG GetTotalNumber() const
    {
        return ulTotalNumber;
    }

    //比较操作符
    bool operator==(const _tag_brm_cash_in_infor &other) const
    {
        if (bAvailable != other.bAvailable)
            return FALSE;       // 送钞数据是否可用
        if (wStatus != other.wStatus)
            return FALSE;
        if (usNumOfRefused != other.usNumOfRefused)
            return FALSE;
        if (usNumOfNoteNumbers != other.usNumOfNoteNumbers)
            return FALSE;
        if ((lpNoteNumber == NULL || other.lpNoteNumber == NULL) &&
            lpNoteNumber != other.lpNoteNumber)
            return FALSE;
        if (lpNoteNumber != NULL &&
            other.lpNoteNumber != NULL &&
            memcmp(lpNoteNumber, other.lpNoteNumber, sizeof(WFSCIMNOTENUMBER) * usNumOfNoteNumbers) != 0)
            return FALSE;
        return TRUE;
    }

    //不等比较符
    bool operator!=(const _tag_brm_cash_in_infor &other) const
    {
        return !(*this == other);
    }

    //赋值操作符
    _tag_brm_cash_in_infor &operator=(const _tag_brm_cash_in_infor &other)
    {
        if (this == &other)
            return *this;
        bAvailable = other.bAvailable;      // 送钞数据是否可用
        wStatus = other.wStatus;
        usNumOfRefused = other.usNumOfRefused;
        usNumOfNoteNumbers = other.usNumOfNoteNumbers;
        ulTotalNumber = other.ulTotalNumber;    //总张数
        if (lpNoteNumber != NULL)
        {
            delete [] lpNoteNumber;
            lpNoteNumber = NULL;
        }
        if (usNumOfNoteNumbers > 0)
        {
            lpNoteNumber = new WFSCIMNOTENUMBER[usNumOfNoteNumbers];
            memcpy(lpNoteNumber, other.lpNoteNumber, sizeof(WFSCIMNOTENUMBER) * usNumOfNoteNumbers);
        }

        return *this;
    }

    void ClearNoteNumberList()
    {
        if (lpNoteNumber != NULL)
        {
            delete [] lpNoteNumber;
        }
        lpNoteNumber = NULL;
        usNumOfNoteNumbers = 0;
    }
    //内部数据
protected:
    friend class XFS_CRSImp;    //SP.ReadWriteCashInFile直接访问它的数据成员

    BOOL                bAvailable;         //送钞数据是否可用
    WORD                wStatus;            //状态
    USHORT              usNumOfRefused;     //拒钞数
    USHORT              usNumOfNoteNumbers; //NoteNumber个数，lpNoteNumber的尺寸
    LPWFSCIMNOTENUMBER  lpNoteNumber;

    ULONG               ulTotalNumber;      //总张数
    //CExtraInforManager    m_Extra;            //额外信息
    CExtraInforHelper   m_Extra;
} BRMCASHININFOR, *LPBRMCASHININFOR;

#ifdef __cplusplus
}       /*extern "C"*/
#endif

#endif //BRM_CASH_IN_INFOR_H
