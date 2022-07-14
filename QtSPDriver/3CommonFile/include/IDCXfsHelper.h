/***************************************************************
* 文件名称：IDCXfsHelper.h
* 文件描述：IDC系列标准结构体封装类
*
* 使用模块: IDX,UKEY,IDC,MSR(如对该文件进行修改,请确认对已使用模块的影响)
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年6月30日
* 文件版本：1.0.0.1
****************************************************************/

#include "XfsHelper.h"

/* 该部分内容已合并到 XfsHelper.h, 注销, 确认所有工程文件修改为引用 XfsHelper.h后, 该文件可以删除

#pragma once

#include "QtTypeDef.h"
#include "XFSIDC.H"

#define MAX_CARD_DATA_COUNT         (7)                     //最大的卡数据个数
#define MAX_CHIP_IO_LEN             4096                    //最大的ChipIO返回数据长度

//todo char gDefExtraInfo[2] = {0, 0};
class CWfsIDCStatus : public WFSIDCSTATUS
{
public:
    CWfsIDCStatus()
    {
        Clear();
    }

    // 初始化
    void Clear()
    {
        fwDevice    = WFS_IDC_DEVONLINE;
        fwMedia     = WFS_IDC_MEDIANOTPRESENT;
        fwRetainBin = WFS_IDC_RETAINBINOK;
        fwSecurity  = WFS_IDC_SECNOTSUPP;
        usCards     = 0;
        fwChipPower = WFS_IDC_CHIPNOTSUPP;
        lpszExtra   = nullptr;//todogDefExtraInfo;
    }

    // 用于比较
    bool Diff(CWfsIDCStatus clStat)
    {
        if (this->fwDevice != clStat.fwDevice ||
            this->fwMedia != clStat.fwMedia ||
            this->fwRetainBin != clStat.fwRetainBin ||
            this->fwSecurity != clStat.fwSecurity ||
            this->usCards != clStat.usCards ||
            this->fwChipPower != clStat.fwChipPower)
        {
            return true;
        }
        return false;
    }
	// 用于复制
    void Copy(CWfsIDCStatus clStat)
    {
        this->fwDevice = clStat.fwDevice;
        this->fwMedia = clStat.fwMedia;
        this->fwRetainBin = clStat.fwRetainBin;
        this->fwSecurity = clStat.fwSecurity;
        this->usCards = clStat.usCards;
        this->fwChipPower = clStat.fwChipPower;
    }
};

class CWfsIDCCap : public WFSIDCCAPS
{
public:
    CWfsIDCCap()
    {
        wClass          = WFS_SERVICE_CLASS_IDC;
        fwType          = WFS_IDC_TYPEMOTOR;
        bCompound       = false;
        fwReadTracks    = WFS_IDC_TRACK1 | WFS_IDC_TRACK2 | WFS_IDC_TRACK3;
        fwWriteTracks   = WFS_IDC_TRACK1 | WFS_IDC_TRACK2 | WFS_IDC_TRACK3;
        fwChipProtocols = WFS_IDC_NOTSUPP;
        usCards         = 20;
        fwSecType       = WFS_IDC_SECNOTSUPP;
        fwPowerOnOption = WFS_IDC_NOACTION;
        fwPowerOffOption = WFS_IDC_NOACTION;
        bFluxSensorProgrammable = false;
        bReadWriteAccessFollowingEject = false;
        fwWriteMode     = WFS_IDC_LOCO | WFS_IDC_HICO | WFS_IDC_AUTO;
        fwChipPower     = WFS_IDC_NOTSUPP;
        lpszExtra       = nullptr;//todo gDefExtraInfo;
    }
};

class CWFSChipIO: public WFSIDCCHIPIO
{
public:
    CWFSChipIO()
    {
        wChipProtocol       = 0;
        ulChipDataLength    = 0;
        lpbChipData         = ChipBuffer;
        memset(lpbChipData, 0, sizeof(BYTE)*MAX_CHIP_IO_LEN);
    }

    const CWFSChipIO &operator=(const WFSIDCCHIPIO &data)
    {
        if (!(this == &data))
        {
            if (data.ulChipDataLength != 0 && data.ulChipDataLength < MAX_CHIP_IO_LEN)
            {
                this->wChipProtocol = data.wChipProtocol;
                this->ulChipDataLength = data.ulChipDataLength;
                memcpy(this->lpbChipData, data.lpbChipData, data.ulChipDataLength * sizeof(BYTE));
            }
        }
        return *this;
    }
private:
    BYTE ChipBuffer[MAX_CHIP_IO_LEN];
};

class CWFSChipPower
{
public:
    CWFSChipPower()
    {
        m_data.lpbChipData = NULL;
        m_data.ulChipDataLength = 0;
    }

    ~CWFSChipPower()
    {
        Clear();
    }

    void Clear()
    {
        if (m_data.lpbChipData)
        {
            delete[] m_data.lpbChipData;
            m_data.lpbChipData = NULL;
        }
        m_data.ulChipDataLength = 0;
    }

    void SetData(char *lpData, ULONG ulDataLen)
    {
        Clear();

        if (ulDataLen != 0)
        {
            m_data.lpbChipData = new unsigned char[ulDataLen];
            memcpy(m_data.lpbChipData, lpData, ulDataLen);
            m_data.ulChipDataLength = ulDataLen;
        }
    }

    WFSIDCCHIPPOWEROUT *GetData()
    {
        return &m_data;
    }

    const CWFSChipPower &operator=(const CWFSChipPower &data)
    {
        if (!(this == &data))
        {
            Clear();

            if (data.m_data.ulChipDataLength != 0)
            {
                m_data.lpbChipData = new unsigned char[data.m_data.ulChipDataLength];
                m_data.ulChipDataLength = data.m_data.ulChipDataLength;
                memcpy(m_data.lpbChipData, data.m_data.lpbChipData, m_data.ulChipDataLength);
            }
        }

        return *this;
    }

private:
    WFSIDCCHIPPOWEROUT m_data;
};

class CWFSIDCCardData : public WFSIDCCARDDATA   //封装WFSIDCCARDDATA
{
public:
    explicit CWFSIDCCardData()
    {
        lpbData = NULL;
    }

    explicit inline CWFSIDCCardData(const WFSIDCCARDDATA &card_data)
    {
        lpbData = NULL;
        *this = card_data;
    }

    const CWFSIDCCardData &operator=(const WFSIDCCARDDATA &card_data)
    {
        if (lpbData)
            delete [] lpbData;
        lpbData = NULL;
        wDataSource     = card_data.wDataSource;
        wStatus         = card_data.wStatus;
        ulDataLength    = card_data.ulDataLength;
        fwWriteMethod   = card_data.fwWriteMethod;
        if (card_data.lpbData)
        {
            lpbData = new BYTE[card_data.ulDataLength + 1];
            memcpy(lpbData, card_data.lpbData, card_data.ulDataLength);
            lpbData[ulDataLength] = 0;
        }
        else
            lpbData = NULL;
        return *this;
    }

    inline const CWFSIDCCardData &operator=(const CWFSIDCCardData &card_data)
    {
        return operator=((const WFSIDCCARDDATA &)card_data);
    }

    ~CWFSIDCCardData()
    {
        if (lpbData)
            delete [] lpbData;
        lpbData = NULL;
    }
};

class CWFSIDCCardDataPtrArray   //封装LPLFSIDCCARDDATA数组
{
public:
    CWFSIDCCardDataPtrArray()
    {
        Clear();
    }

    inline explicit CWFSIDCCardDataPtrArray(LPWFSIDCCARDDATA *pp)
    {
        *this = pp;
    }

    inline explicit CWFSIDCCardDataPtrArray(const CWFSIDCCardDataPtrArray &array)
    {
        *this = array;
    }

    const CWFSIDCCardDataPtrArray &operator=(const CWFSIDCCardDataPtrArray &array)
    {
        Clear();
        for (int i = 0; i < MAX_CARD_DATA_COUNT; i++)
        {
            datas[i] = array.datas[i];
            if (array.pCardDatas[i])
                pCardDatas[i] = datas + i;
        }
        return *this;
    }

    const CWFSIDCCardDataPtrArray &operator=(LPWFSIDCCARDDATA *pp)
    {
        Clear();
        int i = 0;
        while (*pp)
        {
            datas[i] = **pp;
            pCardDatas[i] = datas + i;
            i++;
            pp++;
        }
        return *this;
    }

    inline operator LPWFSIDCCARDDATA *() const
    {
        return (LPWFSIDCCARDDATA *)pCardDatas;
    }

    inline LPWFSIDCCARDDATA operator [](int index) const
    {
        //assert(index >= 0 && index < MAX_CARD_DATA_COUNT);
        return pCardDatas[index];
    }

    inline void Clear()
    {
        memset(pCardDatas, 0, sizeof(pCardDatas));
    }

    LPWFSIDCCARDDATA Find(DWORD data_source)
    {
        int i = 0;
        while (pCardDatas[i])
        {
            if (pCardDatas[i]->wDataSource == data_source)
                return pCardDatas[i];
            i++;
        }
        return NULL;
    }

    WORD GetOption() const
    {
        WORD wOption = 0;
        int i = 0;
        while (pCardDatas[i])
        {
            wOption |= pCardDatas[i]->wDataSource;
            i++;
        }
        return wOption;
    }

    DWORD Size() const
    {
        DWORD i = 0;
        while (pCardDatas[i])
            i++;
        return i;
    }

    BOOL Add(const WFSIDCCARDDATA &data)
    {
        DWORD dwSize = Size();
        //assert(dwSize < MAX_CARD_DATA_COUNT - 1);
        if (dwSize >= MAX_CARD_DATA_COUNT - 1)
            return false;
        datas[dwSize] = data;
        pCardDatas[dwSize] = datas + dwSize;
        return true;
    }

    BOOL SetAt(WORD data_source, const WFSIDCCARDDATA &card_data)
    {
        int i = 0;
        while (pCardDatas[i])
        {
            if (pCardDatas[i]->wDataSource == data_source)
            {
                datas[i] = card_data;
                return true;
            }
            i++;
        }

        return Add(card_data);
    }

    LPWFSIDCCARDDATA GetAt(WORD data_source)
    {
        int i = 0;
        while (pCardDatas[i])
        {
            if (pCardDatas[i]->wDataSource == data_source)
            {
                return pCardDatas[i];
            }
            i++;
        }

        return NULL;
    }
protected:
    LPWFSIDCCARDDATA    pCardDatas[MAX_CARD_DATA_COUNT];
    CWFSIDCCardData datas[MAX_CARD_DATA_COUNT];
};
*/
