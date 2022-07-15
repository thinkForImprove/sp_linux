/***************************************************************************
* 文件名称：xfsHelper.h
* 文件描述：XFS系列标准结构体封装类
*
* 使用模块: 如对该文件进行修改,请确认对如下已使用模块的影响
*       IDX,UKEY,IDC,MSR
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年6月30日
* 文件版本：1.0.0.1
***************************************************************************/

#pragma once

#include "QtTypeDef.h"
#include "XFSIDC.H"
#include "XFSBCR.H"
#include "XFSCAM.H"


//**************************************************************************
// --------------------------- IDC命令系结构体封装 ---------------------------
//**************************************************************************

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


//**************************************************************************
// --------------------------- BCR命令系结构体封装 ---------------------------
//**************************************************************************

// 状态类封装
class CWfsBCRStatus : public WFSBCRSTATUS
{
public:
    CWfsBCRStatus()
    {
        Clear();
    }

    // 初始化
    void Clear()
    {
        fwDevice            = WFS_BCR_DEVNODEVICE;                  // 设备状态: 初始无设备
        fwBCRScanner        = WFS_BCR_SCANNEROFF;                   // 扫描状态: 初始不可用
        MSET_XS(dwGuidLights, WFS_BCR_GUIDANCE_NOT_AVAILABLE,       // 指示灯状态: 初始所有不可用
                sizeof(DWORD) * WFS_BCR_GUIDLIGHTS_SIZE);
        lpszExtra           = nullptr;                              // 扩展信息: 初始空
        wDevicePosition     = WFS_BCR_DEVICEPOSNOTSUPP;             // 设备位置信息: 初始不支持
        usPowerSaveRecoveryTime = 0;                                // 设备从省电模式恢复正常秒数: 初始0
    }

    // 用于比较
    bool Diff(CWfsBCRStatus clStat)
    {
        if (this->fwDevice != clStat.fwDevice ||
            this->fwBCRScanner != clStat.fwBCRScanner ||
            this->wDevicePosition != clStat.wDevicePosition ||
            this->usPowerSaveRecoveryTime != clStat.usPowerSaveRecoveryTime)
        {
            return true;
        }
        return false;
    }

    // 用于复制
    void Copy(CWfsBCRStatus clStat)
    {
        this->fwDevice = clStat.fwDevice;
        this->fwBCRScanner = clStat.fwBCRScanner;
        this->wDevicePosition = clStat.wDevicePosition;
        this->usPowerSaveRecoveryTime = clStat.usPowerSaveRecoveryTime;
    }
};

// 能力值类封装
class CWfsBCRCap : public WFSBCRCAPS
{
public:
    CWfsBCRCap()
    {
        wClass          = WFS_SERVICE_CLASS_BCR;                    // 逻辑服务类
        bCompound       = FALSE;                                    // 设备是否组合设备的一部分
        bCanFilterSymbologies = FALSE;                              // 设备是否支持识别条码类型
        lpwSymbologies  = nullptr;                                  // 设备支持的条码类型列表
        MSET_XS(dwGuidLights, WFS_BCR_GUIDANCE_NOT_AVAILABLE,       // 设备指示灯的显示能力
                sizeof(DWORD) * WFS_BCR_GUIDLIGHTS_SIZE);
        lpszExtra       = nullptr;                                  // 扩展能力
        bPowerSaveControl= FALSE;                                   // 是否支持省电模式
    }
};

// 封装ReadBcr命令回参处理
class CWFSBCRREADOUTPUTHelper
{
public:
    CWFSBCRREADOUTPUTHelper() : m_pszOutput(nullptr), m_uSize(0) { }
    ~CWFSBCRREADOUTPUTHelper() { Release(); }
    bool NewBuff(UINT uSize, UINT uDataMemSize = 4096)
    {
        m_uSize = uSize;
        m_uDataMemSize = m_uDataMemSize;
        m_ppOutput = new LPWFSBCRREADOUTPUT[m_uSize + 1];
        if (m_ppOutput == nullptr)
            return false;
        memset(m_ppOutput, 0x00, sizeof(LPWFSBCRREADOUTPUT) * (m_uSize + 1));

        m_pszOutput = new WFSBCRREADOUTPUT[m_uSize];
        if (m_pszOutput == nullptr)
            return false;
        memset(m_pszOutput, 0x00, sizeof(WFSBCRREADOUTPUT) * m_uSize);
        for (UINT i = 0; i < m_uSize; i++)
        {
            m_ppOutput[i] = &m_pszOutput[i];
        }

        for (UINT i = 0; i < m_uSize; i++)
        {
            m_pszOutput[i].lpxBarcodeData = new WFSBCRXDATA;
            if (m_pszOutput[i].lpxBarcodeData == nullptr)
                return false;

            m_pszOutput[i].lpxBarcodeData->usLength = 0;
            m_pszOutput[i].lpxBarcodeData->lpbData = new BYTE[m_uDataMemSize];
            if (m_pszOutput[i].lpxBarcodeData->lpbData == nullptr)
                return false;

            m_pszOutput[i].lpxBarcodeData->lpbData[0] = 0x00;
        }
        return true;
    }
    void Release()
    {
        if (m_pszOutput != nullptr)
        {
            for (UINT i = 0; i < m_uSize; i++)
            {
                if (m_pszOutput[i].lpxBarcodeData != nullptr)
                {
                    if (m_pszOutput[i].lpxBarcodeData->lpbData != nullptr)
                    {
                        delete[]m_pszOutput[i].lpxBarcodeData->lpbData;
                        m_pszOutput[i].lpxBarcodeData->lpbData = nullptr;
                    }

                    delete m_pszOutput[i].lpxBarcodeData;
                    m_pszOutput[i].lpxBarcodeData = nullptr;
                }
            }
            delete[]m_pszOutput;
            m_pszOutput = nullptr;
        }
    }
    LPWFSBCRREADOUTPUT GetBuff(UINT uIndex)
    {
        if (uIndex < m_uSize)
            return &m_pszOutput[uIndex];
        else
            return nullptr;
    }
    LPWFSBCRREADOUTPUT *GetData() { return m_ppOutput; }
    UINT GetSize() { return m_uSize; }
    UINT GetDataMemSize() { return m_uDataMemSize; }
private:
    LPWFSBCRREADOUTPUT m_pszOutput;
    LPWFSBCRREADOUTPUT *m_ppOutput;
    UINT m_uSize;
    UINT m_uDataMemSize;
};


//**************************************************************************
// --------------------------- CAM命令系结构体封装 ---------------------------
//**************************************************************************
class CWfsCAMStatus : public WFSCAMSTATUS
{
public:
    CWfsCAMStatus()
    {
        Clear();
    }

    // 初始化
    void Clear()
    {
        fwDevice    = WFS_CAM_DEVONLINE;
        for (INT i = 0; i < WFS_CAM_CAMERAS_SIZE; i ++)
        {
            fwMedia[i] = WFS_CAM_MEDIANOTSUPP;
        }
        for (INT i = 0; i < WFS_CAM_CAMERAS_SIZE; i ++)
        {
            fwCameras[i] = WFS_CAM_CAMNOTSUPP;
        }
        for (INT i = 0; i < WFS_CAM_CAMERAS_SIZE; i ++)
        {
            usPictures[i] = 0;
        }
        lpszExtra = nullptr;
        wAntiFraudModule = 0;
    }

    // 用于比较(true:有差别, false:无差别)
    bool Diff(CWfsCAMStatus clStat)
    {
        if (fwDevice != clStat.fwDevice)
        {
            return true;
        }
        for (INT i = 0; i < WFS_CAM_CAMERAS_SIZE; i ++)
        {
            if (clStat.fwMedia[i] != fwMedia[i] ||
                clStat.fwCameras[i] != fwCameras[i] ||
                clStat.usPictures[i] != usPictures[i])
            {
                return true;
            }
        }
        return false;
    }

    // 用于复制
    void Copy(CWfsCAMStatus clStat)
    {
        this->fwDevice = clStat.fwDevice;
        for (INT i = 0; i < WFS_CAM_CAMERAS_SIZE; i ++)
        {
            this->fwMedia[i] = clStat.fwMedia[i];
            this->fwCameras[i] = clStat.fwCameras[i];
            this->usPictures[i] = clStat.usPictures[i];
        }
    }
};

class CWfsCAMCap : public WFSCAMCAPS
{
public:
    CWfsCAMCap()
    {
        wClass                          = WFS_SERVICE_CLASS_CAM;
        fwType                          = WFS_CAM_TYPE_CAM;
        fwCameras[WFS_CAM_ROOM]         = WFS_CAM_NOT_AVAILABLE;
        fwCameras[WFS_CAM_PERSON]       = WFS_CAM_AVAILABLE;
        fwCameras[WFS_CAM_EXITSLOT]     = WFS_CAM_NOT_AVAILABLE;
        fwCameras[WFS_CAM_EXTRA]        = WFS_CAM_NOT_AVAILABLE;
        fwCameras[WFS_CAM_HIGHTCAMERA]  = WFS_CAM_NOT_AVAILABLE;
        fwCameras[5]                    = WFS_CAM_NOT_AVAILABLE;
        fwCameras[WFS_CAM_PANORAMIC]    = WFS_CAM_NOT_AVAILABLE;
        fwCameras[7]                    = WFS_CAM_NOT_AVAILABLE;
        usMaxPictures                   = 1000;
        fwCamData                       = WFS_CAM_MANADD;
        usMaxDataLength                 = 67;
        fwCharSupport                   = WFS_CAM_ASCII;
        lpszExtra                       = nullptr;
        bPictureFile                    = TRUE;
        bAntiFraudModule                = FALSE;
    }
};

